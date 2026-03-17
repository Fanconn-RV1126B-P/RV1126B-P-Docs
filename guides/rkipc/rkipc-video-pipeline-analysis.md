# RKIPC Video Pipeline Analysis for RV1126B-P

## Overview

This document analyzes how RKIPC initializes the video pipeline on the RV1126B-P platform to enable direct camera access for custom development with FFmpeg-Rockchip.

**Target Hardware**: RV1126B-P EVB v1.0 with IMX415 4K camera sensor  
**RKIPC Version**: rockchip_rv1126bp_ipc_64_evb1_v10_defconfig  
**Source Files Analyzed**:
- `/app/rkipc/src/rv1126b_ipc/video/video.c` (3258 lines)
- `/app/rkipc/common/isp/rv1126b/isp.c` (1750+ lines)

## Video Pipeline Architecture

The RV1126B video pipeline consists of these hardware blocks:

```
IMX415 Sensor
    ↓
RKCIF (Camera Interface)
    ↓
RKISP (Image Signal Processor)
    ↓ (3 paths: MAINPATH, SELFPATH, FBCPATH)
VI (Video Input) - RKMedia API abstraction
    ↓
GDC (Geometric Distortion Correction) - Optional FEC/DIS
    ↓
VPSS (Video Process Sub-System) - Multi-channel scaler
    ↓ (Multiple output channels)
VENC (Video Encoder) - MPP hardware encoding
```

## Initialization Sequence

### 1. ISP and AIQ Initialization

**File**: `app/rkipc/common/isp/rv1126b/isp.c`

#### Function: `sample_common_isp_init()`
Location: Lines 58-159

**Purpose**: Initialize the Auto Image Quality (AIQ) library and ISP context for camera control.

**Key Steps**:

```c
// 1. Enumerate camera static information
rk_aiq_static_info_t aiq_static_info;
rk_aiq_uapi2_sysctl_enumStaticMetasByPhyId(cam_id, &aiq_static_info);
// Returns: sensor_name = "m00_b_imx415 3-001a"

// 2. Set scene mode based on HDR configuration
if (WDRMode == RK_AIQ_WORKING_MODE_NORMAL)
    rk_aiq_uapi2_sysctl_preInit_scene(sensor_name, "normal", "day");
else
    rk_aiq_uapi2_sysctl_preInit_scene(sensor_name, "hdr", "day");

// 3. Initialize AIQ context with IQ files
aiq_ctx = rk_aiq_uapi2_sysctl_init(
    sensor_name,        // "m00_b_imx415 3-001a"
    iq_file_dir,        // "/etc/iqfiles"
    NULL, NULL
);

// 4. Optional: Enable AI Image Signal Processing
int enable_aiisp = rk_param_get_int("video.source:enable_aiisp", -1);
if (enable_aiisp != -1) {
    aibnr_api_attrib_t aibnr_attr;
    rk_aiq_user_api2_aibnr_GetAttrib(aiq_ctx, &aibnr_attr);
    aibnr_attr.en = enable_aiisp;
    rk_aiq_user_api2_aibnr_SetAttrib(aiq_ctx, &aibnr_attr);
}

// 5. Optional: Enable MEMC (Motion Estimation Motion Compensation)
btnr_api_attrib_t btnr_attr;
rk_aiq_user_api2_btnr_GetAttrib(aiq_ctx, &btnr_attr);
btnr_attr.stAuto.sta.swMemc.sw_btnr_swMemc_en = 
    rk_param_get_int("video.source:enable_memc", 0);
rk_aiq_user_api2_btnr_SetAttrib(aiq_ctx, &btnr_attr);

g_aiq_ctx[cam_id] = aiq_ctx;
```

**Critical Files**:
- IQ Files: `/etc/iqfiles/` - Camera tuning parameters for ISP
- Sensor name format: `m00_b_imx415 3-001a` (media device, bus, sensor, I2C address)

