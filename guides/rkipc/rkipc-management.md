# RKIPC Management Guide

Complete guide for managing RKIPC (Rockchip IPC) application on RV1126BP.

---

## Table of Contents

1. [Overview](#overview)
2. [Stream Specifications](#stream-specifications)
3. [AI and Computer Vision Features](#ai-and-computer-vision-features)
4. [Starting RKIPC](#starting-rkipc)
5. [Stopping RKIPC](#stopping-rkipc)
6. [Viewing Logs](#viewing-logs)
7. [Configuration](#configuration)
8. [Background: RkLunch.sh](#background-rklunchsh)
9. [Troubleshooting](#troubleshooting)

---

## Overview

### What is RKIPC?

RKIPC is Rockchip's IPC (IP Camera) application that provides:
- **RTSP Streaming**: H.265/H.264 video streaming on port 554
- **Web Interface**: Management UI on port 80
- **Audio Support**: G.711A audio codec
- **Motion Detection**: IVS (Intelligent Video Surveillance)
- **Storage**: Recording to local storage
- **ISP Integration**: Direct integration with camera ISP

### Key Components

| Component | Location | Purpose |
|-----------|----------|---------|
| Main Binary | `/usr/bin/rkipc` | Core RKIPC application |
| Launch Script | `/usr/bin/RkLunch.sh` | Startup script with initialization |
| Stop Script | `/usr/bin/RkLunch-stop.sh` | Graceful shutdown script |
| Init Script | `/etc/init.d/S99rkipc` | System startup integration |
| Configuration | `/userdata/rkipc.ini` | Main configuration file |
| Default Config | `/usr/share/rkipc-*.ini` | Factory default configurations |
| IQ Files | `/etc/iqfiles/` | ISP tuning files |

### Process Information

```bash
# Check if RKIPC is running
ps | grep rkipc

# Expected output:
#  1670 root     rkipc
```

### Access URLs

| Service | URL | Credentials |
|---------|-----|-------------|
| RTSP Main Stream | `rtsp://<BOARD_IP>:554/live/0` | None |
| RTSP Sub Stream | `rtsp://<BOARD_IP>:554/live/1` | None |
| Web Interface | `http://<BOARD_IP>` | admin / admin |

**Test RTSP streams:**
```bash
# Main stream (high resolution - typically 3840x2160)
ffplay -rtsp_transport tcp rtsp://<BOARD_IP>:554/live/0

# Sub stream (lower resolution - typically 1920x1080)
ffplay -rtsp_transport tcp rtsp://<BOARD_IP>:554/live/1
```

**Access web interface:**
```bash
# Open in browser
http://<BOARD_IP>

# Login with:
# Username: admin
# Password: admin
```

---

## Stream Specifications

### Video Streams

RKIPC provides two H.265 encoded video streams with different resolutions and purposes:

#### Stream 0 - Main Stream (High Resolution)

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Resolution** | 3840x2160 | 4K UHD |
| **Frame Rate** | 30 fps | Configurable in rkipc.ini |
| **Codec** | H.265 (HEVC) | Hardware accelerated |
| **Bitrate Control** | VBR | Variable bitrate |
| **Quality** | Highest | Maximum quality setting |
| **GOP Size** | 60 frames | 2 seconds at 30fps |
| **Smart Encoding** | Enabled | AI-based ROI optimization |
| **RTSP Path** | `/live/0` | rtsp://\<IP\>:554/live/0 |
| **Purpose** | Recording, archival | High-quality main stream |

**Configuration in `/userdata/rkipc.ini`:**
```ini
[video.0]
type = mainStream
width = 3840
height = 2160
src_frame_rate_num = 30
dst_frame_rate_num = 30
video_type = compositeStream
h264_profile = high
h265_profile = main
output_data_type = H.265
rc_mode = VBR
rc_quality = highest
smart = open
gop_mode = normalP
gop = 60
```

#### Stream 1 - Sub Stream (Lower Resolution)

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Resolution** | 640x480 | SD quality |
| **Frame Rate** | 30 fps | Configurable in rkipc.ini |
| **Codec** | H.265 (HEVC) | Hardware accelerated |
| **Bitrate Control** | VBR | Variable bitrate |
| **Quality** | Highest | Maximum quality setting |
| **GOP Size** | 60 frames | 2 seconds at 30fps |
| **Smart Encoding** | Enabled | AI-based ROI optimization |
| **RTSP Path** | `/live/1` | rtsp://\<IP\>:554/live/1 |
| **Purpose** | Live preview, mobile | Lower bandwidth stream |

**Note**: The 640x480 resolution may appear with a distorted aspect ratio (4:3 displayed on 16:9). This is a known limitation of the current configuration.

**Configuration in `/userdata/rkipc.ini`:**
```ini
[video.1]
type = subStream
width = 640
height = 480
src_frame_rate_num = 30
dst_frame_rate_num = 30
video_type = compositeStream
h264_profile = main
h265_profile = main
output_data_type = H.265
rc_mode = VBR
rc_quality = highest
smart = open
gop_mode = normalP
gop = 60
```

### Encoder Settings

Both streams use:
- **Smart P-Frame Length**: 30 frames
- **Stream Smoothness**: 50% (balance between quality and bandwidth)
- **Enable Combine**: Enabled for audio/video muxing

---

## AI and Computer Vision Features

RKIPC includes integrated AI and computer vision capabilities powered by the RV1126's NPU (Neural Processing Unit) and Rockchip's intelligent video algorithms.

### Overview

| Feature | Status | Purpose |
|---------|--------|---------|
| **NPU (Neural Processing Unit)** | Enabled | Hardware AI acceleration |
| **IVS (Intelligent Video Surveillance)** | Enabled | Motion and object detection |
| **Motion Detection** | Enabled | Detect movement in frame |
| **Object Detection** | Enabled | Detect and classify objects |
| **Rockiva Framework** | Configured | AI model inference engine |
| **Smart Encoding** | Enabled | ROI-based bitrate optimization |

### NPU (Neural Processing Unit)

The RV1126's integrated NPU provides hardware acceleration for AI inference tasks.

**Configuration:**
```ini
[video.source]
enable_npu = 1      # NPU enabled
npu_fps = 10        # Process 10 frames/sec for AI
```

**Specifications:**
- **Processing Rate**: 10 fps (configurable)
- **Purpose**: Run neural network models for object detection
- **Hardware**: Dedicated NPU core separate from CPU
- **Integration**: Connected to video pipeline for real-time analysis

**What it does:**
- Runs AI models on selected video frames
- Provides inference results to IVS and smart encoding
- Operates in parallel with video encoding (minimal CPU impact)

### IVS (Intelligent Video Surveillance)

IVS provides motion detection and object detection capabilities integrated into the video pipeline.

**Configuration:**
```ini
[ivs]
smear = 0           # Smear detection disabled
weightp = 0         # Weighted prediction disabled
md = 1              # Motion detection ENABLED
od = 1              # Object detection ENABLED
md_sensibility = 3  # Motion detection sensitivity (1-5)
```

**Motion Detection (MD):**
- **Status**: Enabled (`md=1`)
- **Sensitivity**: Level 3 (medium)
- **Range**: 1 (low) to 5 (high sensitivity)
- **Purpose**: Detect movement in camera frame
- **Output**: Motion regions reported to smart encoding

**Object Detection (OD):**
- **Status**: Enabled (`od=1`)
- **Purpose**: Detect and classify objects (person, vehicle, etc.)
- **Integration**: Works with Rockiva AI models
- **Output**: Bounding boxes and classifications

**How to modify:**
```bash
# Edit configuration
adb shell "vi /userdata/rkipc.ini"

# Change motion detection sensitivity (1-5)
[ivs]
md_sensibility = 5  # More sensitive

# Disable motion detection
md = 0

# Disable object detection
od = 0

# Restart RKIPC
adb shell "killall rkipc && /usr/bin/RkLunch.sh &"
```

### Rockiva AI Framework

Rockiva is Rockchip's AI framework for video analytics, using RKNN (Rockchip Neural Network) models.

**Configuration:**
```ini
[rockiva]
rockiva_model_type = big        # Model size: big or small
rockiva_model_path = /usr/lib/  # Path to AI models
sensitivity_level = 90          # Detection confidence threshold (0-100)
time_threshold = 1              # Time threshold in seconds
```

**Model Types:**
- **big**: Larger model, more accurate, higher NPU load
- **small**: Smaller model, faster inference, lower accuracy

**Parameters:**
- **Model Path**: `/usr/lib/` - Contains RKNN model files
- **Sensitivity**: 90% - Detection confidence threshold
  - Higher value = fewer false positives, might miss detections
  - Lower value = more detections, more false positives
- **Time Threshold**: 1 second - Minimum time for detection to be valid

**Supported Detections:**
Based on the model configuration, Rockiva typically detects:
- **Person**: Human detection and tracking
- **Vehicle**: Car, truck, motorcycle
- **Face**: Face detection (if enabled)
- **Other objects**: Depending on model training

**Model Files:**
```bash
# List available models
adb shell "ls -la /usr/lib/*.rknn"

# Check model in use
adb shell "cat /userdata/rkipc.ini | grep rockiva_model"
```

### Smart Encoding

Smart encoding uses AI detection results to optimize video encoding by allocating more bitrate to regions of interest (ROI).

**How it works:**
1. **AI Detection**: NPU + Rockiva detect objects and motion
2. **ROI Identification**: Detected objects marked as important regions
3. **Bitrate Allocation**: Encoder gives more bitrate to ROI areas
4. **Background Reduction**: Less important areas get lower quality
5. **Result**: Better quality on subjects, lower overall bitrate

**Configuration:**
```ini
[video.0]
smart = open        # Smart encoding enabled

[video.1]
smart = open        # Smart encoding enabled
```

**Benefits:**
- **Bandwidth Savings**: 20-40% reduction in average bitrate
- **Quality Improvement**: Better quality on detected objects
- **Consistent File Size**: More predictable storage requirements

**Smart P-Frame Settings:**
```ini
smartp_viridrlen = 30    # Virtual IDR length for smart P-frames
stream_smooth = 50       # Smoothness factor (0-100)
```

### ROI (Region of Interest)

RKIPC supports manual ROI configuration for fixed areas that should receive higher encoding quality.

**Configuration:**
```ini
[roi.0]
stream_type = mainStream    # Apply to main stream
position_x = 0              # ROI x coordinate
position_y = 0              # ROI y coordinate
width = 640                 # ROI width
height = 480                # ROI height
quality_level = 0           # Quality delta (0=highest)
enabled = 0                 # Currently disabled
```

**Available ROI Zones:**
- `[roi.0]`, `[roi.1]`: Main stream ROI zones
- `[roi.2]`, `[roi.3]`: Sub stream ROI zones
- `[roi.4]`, `[roi.5]`: Third stream ROI zones (if enabled)

**Quality Levels:**
- **0**: Highest quality (maximum bitrate)
- **1-3**: Medium quality
- **4-6**: Lower quality (minimum bitrate)

**Enable ROI:**
```bash
# Edit configuration
adb shell "vi /userdata/rkipc.ini"

# Configure ROI for center 1/4 of 3840x2160 frame
[roi.0]
stream_type = mainStream
position_x = 960      # Center X
position_y = 540      # Center Y
width = 1920          # 1/2 frame width
height = 1080         # 1/2 frame height
quality_level = 0     # Highest quality
enabled = 1           # Enable ROI

# Restart RKIPC
adb shell "killall rkipc && /usr/bin/RkLunch.sh &"
```

**Note**: Smart encoding (AI-based ROI) and manual ROI can work together. Smart encoding dynamically adjusts based on detections, while manual ROI provides fixed priority regions.

### Viewing AI Detection Results

AI detection results are not directly visible in RTSP streams by default. To view detection data:

**Option 1: Web Interface**
```bash
# Access web UI
http://<BOARD_IP>

# Navigate to Event or Detection sections
# May show motion detection alerts and object detection logs
```

**Option 2: API Access**
The IPC web backend provides API endpoints for detection events:
```bash
# Motion detection info
curl http://<BOARD_IP>/api/event/motion-detection

# Object detection events
curl http://<BOARD_IP>/api/event/object-detection
```

**Option 3: Smart Encoding Visualization**
```bash
# Enable smart encoding debug (if available in source)
# This would overlay ROI boxes on video stream
# Requires recompilation with debug flags
```

### Performance Considerations

**CPU Usage:**
- **Without AI**: ~40-50% CPU for dual H.265 encoding
- **With AI (NPU)**: ~45-55% CPU (NPU offloads most AI work)
- **NPU Load**: Depends on npu_fps setting (10fps = moderate load)

**Memory Usage:**
- **Base RKIPC**: ~80-100MB
- **With AI Models**: +20-40MB for model storage
- **Frame Buffers**: +30-50MB for NPU frame queue

**Latency:**
- **Video Encoding**: <100ms for both streams
- **AI Detection**: 100ms per frame (at 10fps = every 3rd frame)
- **RTSP Latency**: 200-500ms depending on network

**Tuning AI Performance:**
```bash
# Reduce NPU frame rate for lower CPU usage
adb shell "vi /userdata/rkipc.ini"

[video.source]
npu_fps = 5  # Process every 6th frame instead of every 3rd

# Or disable AI features entirely
enable_npu = 0
enable_ivs = 0

# Restart RKIPC
adb shell "killall rkipc && /usr/bin/RkLunch.sh &"
```

### AI Feature Summary

| Feature | Default | Modifiable | Impact |
|---------|---------|------------|--------|
| NPU Processing | 10 fps | Yes | CPU: Low, Accuracy: High |
| Motion Detection | Enabled | Yes | Alerts, Smart Encoding |
| Object Detection | Enabled | Yes | Classification, ROI |
| Rockiva Model | Big | Yes | Accuracy vs Performance |
| Smart Encoding | Enabled | Yes | Bitrate Optimization |
| Manual ROI | Disabled | Yes | Fixed Region Priority |

**Default configuration provides:**
- ✅ Real-time motion detection (sensitivity level 3)
- ✅ Object detection and classification (90% confidence)
- ✅ AI-optimized encoding (20-40% bitrate savings)
- ✅ Hardware-accelerated inference (minimal CPU impact)

---

## Starting RKIPC

### Method 1: Using RkLunch.sh (Recommended)

```bash
# Via ADB
adb shell "/usr/bin/RkLunch.sh &"

# Via SSH  
ssh root@<BOARD_IP> '/usr/bin/RkLunch.sh &'

# Directly on device
/usr/bin/RkLunch.sh &
```

**What RkLunch.sh does:**
1. Waits for `/userdata` partition to mount
2. Loads kernel modules (`kmpp.ko`, `rockit.ko`, etc.)
3. Initializes network (DHCP)
4. Detects camera resolution and selects appropriate config
5. Creates `/userdata/rkipc.ini` from defaults if needed
6. Launches `rkipc` binary with IQ files

### Method 2: Using Init Script

```bash
# Start RKIPC via init system
adb shell "/etc/init.d/S99rkipc start"

# Or via SSH
ssh root@<BOARD_IP> '/etc/init.d/S99rkipc start'
```

### Method 3: Direct Binary Execution

```bash
# Start RKIPC directly (not recommended)
adb shell "rkipc -a /usr/share/iqfiles &"

# Start with logging
adb shell "rkipc -a /usr/share/iqfiles > /tmp/rkipc.log 2>&1 &"
```

### Verify RKIPC Started

```bash
# Check process
adb shell "ps | grep rkipc | grep -v grep"

# Check RTSP port is listening
adb shell "netstat -tulpn | grep :554"

# Check web port is listening  
adb shell "netstat -tulpn | grep :80"

# Test RTSP stream
ffplay -rtsp_transport tcp rtsp://<BOARD_IP>:554/live/0
```

### Auto-Start on Boot

RKIPC starts automatically on boot via `/etc/init.d/S99rkipc`. The init script:
1. Kills any existing `rkaiq_3A_server` processes
2. Launches `/usr/bin/RkLunch.sh` in background

No additional configuration needed - it works out of the box.

---

## Stopping RKIPC

### Method 1: Simple Kill (Quick)

```bash
# Stop RKIPC immediately
adb shell "killall rkipc"

# Force stop if not responding
adb shell "killall -9 rkipc"

# Verify stopped
adb shell "ps | grep rkipc"
```

### Method 2: Using Stop Script (Graceful)

**Note**: The stock `RkLunch-stop.sh` has issues. Use killall instead.

```bash
# The script attempts graceful shutdown but may hang
adb shell "/usr/bin/RkLunch-stop.sh"
```

### Method 3: Using Init Script

```bash
# Stop via init system
adb shell "/etc/init.d/S99rkipc stop"
```

### Restart RKIPC

```bash
# Quick restart
adb shell "killall rkipc && sleep 2 && /usr/bin/RkLunch.sh &"

# Via init script
adb shell "/etc/init.d/S99rkipc stop && sleep 2 && /etc/init.d/S99rkipc start"
```

---

## Viewing Logs

### Real-Time Console Output

RKIPC logs to stdout/stderr. To capture logs:

```bash
# Method 1: Start with log redirection
adb shell "killall rkipc"
adb shell "/usr/bin/RkLunch.sh > /tmp/rkipc.log 2>&1 &"
adb shell "tail -f /tmp/rkipc.log"

# Method 2: Direct rkipc with logging
adb shell "killall rkipc"
adb shell "rkipc -a /usr/share/iqfiles > /tmp/rkipc.log 2>&1 &"
adb shell "tail -f /tmp/rkipc.log"
```

### View Kernel Messages

```bash
# Check kernel messages (camera, ISP, etc.)
adb shell "dmesg | tail -50"

# Watch for new messages (manual refresh)
adb shell "while true; do clear; dmesg | tail -30; sleep 2; done"

# Check camera sensor detection
adb shell "dmesg | grep -i imx415"

# Check ISP initialization
adb shell "dmesg | grep -i isp"
```

### Log Levels

RKIPC log level is controlled by launch parameter:

```bash
# Log level 2 (default - INFO)
rkipc -a /usr/share/iqfiles

# More verbose logging would require modifying source
```

Rockit (media framework) log level:

```bash
# Set via environment variable before starting
export rt_log_level=6  # 0-6, where 6 is most verbose
/usr/bin/RkLunch.sh &

# Or via runtime file
echo "all=6" > /tmp/rt_log_level
```

### Important Log Messages

When RKIPC starts successfully, you should see:

```
[rkipc.c][main]:main begin
[rkipc.c][main]:rkipc_ini_path_ is (null), rkipc_iq_file_path_ is (null), rkipc_log_level is 2
[param.c][rk_param_init]:g_ini_path_ is /userdata/rkipc.ini
[isp.c][rk_isp_init]:cam_id is 0
[isp.c][rk_isp_init]:g_iq_file_dir_ is /etc/iqfiles
XCORE:K:rk_aiq_init_lib, ISP HW ver: 35
[isp.c][sample_common_isp_init]:ID: 0, sensor_name is m00_b_imx415 1-0010
[video.c][rk_video_init]:begin
[INFO  rtsp_demo.c:280:rtsp_new_demo] rtsp server demo starting on port 554
[rkipc.c][main]:rkipc init over
```

---

## Configuration

### Configuration File Location

```bash
/userdata/rkipc.ini       # Active configuration (user modifiable)
/tmp/rkipc-factory-config.ini  # Symlink to resolution-specific default
/usr/share/rkipc-*.ini    # Factory defaults by resolution
```

### View Current Configuration

```bash
# View full config
adb shell "cat /userdata/rkipc.ini"

# View specific sections
adb shell "cat /userdata/rkipc.ini | grep -A 10 '\[video.0\]'"
adb shell "cat /userdata/rkipc.ini | grep -A 5 '\[system\]'"
```

### Change Configuration

#### Edit Configuration File

```bash
# Connect to device
adb shell  # or: ssh root@<BOARD_IP>

# Edit configuration
vi /userdata/rkipc.ini

# Example changes:
# - Video resolution: width/height under [video.0], [video.1]
# - Frame rate: framerate under [video.0], [video.1]
# - Bitrate: bitrate under [video.0], [video.1]
# - Audio: volume, sample_rate under [audio.0]
# - Admin password: password under [system] (base64 encoded)

# Restart RKIPC to apply changes
killall rkipc
/usr/bin/RkLunch.sh &
```

#### Common Configuration Changes

**Change video resolution:**
```ini
[video.0]
width=1920          # Change from 3840
height=1080         # Change from 2160
framerate=30
bitrate=4096
```

**Change admin password:**
```bash
# Generate base64 password
echo -n "newpassword" | base64
# Output: bmV3cGFzc3dvcmQ=

# Edit config
vi /userdata/rkipc.ini

[system]
password=bmV3cGFzc3dvcmQ=  # Replace with your base64 password
```

**Enable/disable RTSP:**
```ini
[video.source]
enable_rtsp=1  # 1=enabled, 0=disabled
```

### Reset to Factory Defaults

```bash
# Remove user config to trigger factory reset
adb shell "rm /userdata/rkipc.ini && reboot"

# Or manually copy from factory
adb shell "cp /usr/share/rkipc-3840x2160.ini /userdata/rkipc.ini"
adb shell "killall rkipc && /usr/bin/RkLunch.sh &"
```

---

## Background: RkLunch.sh

### Purpose

`RkLunch.sh` is the official launcher script that handles all initialization before starting RKIPC. It's more than just starting the binary - it sets up the complete environment.

### What It Does

#### 1. System Readiness Check

```bash
# Waits up to 3 seconds for /userdata to mount
cnt=0
while [ $cnt -lt 30 ]; do
    cnt=$(( cnt + 1 ))
    if mount | grep -w userdata; then
        break
    fi
    sleep .1
done
```

#### 2. Kernel Module Loading

```bash
# Loads media processing kernel modules
cd /ko && sh insmod_ko.sh
# Or: cd /usr/lib/module && sh insmod_ko.sh

# Modules loaded:
# - kmpp.ko: Kernel MPP (Media Process Platform)
# - kmpp_smart.ko: Smart encoding support
# - rockit_osal.ko: OS abstraction layer
# - rockit_base.ko: Rockit base framework
# - rockit.ko: Main Rockit module
```

#### 3. Network Initialization

```bash
# Handles MAC address persistence
ethaddr1=`ifconfig -a | grep "eth.*HWaddr" | awk '{print $5}'`
if [ -f /data/ethaddr.txt ]; then
    ethaddr2=`cat /data/ethaddr.txt`
    if [ $ethaddr1 == $ethaddr2 ]; then
        echo "eth HWaddr cfg ok"
    else
        ifconfig eth0 down
        ifconfig eth0 hw ether $ethaddr2
    fi
else
    echo $ethaddr1 > /data/ethaddr.txt
fi
ifconfig eth0 up && udhcpc -i eth0
```

#### 4. Filesystem Links

```bash
# Creates web interface links
ln -sf /userdata   /usr/www/userdata
ln -sf /media/usb0 /usr/www/usb0
ln -sf /mnt/sdcard /usr/www/sdcard
```

#### 5. Configuration Detection

```bash
# Detects camera resolution and selects appropriate config
resolution=$(grep -o "Size:[0-9]*x[0-9]*" /proc/rkisp-vir0 | cut -d':' -f2)

# Links to resolution-specific default:
# - /usr/share/rkipc-1920x1080.ini
# - /usr/share/rkipc-2688x1520.ini
# - /usr/share/rkipc-3200x1800.ini
# - /usr/share/rkipc-3840x2160.ini

ln -s -f /usr/share/rkipc-$resolution.ini /tmp/rkipc-factory-config.ini
```

#### 6. Configuration Management

```bash
# Checks if factory config changed (via MD5)
tmp_md5=/tmp/.rkipc-ini.md5sum
data_md5=/userdata/.rkipc-default.md5sum
md5sum $default_rkipc_ini > $tmp_md5
chk_rkipc=`cat $tmp_md5|awk '{print $1}'`

# If MD5 doesn't match, reset user config
grep -w $chk_rkipc $data_md5
if [ $? -ne 0 ]; then
    rm -f $rkipc_ini
    echo "$chk_rkipc" > $data_md5
fi

# Copy factory to user if not exists
if [ ! -f "$rkipc_ini" ]; then
    cp $default_rkipc_ini $rkipc_ini -f
fi
```

#### 7. Core Dumps

```bash
# Enable unlimited core dumps
ulimit -c unlimited
echo "/data/core-%p-%e" > /proc/sys/kernel/core_pattern
```

#### 8. Start RKIPC

```bash
# Start with IQ files if available
if [ -d "/usr/share/iqfiles" ]; then
    rkipc -a /usr/share/iqfiles &
else
    rkipc &
fi
```

### Why Use RkLunch.sh?

**Always use `RkLunch.sh` instead of calling `rkipc` directly because:**

1. **Dependency Management**: Ensures all kernel modules are loaded
2. **Configuration**: Handles resolution detection and config selection
3. **Network Setup**: Initializes ethernet with proper MAC address
4. **Error Handling**: Waits for filesystem mount, handles module conflicts
5. **Consistency**: Factory defaults and upgrades are handled properly

### RkLunch-stop.sh

The stop script attempts graceful shutdown:

```bash
#!/bin/sh
echo "Stop Application ..."
killall rkipc
killall udhcpc

# Waits for rkipc to exit
while [ 1 ]; do
    sleep 1
    ps|grep rkipc|grep -v grep
    if [ $? -ne 0 ]; then
        echo "rkipc exit"
        break
    else
        echo "rkipc active"
    fi
done
```

**Note**: This script may hang if rkipc doesn't respond. Use `killall -9 rkipc` for forced stop.

---

## Troubleshooting

### RKIPC Won't Start

**Symptom**: `RkLunch.sh` runs but RKIPC process doesn't appear

**Check:**

```bash
# 1. Verify userdata is mounted
adb shell "mount | grep userdata"

# 2. Check for kernel module errors
adb shell "dmesg | grep -i error | tail -20"

# 3. Verify camera sensor detected
adb shell "dmesg | grep -i imx415"
adb shell "ls -l /dev/video*"

# 4. Check configuration file exists
adb shell "ls -la /userdata/rkipc.ini"

# 5. Try manual start with logging
adb shell "rkipc -a /usr/share/iqfiles > /tmp/rkipc.log 2>&1 &"
adb shell "cat /tmp/rkipc.log"
```

**Solutions:**

```bash
# Reset configuration
adb shell "rm /userdata/rkipc.ini && /usr/bin/RkLunch.sh &"

# Check IQ files
adb shell "ls -la /etc/iqfiles/"
adb shell "ls -la /usr/share/iqfiles/"

# Reboot device
adb reboot
```

### RKIPC Crashes Immediately

**Symptom**: Process starts but exits immediately

**Check logs:**

```bash
# Start with logging
adb shell "killall rkipc"
adb shell "rkipc -a /usr/share/iqfiles > /tmp/rkipc.log 2>&1 &"
adb shell "sleep 2 && cat /tmp/rkipc.log"

# Check for core dump
adb shell "ls -la /data/core-*"
```

**Common causes:**

1. **Missing IQ files**: Check `/etc/iqfiles/` exists
2. **Wrong camera sensor**: IQ files must match sensor (IMX415)
3. **Corrupted config**: Reset `/userdata/rkipc.ini`
4. **Memory issues**: Check `dmesg` for OOM errors

### RTSP Stream Not Working

**Symptom**: RKIPC runs but no RTSP stream available

**Check:**

```bash
# Verify RKIPC is running
adb shell "ps | grep rkipc"

# Check RTSP port
adb shell "netstat -tulpn | grep :554"

# Check configuration
adb shell "cat /userdata/rkipc.ini | grep enable_rtsp"

# Test locally
adb shell "ffmpeg -rtsp_transport tcp -i rtsp://localhost:554/live/0 -frames:v 1 /tmp/test.jpg"
```

**Solutions:**

```bash
# Enable RTSP in config
adb shell "sed -i 's/enable_rtsp.*=.*/enable_rtsp = 1/' /userdata/rkipc.ini"
adb shell "killall rkipc && /usr/bin/RkLunch.sh &"

# Check firewall (if enabled)
adb shell "iptables -L -n | grep 554"
```

### Configuration Changes Not Applied

**Symptom**: Edited `/userdata/rkipc.ini` but changes don't take effect

**Solution:**

```bash
# Must restart RKIPC after config changes
adb shell "killall rkipc && sleep 2 && /usr/bin/RkLunch.sh &"

# Verify config is read
adb shell "cat /userdata/rkipc.ini | grep -A 5 '\[video.0\]'"
```

**Note**: Some parameters may have limits enforced by hardware or ISP.

### High CPU Usage

**Symptom**: RKIPC consuming excessive CPU

**Check:**

```bash
# Monitor CPU usage
adb shell "top -b -n 1 | grep rkipc"

# Check video encoding settings
adb shell "cat /userdata/rkipc.ini | grep -E 'width|height|framerate|bitrate'"
```

**Solutions:**

```bash
# Reduce resolution or framerate
# Edit /userdata/rkipc.ini:
[video.0]
width=1920       # Lower from 3840
height=1080      # Lower from 2160
framerate=25     # Lower from 30

# Restart RKIPC
adb shell "killall rkipc && /usr/bin/RkLunch.sh &"
```

### Camera Not Detected

**Symptom**: RKIPC fails to initialize camera

**Check:**

```bash
# Verify camera sensor
adb shell "dmesg | grep -i imx415"

# Check video devices
adb shell "ls -l /dev/video*"

# Check ISP device
adb shell "cat /proc/rkisp-vir0"
```

**Expected output:**

```
dmesg: imx415 1-001a: driver version: 00.01.02
       imx415 1-001a: mode: 3840x2160
```

**Solutions:**

1. Check camera cable connection
2. Verify power supply is adequate (5V/2A minimum)
3. Check device tree configuration for camera
4. Reboot device: `adb reboot`

---

## Quick Reference

### Start Commands

```bash
# Standard start
adb shell "/usr/bin/RkLunch.sh &"

# With logging
adb shell "killall rkipc; rkipc -a /usr/share/iqfiles > /tmp/rkipc.log 2>&1 &"

# Via init script
adb shell "/etc/init.d/S99rkipc start"
```

### Stop Commands

```bash
# Simple stop
adb shell "killall rkipc"

# Force stop
adb shell "killall -9 rkipc"

# Via init script
adb shell "/etc/init.d/S99rkipc stop"
```

### Status Commands

```bash
# Check if running
adb shell "ps | grep rkipc | grep -v grep"

# Check ports
adb shell "netstat -tulpn | grep -E '(:554|:80)'"

# View config
adb shell "cat /userdata/rkipc.ini | head -50"
```

### Log Commands

```bash
# View kernel log
adb shell "dmesg | tail -50"

# Start with logging
adb shell "killall rkipc; rkipc -a /usr/share/iqfiles > /tmp/rkipc.log 2>&1 &"
adb shell "tail -f /tmp/rkipc.log"

# Check camera detection
adb shell "dmesg | grep -i imx415"
```

---

## Additional Resources

- **Source Code**: `~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/app/rkipc/`
- **Configuration Reference**: `~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/app/rkipc/README.md`
- **Build Guide**: See [build-and-flash.md](build-and-flash.md)

---

**Last Updated**: January 23, 2026  
**Tested on**: RV1126BP EVB v1.0 with IMX415 camera sensor  
**RKIPC Version**: IPC 64-bit configuration