#### Function: `sample_common_isp_run()`
Location: Lines 161-178

**Purpose**: Start the ISP processing pipeline.

```c
// 1. Prepare ISP with working mode
rk_aiq_uapi2_sysctl_prepare(g_aiq_ctx[cam_id], 0, 0, g_WDRMode[cam_id]);

// 2. Start ISP processing
rk_aiq_uapi2_sysctl_start(g_aiq_ctx[cam_id]);
```

#### Function: `rk_isp_init()`
Location: Lines 1657-1750

**Purpose**: High-level ISP initialization called during RKIPC startup.

```c
// 1. Get IQ file directory from config
const char *iq_file_dir = rk_param_get_string("isp.0.blc:iqfiles", NULL);
// Returns: "/etc/iqfiles"

// 2. Detect HDR mode from config
int hdr_mode = rk_param_get_int("video.source:hdr_mode", 0);
rk_aiq_working_mode_t work_mode = (hdr_mode == 0) ? 
    RK_AIQ_WORKING_MODE_NORMAL : RK_AIQ_WORKING_MODE_ISP_HDR2;

// 3. Initialize ISP context
sample_common_isp_init(0, work_mode, false, iq_file_dir);

// 4. Start ISP
sample_common_isp_run(0);
```

### 2. VI (Video Input) Device Initialization

**File**: `app/rkipc/src/rv1126b_ipc/video/video.c`

#### Function: `rkipc_vi_dev_init()`
Location: Lines 367-437

**Purpose**: Initialize the VI device and bind it to the ISP pipeline.

**Channel Definitions** (Lines 19-29):
```c
#define RKISP_MAINPATH   0  // Main output path (full resolution)
#define RKISP_SELFPATH   1  // Secondary path (lower resolution)
#define RKISP_FBCPATH    2  // Frame Buffer Compression path

static int g_vi_chn_id = 0;           // Main VI channel
static int g_vi_for_venc_0_id = 3;    // VI channel for encoder 0
static int g_vi_for_venc_1_id = 2;    // VI channel for encoder 1
```

**Key Steps**:

```c
// 1. Configure VI device attributes
VI_DEV_ATTR_S stDevAttr;
memset(&stDevAttr, 0, sizeof(VI_DEV_ATTR_S));
stDevAttr.enWorkMode = VI_WORK_MODE_NORMAL;  // or VI_WORK_MODE_LUMA_ONLY

// Get sensor image type (SDR8, HDR10, etc.)
const char *sensor_image_type = 
    rk_param_get_string("video.source:sensor_image_type", NULL);
if (sensor_image_type) {
    if (!strcmp(sensor_image_type, "SDR8"))
        stDevAttr.enImgDynRg = DYNAMIC_RANGE_SDR8;
    else if (!strcmp(sensor_image_type, "HDR10"))
        stDevAttr.enImgDynRg = DYNAMIC_RANGE_HDR10;
    // ... other formats
}

// Set VI device attributes
RK_MPI_VI_SetDevAttr(pipe_id_, &stDevAttr);

// 2. Enable VI device
RK_MPI_VI_EnableDev(pipe_id_);

// 3. Bind VI device to ISP pipe
VI_DEV_BIND_PIPE_S stBindPipe;
memset(&stBindPipe, 0, sizeof(VI_DEV_BIND_PIPE_S));
stBindPipe.u32PipeNum = pipe_id_;
stBindPipe.PipeId[0] = pipe_id_;
RK_MPI_VI_SetDevBindPipe(pipe_id_, &stBindPipe);

// 4. Create extension channels if needed
int enc_image_type = rk_param_get_int("video.0:enc_image_type", 0);
if (enc_image_type == 1) {  // RGBIR mode
    // Setup additional VI channels for RGB and IR streams
    VI_EXT_CHN_ATTR_S stExtChnAttr;
    memset(&stExtChnAttr, 0, sizeof(VI_EXT_CHN_ATTR_S));
    stExtChnAttr.enCompressMode = COMPRESS_MODE_NONE;
    stExtChnAttr.enPixFmt = g_enPixFmt;
    stExtChnAttr.stSize.u32Width = width;
    stExtChnAttr.stSize.u32Height = height;
    RK_MPI_VI_SetChnExtAttr(pipe_id_, g_vi_for_venc_0_id, &stExtChnAttr);
}
```

**Critical Parameters**:
- `pipe_id_`: ISP pipeline ID (typically 0 for single camera)
- Work modes: `VI_WORK_MODE_NORMAL`, `VI_WORK_MODE_LUMA_ONLY`
- Dynamic ranges: `DYNAMIC_RANGE_SDR8`, `DYNAMIC_RANGE_HDR10`, etc.

### 3. VI + GDC + VPSS Pipeline Setup

#### Function: `rkipc_vi_gdc_vpss_init()`
Location: Lines 439-700+

**Purpose**: Create the complete video processing pipeline with VI, GDC, and VPSS.

#### 3.1 VI Channel Configuration

```c
VI_CHN_ATTR_S vi_chn_attr;
memset(&vi_chn_attr, 0, sizeof(vi_chn_attr));

// Buffer configuration
vi_chn_attr.stIspOpt.u32BufCount = 3;  // Triple buffering
vi_chn_attr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;

// Maximum resolution
vi_chn_attr.stIspOpt.stMaxSize.u32Width = 
    rk_param_get_int("video.0:max_width", 2560);
vi_chn_attr.stIspOpt.stMaxSize.u32Height = 
    rk_param_get_int("video.0:max_height", 1440);

// Output resolution
vi_chn_attr.stSize.u32Width = video_width;   // 3840 for 4K
vi_chn_attr.stSize.u32Height = video_height; // 2160 for 4K

// Pixel format
vi_chn_attr.enPixelFormat = RK_FMT_YUV420SP;  // NV12

// Optional: Frame buffer compression
if (rk_param_get_int("video.source:enable_compress", 0))
    vi_chn_attr.enVideoFormat = VIDEO_FORMAT_TILE_4x4;
else
    vi_chn_attr.enVideoFormat = VIDEO_FORMAT_LINEAR;

vi_chn_attr.enCompressMode = COMPRESS_MODE_NONE;

// Apply configuration
RK_MPI_VI_SetChnAttr(pipe_id_, g_vi_chn_id, &vi_chn_attr);
RK_MPI_VI_EnableChn(pipe_id_, g_vi_chn_id);
```

#### 3.2 GDC (Geometric Distortion Correction) Configuration

```c
GDC_CHN_ATTR_S stAttr = {0};

stAttr.u32MaxInQueue = 3;
stAttr.u32MaxOutQueue = 3;
stAttr.s32Depth = 0;
stAttr.s32DstWidth = video_width;
stAttr.s32DstHeight = video_height;
stAttr.enDstPixelFormat = RK_FMT_YUV420SP;

// Optional: Output compression
if (rk_param_get_int("video.source:enable_compress", 0))
    stAttr.enDstCompMode = COMPRESS_RFBC_64x4;
else
    stAttr.enDstCompMode = COMPRESS_MODE_NONE;

// Mode selection: DIS (Digital Image Stabilization) or FEC (Fish Eye Correction)
const char *distortion_correction = 
    rk_param_get_string("video.source:distortion_correction", NULL);

if (!strcmp(distortion_correction, "DIS")) {
    stAttr.enMode = GDC_CHN_MODE_DIS;
    const char *dis_file = 
        rk_param_get_string("isp.0.enhancement:dis_file", NULL);
    memcpy(stAttr.cfgFile, dis_file, strlen(dis_file));
} else {
    stAttr.enMode = GDC_CHN_MODE_FEC;
    // FEC parameters from ini file
    const char *fec_ini_file = 
        rk_param_get_string("isp.0.enhancement:fec_ini_file", NULL);
    RK_MPI_GDC_GetAttrFromFile(&stUpdateAttr, fec_ini_file);
    // ... configure FEC coefficients
}

RK_MPI_GDC_CreateChn(0, &stAttr);
```

#### 3.3 VPSS (Video Process Sub-System) Configuration

```c
VPSS_GRP VpssGrp = 0;
VPSS_GRP_ATTR_S stVpssGrpAttr;
memset(&stVpssGrpAttr, 0, sizeof(stVpssGrpAttr));

// Group attributes
stVpssGrpAttr.u32MaxW = 4096;
stVpssGrpAttr.u32MaxH = 4096;
stVpssGrpAttr.enPixelFormat = RK_FMT_YUV420SP;
stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;  // Auto
stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;  // Auto

// Compression
if (rk_param_get_int("video.source:enable_compress", 0))
    stVpssGrpAttr.enCompressMode = COMPRESS_RFBC_64x4;
else
    stVpssGrpAttr.enCompressMode = COMPRESS_MODE_NONE;

// Processing device selection (VPSS hardware, RGA, or GPU)
const char *vpss_proc_dev = 
    rk_param_get_string("video.source:vpss_proc_dev", "vpss");
if (!strcmp(vpss_proc_dev, "gpu"))
    stVpssGrpAttr.enVProcDev = VIDEO_PROC_DEV_GPU;
else if (!strcmp(vpss_proc_dev, "rga"))
    stVpssGrpAttr.enVProcDev = VIDEO_PROC_DEV_RGA;
else
    stVpssGrpAttr.enVProcDev = VIDEO_PROC_DEV_VPSS;

RK_MPI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);

// Configure output channels (up to 4)
// Channel 0: Primary encoder (4K HEVC)
VPSS_CHN_ATTR_S stVpssChnAttr[0];
stVpssChnAttr[0].enChnMode = VPSS_CHN_MODE_AUTO;
stVpssChnAttr[0].enDynamicRange = DYNAMIC_RANGE_SDR8;
stVpssChnAttr[0].enPixelFormat = RK_FMT_YUV420SP;
stVpssChnAttr[0].u32Width = 3840;   // Output width
stVpssChnAttr[0].u32Height = 2160;  // Output height
stVpssChnAttr[0].u32FrameBufCnt = 2;
RK_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_CHN0, &stVpssChnAttr[0]);
RK_MPI_VPSS_EnableChn(VpssGrp, VPSS_CHN0);

// Channel 1: Secondary encoder (1080p H.264)
// Channel 2: NPU/IVS processing
// Channel 3: Video output (VO)

RK_MPI_VPSS_EnableBackupFrame(VpssGrp);
RK_MPI_VPSS_StartGrp(VpssGrp);
```

#### 3.4 Module Binding

```c
// Define module channels
MPP_CHN_S vi_chn, gdc_chn, vpss_in_chn;

vi_chn.enModId = RK_ID_VI;
vi_chn.s32DevId = 0;
vi_chn.s32ChnId = g_vi_chn_id;  // 0

gdc_chn.enModId = RK_ID_GDC;
gdc_chn.s32DevId = 0;
gdc_chn.s32ChnId = 0;

vpss_in_chn.enModId = RK_ID_VPSS;
vpss_in_chn.s32DevId = 0;
vpss_in_chn.s32ChnId = 0;

// Bind modules: VI → GDC → VPSS
RK_MPI_SYS_Bind(&vi_chn, &gdc_chn);
RK_MPI_SYS_Bind(&gdc_chn, &vpss_in_chn);
```

### 4. Media Controller (V4L2) Layer

While RKIPC primarily uses RKMedia API, the underlying media controller pipeline is managed by the kernel. To inspect it:

```bash
media-ctl -p -d /dev/media0
```

**Expected Output** (from RKISC system):
```
- entity 1: rkisp-isp-subdev (4 pads, 8 links)
            type V4L2 subdev subtype Unknown flags 0
        pad0: Sink
            <- "rkisp-csi-subdev":1 [ENABLED]
        pad2: Source
            -> "rkisp_mainpath":0 [ENABLED]
            -> "rkisp_selfpath":0 []
        pad3: Source
            -> "rkisp_rawpath":0 []

- entity 6: rkisp_mainpath (1 pad, 1 link)
            type Node subtype V4L flags 0
            device node name /dev/video0
        pad0: Sink
            <- "rkisp-isp-subdev":2 [ENABLED]

- entity 12: rkisp-csi-subdev (2 pads, 2 links)
             type V4L2 subdev subtype Unknown flags 0
         pad0: Sink
             <- "rockchip-mipi-csi2":1 [ENABLED]
         pad1: Source
             -> "rkisp-isp-subdev":0 [ENABLED]

- entity 15: rockchip-mipi-csi2 (2 pads, 2 links)
             type V4L2 subdev subtype Unknown flags 0
         pad0: Sink
             [fmt:SBGGR10_1X10/3840x2160]
         pad1: Source
             [fmt:SBGGR10_1X10/3840x2160]
             -> "rkisp-csi-subdev":0 [ENABLED]

- entity 18: m00_b_imx415 3-001a (1 pad, 1 link)
             type V4L2 subdev subtype Sensor flags 0
             device node name /dev/v4l-subdev0
         pad0: Source
             [fmt:SBGGR10_1X10/3840x2160 @1/30]
             -> "rockchip-mipi-csi2":0 [ENABLED]
```

**Key V4L2 Devices**:
- `/dev/video0`: RKISP mainpath (full resolution output after ISP)
- `/dev/video1`: RKISP selfpath (secondary output)
- `/dev/video-camera0`: Symlink to mainpath (used by RKIPC)
- `/dev/v4l-subdev0`: IMX415 sensor control interface

## Complete Initialization Call Sequence

Based on RKIPC source analysis, here's the complete initialization order:

```c
// 1. System initialization (RKMedia)
RK_MPI_SYS_Init();

// 2. ISP/AIQ initialization
rk_isp_init(0);  // cam_id = 0
    → sample_common_isp_init(0, work_mode, false, "/etc/iqfiles")
        → rk_aiq_uapi2_sysctl_enumStaticMetasByPhyId(0, &aiq_static_info)
        → rk_aiq_uapi2_sysctl_preInit_scene(sensor_name, "normal", "day")
        → rk_aiq_uapi2_sysctl_init(sensor_name, iq_file_dir, NULL, NULL)
    → sample_common_isp_run(0)
        → rk_aiq_uapi2_sysctl_prepare(aiq_ctx, 0, 0, work_mode)
        → rk_aiq_uapi2_sysctl_start(aiq_ctx)

// 3. VI device initialization
rkipc_vi_dev_init();
    → RK_MPI_VI_SetDevAttr(pipe_id, &stDevAttr)
    → RK_MPI_VI_EnableDev(pipe_id)
    → RK_MPI_VI_SetDevBindPipe(pipe_id, &stBindPipe)

// 4. Video processing pipeline
rkipc_vi_gdc_vpss_init();
    → RK_MPI_VI_SetChnAttr(pipe_id, g_vi_chn_id, &vi_chn_attr)
    → RK_MPI_VI_EnableChn(pipe_id, g_vi_chn_id)
    → RK_MPI_GDC_CreateChn(0, &gdc_attr)
    → RK_MPI_VPSS_CreateGrp(VpssGrp, &vpss_grp_attr)
    → RK_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn[i], &vpss_chn_attr[i])
    → RK_MPI_VPSS_EnableChn(VpssGrp, VpssChn[i])
    → RK_MPI_VPSS_StartGrp(VpssGrp)
    → RK_MPI_SYS_Bind(&vi_chn, &gdc_chn)
    → RK_MPI_SYS_Bind(&gdc_chn, &vpss_in_chn)

// 5. Video encoder initialization (for each stream)
rkipc_venc_0_init();
    → RK_MPI_VENC_CreateChn(venc_chn_id, &venc_chn_attr)
    → RK_MPI_VENC_StartRecvFrame(venc_chn_id, &recv_param)
    → RK_MPI_SYS_Bind(&vpss_out_chn[0], &venc_chn)

// 6. Start receiving frames
// Now camera frames flow: Sensor → ISP → VI → GDC → VPSS → VENC
```

## Required Libraries and Headers

To replicate this initialization in a standalone application:

**Libraries**:
```
librkaiq.so         # AIQ library (ISP control)
librockit.so        # RKMedia API
libeasymedia.so     # Media framework
librga.so           # RGA hardware acceleration
libmpp.so           # MPP codec library
```

**Header Files**:
```c
#include <rk_aiq_user_api2_sysctl.h>    // AIQ system control
#include <rk_aiq_user_api2_imgproc.h>   // AIQ image processing
#include <rkmedia_api.h>                 // RKMedia main API
#include <rkmedia_venc.h>                // Video encoder
#include <rkmedia_vi.h>                  // Video input
```

## Configuration Parameters

RKIPC reads these from `/userdata/rkipc.ini`:

**Video Source**:
```ini
[video.source]
width = 3840
height = 2160
max_width = 3840
max_height = 2160
rotation = 0
hdr_mode = 0
sensor_image_type = SDR8
enable_compress = 0
enable_aiisp = 0
enable_memc = 0
vpss_proc_dev = vpss
distortion_correction = close
```

**ISP Configuration**:
```ini
[isp.0.blc]
iqfiles = /etc/iqfiles

[isp.0.enhancement]
fec_ini_file = /etc/fec_config.ini
fec_level = 0
dis_file = /etc/dis_config.bin
```

**Video Stream 0** (Main):
```ini
[video.0]
width = 3840
height = 2160
max_width = 3840
max_height = 2160
output_data_type = H.265
enc_image_type = 0
h264_profile = 100
h265_profile = 0
bitrate = 8192
```

## Direct V4L2 Access Challenges

Attempting to use `/dev/video-camera0` directly with FFmpeg fails because:

1. **ISP Not Initialized**: Sensor outputs raw Bayer data (SBGGR10), but ISP not running to convert to YUV
2. **AIQ Not Running**: Auto exposure, focus, white balance not active
3. **Media Links Not Configured**: Kernel media controller pipeline not properly set up
4. **Sensor Not Streaming**: Sensor may not be powered/configured to output frames

**Why RKIPC Method Works**:
- RKMedia API automatically handles all kernel V4L2/media-ctl setup
- AIQ library runs 3A algorithms in background threads
- VI device abstracts away V4L2 complexity
- DMABUF zero-copy path from ISP to encoder

## Simplified Approach for Custom Development

To access camera with FFmpeg without RKIPC, you have two options:

### Option A: Use RTSP Stream (Recommended)

**Pros**: Simple, works immediately, hardware encoding included  
**Cons**: RKIPC must keep running, extra latency

```bash
# Keep RKIPC running
# Use FFmpeg to capture and transcode
ffmpeg -rtsp_transport tcp -i rtsp://localhost:554/live/0 \
    -c:v h264_rkmpp -b:v 4M output.mp4
```

### Option B: Create Minimal RKMedia Application

**Pros**: Full control, no RKIPC dependency, lower latency  
**Cons**: Complex initialization, requires linking RKMedia libraries

Create a standalone C program that:
1. Initializes ISP/AIQ with `sample_common_isp_init()` and `sample_common_isp_run()`
2. Sets up VI device with `rkipc_vi_dev_init()`
3. Creates VPSS channels for desired resolutions
4. Exposes a V4L2 output device or uses RKMedia callbacks
5. Feeds frames to FFmpeg via pipe or shared memory

**Example skeleton** (requires proper error handling):
```c
#include <rkmedia_api.h>

int main() {
    // 1. System init
    RK_MPI_SYS_Init();
    
    // 2. ISP init
    sample_common_isp_init(0, RK_AIQ_WORKING_MODE_NORMAL, 
                          false, "/etc/iqfiles");
    sample_common_isp_run(0);
    
    // 3. VI device
    VI_DEV_ATTR_S dev_attr = {
        .enWorkMode = VI_WORK_MODE_NORMAL,
        .enImgDynRg = DYNAMIC_RANGE_SDR8
    };
    RK_MPI_VI_SetDevAttr(0, &dev_attr);
    RK_MPI_VI_EnableDev(0);
    
    // 4. VI channel
    VI_CHN_ATTR_S chn_attr = {
        .stSize = { .u32Width = 3840, .u32Height = 2160 },
        .enPixelFormat = RK_FMT_YUV420SP,
        .stIspOpt = {
            .u32BufCount = 3,
            .enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF
        }
    };
    RK_MPI_VI_SetChnAttr(0, 0, &chn_attr);
    RK_MPI_VI_EnableChn(0, 0);
    
    // 5. Get frames in loop
    VIDEO_FRAME_INFO_S frame;
    while (running) {
        RK_MPI_VI_GetChnFrame(0, 0, &frame, 1000);
        // Process frame (encode, write to pipe, etc.)
        RK_MPI_VI_ReleaseChnFrame(0, 0, &frame);
    }
    
    // Cleanup
    RK_MPI_VI_DisableChn(0, 0);
    RK_MPI_VI_DisableDev(0);
    sample_common_isp_stop(0);
    RK_MPI_SYS_Exit();
}
```

### Option C: Use Existing V4L2 After RKIPC Initialization

**Pros**: Simplest hybrid approach  
**Cons**: RKIPC must start first, then be stopped carefully

```bash
# 1. Let RKIPC initialize everything
systemctl start rkipc

# 2. Wait for initialization (check logs)
sleep 5

# 3. Stop RKIPC but keep ISP running
# This is DANGEROUS - may crash ISP if not done correctly
systemctl stop rkipc

# 4. Now try V4L2 capture (may still fail if ISP stops)
ffmpeg -f v4l2 -i /dev/video-camera0 -c:v h264_rkmpp output.mp4
```

**Warning**: This approach is unreliable because:
- Stopping RKIPC may deinitialize ISP/AIQ
- VI channels may be torn down
- No 3A algorithms running (image quality degrades)

## Next Steps for Custom Development

1. **Study RKMedia API**: Read `/usr/include/rkmedia/*.h` headers
2. **Extract IQ Files**: Ensure `/etc/iqfiles/` contains IMX415 tuning files
3. **Build Test Application**: Start with minimal VI + ISP initialization
4. **Test Frame Capture**: Use `RK_MPI_VI_GetChnFrame()` to verify pipeline
5. **Integrate with FFmpeg**: Either pipe frames or use RKMedia encoder directly
6. **Add Error Handling**: Handle initialization failures gracefully

## References

- RKMedia API Documentation: `RV1126B-P-SDK/docs/Multimedia/Rockchip_Developer_Guide_Linux_RKMedia_CN.pdf`
- AIQ API Reference: `RV1126B-P-SDK/docs/Camera/Rockchip_Development_Guide_ISP2x_CN_v1.4.0.pdf`
- Media Controller: `media-ctl -p -d /dev/media0` output on running device

## Conclusion

RKIPC initializes the video pipeline using the RKMedia API which provides a high-level abstraction over V4L2/media-ctl. The key components are:

1. **ISP/AIQ**: Handles sensor control and image quality
2. **VI Device**: Abstracts V4L2 video capture
3. **VPSS**: Multi-channel video scaling/processing
4. **VENC**: Hardware video encoding

For custom FFmpeg integration, using the RTSP stream is the simplest approach. For full control, you'll need to create a RKMedia application that initializes the pipeline and provides frames to FFmpeg.
