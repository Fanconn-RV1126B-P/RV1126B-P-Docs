# RV1126BP-71 — Explore 4K@60fps Capture and Encoding

**Branch**: `RV1126BP-71-Explore-4K60fps-capture-and-encoding`  
**Commit subject prefix**: `RV1126Bp-71`  
**Status**: Phase 1–3 complete — findings recorded below

---

## Confirmed Results (Phases 1–3, April 5 2026)

> All tests run on board at `root@[internal_ip]`, CPU locked to 1608 MHz (performance governor), RKIPC stopped, VPU idle at start.

### VEPU511 Hardware Ceiling (Phase 2)

| Resolution | Codec | Streams | Measured fps/stream | Combined fps | VEPU Load |
|-----------|-------|---------|----------------------|--------------|-----------|
| 1920×1080 | H.264 | 1 | 99.99 fps | 99.99 fps | ~59% |
| 3840×2160 | H.264 | 1 | 26.69 fps | 26.69 fps | 74% ← CPU bottlenecked |
| 3840×2160 | H.265 | 1 | 26.77 fps | 26.77 fps | 72.5% ← CPU bottlenecked |
| 3840×2160 | H.264 | **2** | 17.95 fps | **35.9 fps** | **99.78% ← VEPU saturated** |
| 3840×2160 | H.265 | **2** | 18.46 fps | **36.9 fps** | **99.77% ← VEPU saturated** |

**VEPU511 hardware ceiling at 4K: ~36 fps (H.264) / ~37 fps (H.265)**

The single-stream result (26.7fps @ 73% VEPU) is CPU-feed limited: the CPU at 1608 MHz cannot allocate and fill 3840×2160 NV12 frames (~12 MB each) fast enough to keep the VEPU fully fed from a single thread. Two parallel threads interleave submissions and saturate the VEPU.

With a real zero-copy DMA-BUF camera feed (no CPU frame fill), the single-stream ceiling approaches the 2-stream combined ceiling (~36fps). But this is still NOT 60fps.

### IMX415 Sensor — Actual Mode Enumeration (Phase 3)

- **Subdev**: `/dev/v4l-subdev4` (I2C 1-0010)
- **Active mode**: SGBRG10_1X10 / 3864×2192 @ **30.000 fps** (300000/10000)
- **Active MIPI link**: index 1 = 446 MHz (= 891 Mbps/lane after DDR), pixel_rate = 356.8 MHz
- **MaxIMX415 pixel_rate capacity**: 950.4 MHz (reported by driver)

Registered frame intervals from `VIDIOC_SUBDEV_ENUM_FRAME_INTERVAL`:

| Format | Resolution | FPS available |
|--------|-----------|--------------|
| SGBRG10_1X10 (10-bit) | 3864×2192 | 30, 30, **20**, **20**, 30, 30, **20**, 30, 30 |
| SGBRG12_1X12 (12-bit) | 3864×2192 | 30, 30, **20**, **20**, 30, 30, **20**, 30, 30 |
| SGBRG12_1X12 (12-bit) | 1944×1097 | 30, 30, **20**, **20**, 30, 30, **20**, 30, 30 |

> **Key finding**: The 20fps entries correspond to HDR_X3 modes (three interleaved exposures — NOT usable as a linear source). **No linear mode above 30fps exists in the current kernel driver at any resolution.**

The minimum `vertical_blanking` = 58 lines (already at minimum = cannot increase fps by reducing blanking within the same mode).

The `link_frequency` V4L2 control is writable (index 3 = 891 MHz was set) but changing it has no runtime effect — a full mode re-select via `media-ctl` is required, and no higher-fps linear mode is registered to select.

### Pipeline Topology Confirmed

```
IMX415 (I2C 1-0010) → rkcif-mipi-lvds (SGBRG10_1X10/3840x2160@1/30)
  → rkisp-isp-subdev (→YUYV8_2X8/3840x2160) → rkisp_mainpath → /dev/video-camera0
```

### Overall Verdict

| Target | Verdict | Reason |
|--------|---------|--------|
| 4K@60fps | ❌ **IMPOSSIBLE** | VEPU ceiling ~36fps AND no >30fps linear sensor mode |
| 4K@50fps | ❌ **IMPOSSIBLE** | VEPU ceiling ~36fps |
| 4K@36fps | ⚠️ **Theoretically reachable** | Would require new kernel sensor mode (lower VTS) AND 100% VEPU at all times |
| 4K@30fps | ✅ **Production-proven** | RKIPC at 85% VEPU; synthetic test 26.7fps (CPU-feed bottleneck adds ~10% extra VEPU headroom with real DMA-BUF camera) |

**4K@60fps on the VEPU511 is fundamentally impossible** — the hardware processes a maximum of ~36fps at 3840×2160 even under ideal conditions (no RC overhead, no I/O, back-to-back parallel submission).

---

---

## Goal

Determine whether the Rockchip RV1126B SoC can capture from the Sony IMX415 sensor and encode video at **4K (3840×2160) @ 60 fps** using the MPP hardware encoder (VEPU511). Document the true performance ceiling across all pipeline bottlenecks and produce a reproducible benchmark suite.

---

## Background & Hypothesis

Existing benchmarks show the VEPU511 encoding 4K H.264 at only ~28 fps. However, that test used a software-generated frame source, and at that point the VEPU was only at **66% utilization** — meaning the **CPU was the bottleneck**, not the encoder hardware.

With a real camera pipeline using zero-copy DMA-BUF memory (sensor → ISP → encoder with no CPU copy), the VEPU can be fed at its true throughput capacity. If the 28 fps ceiling is entirely CPU-feed limited, the hardware may reach closer to 50–60 fps at 4K.

A secondary unknown is the IMX415 sensor: the kernel driver registers no full-resolution linear mode above 33.3 fps. The HDR_X3 modes reach 50 fps, but those deliver three-exposure interleaved frames, not a standard linear stream. This must be investigated.

CPU is at 1.2 GHz (idle governor), max is 1.608 GHz. VEPU confirmed at 480 MHz core / 396 MHz AXI. 
```
ssh root@[internal_ip] "echo '=== CPU freq ===' && cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq && cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq && echo '=== VENC clocks ===' && cat /sys/kernel/debug/clk/clk_summary 2>/dev/null | grep -i venc || grep -r 'venc\|vepu' /sys/kernel/debug/clk/ 2>/dev/null | head -20"
=== CPU freq ===
1200000
1608000
=== VENC clocks ===
    clk_core_vepu                    0       1        0        480000000   0          0     50000      N      21f40000.rkvenc       clk_core                 
                                                                                                              21f40000.rkvenc       clk_core                 
                                                                                                              rkvenc@21f40000       no_connection_id         
                aclk_vepu            0       3        0        396000000   0          0     50000      N      21f40000.rkvenc       aclk_vcodec              
                                                                                                              rkvenc@21f40000       no_connection_id         
                hclk_vepu            0       3        0        148500000   0          0     50000      N      21f40000.rkvenc       hclk_vcodec              
                                                                                                              rkvenc@21f40000       no_connection_id   
```

Set CPU to performance mode before benchmarking
```
ssh root@[internal_ip] "for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do echo performance > \$cpu; done && echo 'Governor set' && cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"
```

### Key Questions

| # | Question | Answer | Phase |
|---|----------|--------|-------|
| 1 | Is 28fps at 4K a CPU bottleneck or a VEPU hardware limit? | **CPU bottleneck** — VEPU was only 73% loaded; 2-stream test hit 100% at 35.9fps combined | ✅ Phase 2 |
| 2 | What is the true VEPU511 ceiling at 4K (fps at 100% VPU load)? | **~36fps H.264 / ~37fps H.265** | ✅ Phase 2 |
| 3 | Does the IMX415 expose a >33fps linear mode at 3864×2192 on this board? | **No** — only 30fps (linear) and 20fps (HDR_X3) registered at any resolution | ✅ Phase 3 |
| 4 | Can a real camera zero-copy pipeline deliver frames faster than 30fps to the encoder? | **No** — sensor driver limits to 30fps linear; 30fps is the cap regardless of DMA-BUF path | ✅ Phase 3 |
| 5 | Is DDR bandwidth a secondary ceiling at higher bitrates? | Pending — Phase 6 bitrate sweep | ⏳ Phase 6 |
| 6 | Is 4K@60fps thermally sustainable for 10 minutes? | **Moot** — 4K@60fps is impossible; 4K@30fps endurance not yet tested | ⏳ Phase 6 |

---

## Hardware Context

### SoC

| Component | Spec |
|-----------|------|
| CPU | Quad-core ARM Cortex-A53 @ 1.5 GHz (DVFS up to 1.9 GHz) + RISC-V @ 300 MHz |
| NPU | 3 TOPS (INT8) |
| Encoder HW | VEPU511, core @ 480 MHz, AXI @ 396 MHz |
| Decoder HW | VDPU384A, AXI @ 297 MHz |
| Frame parallelism | **Not supported** (VEPU511 — only RK3588/RK3576 support this) |
| Memory | 4 GB DDR4 |
| ISP | RKISP, dedicated AI-ISP (independent of NPU) |

### IMX415 Sensor — Driver-Registered Modes

**4-lane configuration** (`supported_modes[]` in `kernel-6.1/drivers/media/i2c/imx415.c`):

| Mode | Width | Height | FPS | HDR | BPP | MIPI Freq |
|------|-------|--------|-----|-----|-----|-----------|
| 0 | 3864 | 2192 | 30 | NO_HDR | 10 | 891 MHz |
| 1 | 3864 | 2192 | 30 | HDR_X2 | 10 | 1485 MHz |
| 2 | 3864 | 2192 | **50** | HDR_X3 | 10 | 1485 MHz |
| 3 | 3864 | 2192 | **50** | HDR_X3 | 10 | 1782 MHz |
| 4 | 3864 | 2192 | 30 | NO_HDR | 12 | 891 MHz |
| 5 | 3864 | 2192 | 30 | HDR_X2 | 12 | 1782 MHz |
| 6 | 3864 | 2192 | **50** | HDR_X3 | 12 | 1782 MHz |
| 7 | 1944 | 1097 | 30 | NO_HDR | 12 | 594 MHz |
| 8 | 1944 | 1097 | 30 | HDR_X2 | 12 | 891 MHz |

**2-lane configuration** (`supported_modes_2lane[]`):

| Mode | Width | Height | FPS | HDR | BPP | MIPI Freq |
|------|-------|--------|-----|-----|-----|-----------|
| 0 | 3864 | 2192 | **66.7** | NO_HDR | 12 | 891 MHz |
| 1 | 1284 | 720 | 11.1 | NO_HDR | 12 | 2376 MHz |

> **Note**: No kernel-registered linear mode exists at >33fps for 3864×2192. The 50fps modes are HDR_X3 (3-exposure interleaved). The 2-lane 66.7fps mode is anomalous — its bandwidth math needs verification and it is likely not wired on this board variant.

### Known Benchmark Results (from `HARDWARE_TEST_RESULTS.md`)

| Resolution | Codec | Source | FPS | VEPU Load |
|-----------|-------|--------|-----|-----------|
| 1280×720 | H.264 | testsrc2 (SW) | 203 fps | 56% |
| 1920×1080 | H.264 | testsrc2 (SW) | 99 fps | 59% |
| 3840×2160 | H.264 | testsrc2 (SW) | **28 fps** | 66% |
| 3840×2160 | H.265 | testsrc2 (SW) | **27 fps** | 66% |

RKIPC running 4K@30fps H.265 consumes **85% VEPU**. VPU clock is stable at 480 MHz core.

---

## Implementation Plan

### Phase 1 — Baseline & Environment Setup ✅ COMPLETE

**Goal**: Clean environment, confirm existing baseline matches known numbers before any new tests.

**Steps**:
1. Stop RKIPC service and confirm VPU is idle:
   ```bash
   systemctl stop rkipc
   echo 1000 > /proc/mpp_service/load_interval
   cat /proc/mpp_service/load  # expect 0.00%
   ```
2. Check/set CPU and VEPU clocks to max:
   ```bash
   cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
   cat /sys/kernel/debug/clk/clk_summary | grep -i venc
   ```
3. Cross-compile `mpi_enc_test` from `mpp/test/mpi_enc_test.c` using the SDK toolchain and deploy to the board.
4. Run baseline H.264 and H.265 at 1080p — confirm ~99fps H.264, ~59fps H.265 decode.
5. Run baseline 4K H.264 — confirm ~28fps to match `HARDWARE_TEST_RESULTS.md`.

**Actual results (April 5 2026)**:
- RKIPC was not loaded; VPU confirmed at 0.00%
- CPU set to performance governor → **1608 MHz** (max for this board)
- VEPU511 confirmed: core 480 MHz, AXI 396 MHz, hclk 148.5 MHz
- `mpi_enc_test` found pre-installed at `/usr/bin/mpi_enc_test` — no cross-compile needed
- CLI note: `-rc` takes integers (0=VBR, 1=CBR, 2=FIXQP); `-t 7`=H.264, `-t 16777220`=H.265
- 1080p H.264 FIXQP: **99.99 fps** ✅ (matches documented baseline)
- 4K H.264 FIXQP single stream: **26.69 fps @ 74% VEPU** (doc baseline: 28fps/66% via GStreamer CBR — difference expected due to different source path)
- 4K H.265 FIXQP single stream: **26.77 fps @ 72.5% VEPU**

**Pass criteria**: ✅ Met — numbers consistent with documented baseline (within expected deviation for different source method).

---

### Phase 2 — Encoder Hardware Ceiling (Synthetic Source, DMA-BUF) ✅ COMPLETE

**Goal**: Isolate the VEPU511 hardware ceiling at 4K by eliminating the CPU frame-feed bottleneck. Use pre-allocated DMA-BUF buffers fed directly to the encoder without CPU copies.

**Steps**:
6. Run `mpi_enc_test` at 4K with FIXQP mode (disables rate-control CPU overhead):
   ```bash
   # H.264 FIXQP, unlimited fps input
   mpi_enc_test -w 3840 -h 2160 -f 0 -t 7 -n 300 -rc 2 -v f
   # H.265 FIXQP
   mpi_enc_test -w 3840 -h 2160 -f 0 -t 16777220 -n 300 -rc 2 -v f
   ```
7. Sweep fps inputs — observe at what target fps the actual output fps stops increasing (VEPU at 100%).
8. Record VEPU load at each point from `/proc/mpp_service/load`.

**Actual results (April 5 2026)**:
- `-fps` flag changes PTS timestamps only — no effect on actual encode throughput
- `-l` loop flag with `-n 1` only processed 1 frame, not useful for throughput
- **Single stream is CPU-feed limited** at this resolution. The CPU at 1608 MHz cannot fill and submit 12 MB NV12 frames fast enough to saturate the VEPU from one thread.  
- **Key test: `-s 2` parallel instances**:  
  - H.264: 17.95fps × 2 = **35.9 fps combined at 99.78% VEPU** ← true hardware ceiling  
  - H.265: 18.46fps × 2 = **36.9 fps combined at 99.77% VEPU** ← true hardware ceiling  
- H.265 is marginally faster than H.264 at 4K on VEPU511 (better entropy coding HW for HEVC)

**Pass criteria**: ✅ VEPU saturation confirmed. True 4K ceiling = **~36fps (H.264) / ~37fps (H.265)**. Any pipeline delivering frames faster than this will hit the encoder, not the CPU, as bottleneck.

---

### Phase 3 — Sensor & ISP Throughput Probe ✅ COMPLETE

**Goal**: Determine what fps the IMX415 actually exposes on this board via V4L2 and whether any >33fps linear mode is accessible.

**Steps**:
9. Enumerate sensor modes:
   ```bash
   v4l2-ctl -d /dev/v4l-subdev4 --list-subdev-frameintervals pad=0,width=3864,height=2192,code=0x300e
   ```
10. Check current vblank range:
    ```bash
    v4l2-ctl -d /dev/v4l-subdev4 --list-ctrls-menus
    v4l2-ctl -d /dev/v4l-subdev4 --get-subdev-fps
    ```
11. Check MIPI link frequency and pipeline via media-ctl:
    ```bash
    media-ctl -p -d /dev/media2
    ```
12. Record active sensor mode at boot (`dmesg | grep imx415`).

**Actual results (April 5 2026)**:
- IMX415 is `/dev/v4l-subdev4` (I2C 1-0010); active mode confirmed via `dmesg`
- Active format: **SGBRG10_1X10/3864×2192 @ 30.000fps** (VTS=2250, HTS=8800)
- `vertical_blanking` min=58 = already at minimum — **cannot increase fps by adjusting vblank**
- Current `link_frequency` = index 1 = **446 MHz** (= 891 Mbps/lane DDR); `pixel_rate` = 356.8 MHz; max pixel_rate = 950.4 MHz
- `link_frequency` control is writable but changing it has no runtime effect without a mode re-select
- Registered frame intervals for 3864×2192 (10-bit and 12-bit):
  - **30fps** (NO_HDR linear modes — modes 0, 1, 4, 5, 7 from driver table)
  - **20fps** (HDR_X3 three-exposure modes — useless as linear source)
- Half-resolution 1944×1097: also limited to 30fps linear
- **No linear sensor mode >30fps exists in the current SDK kernel driver at any resolution**
- Pipeline: `rkcif-mipi-lvds [SGBRG10_1X10/3840x2160@1/30]` → `rkisp-isp-subdev` → `rkisp_mainpath`

**Pass criteria**: ✅ Sensor ceiling confirmed. Max fps from hardware with current driver = **30fps linear at full resolution**. Higher fps requires new register table in `imx415.c` kernel driver (Phase 5 stretch).

---

### Phase 4 — Real Camera Zero-Copy Pipeline Benchmark

**Goal**: Test the full pipeline (sensor → ISP → encoder) with zero-copy DMA-BUF at progressively higher frame rates. This is the primary 4K@60fps feasibility test.

#### 4a — 1080p GStreamer Real-Camera Baseline ✅ COMPLETE (April 11 2026)

Confirmed working pipeline using `05_rtsp_stream.py` (Example RV1126BP-64):

```
v4l2src device=/dev/video-camera0
  ! video/x-raw,format=NV12,width=1920,height=1080,framerate=30/1
  ! mpph264enc bps=4000000 rc-mode=cbr
  ! h264parse
  ! rtspclientsink location=rtsp://127.0.0.1:8554/live protocols=tcp
```

MPP init log confirms encoder at 4 Mbps CBR, 30/1, GOП 30:
```
mpp_enc: MPP_ENC_SET_RC_CFG bps 4000000 [3750000 : 4250000] fps [30:30] gop 30
h264e_api_v2: MPP_ENC_SET_PREP_CFG w:h [1920:1080] stride [1920:1088]
mpp_enc: mode cbr bps [3750000:4000000:4250000] fps fix [30/1] -> fix [30/1] gop i [30] v [0]
```

Verified from dev host:
```
ffprobe: h264  1920×1080  30/1 fps  rtsp://192.168.1.95:8554/live ✅
```

**Observations (benign, no impact):**
- `libv4l2: error getting pixformat: Invalid argument` — the ISP vnode `/dev/video-camera0` is a multiplanar (mplane) capture device; libv4l2's userspace ioctl shim attempts `VIDIOC_G_FMT` on every open, which fails on mplane devices. GStreamer switches to the mplane plugin and proceeds correctly (`Using mplane plugin for capture`).
- `mpp: unable to create enc vp8 for soc rv1126b unsupported` — MPP scans all available codec types on init; vp8 encode is unsupported on RV1126B, the warning is printed once and ignored.
- `rga_api version 1.10.5_[4] … deprecated API` — `h264parse` or the internal DRM allocator touches the legacy im2d RGA path; has no effect on encode performance.

#### 4b — 4K Encoder Benchmark (Real Camera DMA-BUF)

**New file**: `RV1126B-P-Camera-Pipeline/examples/03_mpp_encoder_benchmark.c`  
**New script**: `RV1126B-P-Camera-Pipeline/scripts/benchmark_encoder.sh`

**Steps**:
13. Build `03_mpp_encoder_benchmark.c` — a GStreamer-based pipeline that:
    - Accepts `-w/-H/-r/-c/-b/-t` arguments (resolution, fps, codec, bitrate, duration)
    - Enables `disable-copy=true` on the MPP encoder element for zero-copy DMA-BUF
    - Prints per-second encoded fps, dropped-frame count, and polls `/proc/mpp_service/load`
    - Writes a summary CSV at exit
14. Run benchmarks against real camera device `/dev/video-camera0`:
    ```bash
    # Baseline
    ./03_mpp_encoder_benchmark -w 3840 -H 2160 -r 30 -c h265 -b 8000000 -t 60
    # Push fps
    ./03_mpp_encoder_benchmark -w 3840 -H 2160 -r 45 -c h265 -b 8000000 -t 60
    ./03_mpp_encoder_benchmark -w 3840 -H 2160 -r 60 -c h265 -b 8000000 -t 60
    # H.264 comparison
    ./03_mpp_encoder_benchmark -w 3840 -H 2160 -r 60 -c h264 -b 8000000 -t 60
    ```
15. If 60fps fails, binary search for the maximum stable fps (0 dropped frames over 60 seconds).
16. GStreamer fakesink test (no file I/O to eliminate storage bottleneck):
    ```bash
    gst-launch-1.0 v4l2src device=/dev/video-camera0 \
      ! video/x-raw,format=NV12,width=3840,height=2160,framerate=60/1 \
      ! mpph265enc bps=8000000 rc-mode=vbr disable-copy=true \
      ! fakesink sync=false
    ```

**Pass criteria**:
- ✅ 4K@60fps with 0 dropped frames → goal achieved
- ⚠️ 4K@60fps with <5% dropped frames → partial success, note conditions  
- ❌ >5% dropped frames at 60fps → document max stable fps instead

---

### Phase 5 — ISP/Sensor Configuration Push *(stretch, parallel with Phase 4)*

**Goal**: If Phase 3 shows the sensor/MIPI is limiting, investigate driver/DTS changes to enable higher sensor fps.

**Steps** (only if sensor is the bottleneck found in Phase 3):
17. Attempt to enable IMX415 HDR_X3 mode at 50fps and use a single exposure output (not the merged HDR frame) as a 50fps linear source:
    ```bash
    media-ctl -d /dev/media0 --set-v4l2 '"m00_b_imx415 3-001a":0[fmt:SGBRG10_1X10/3864x2192@1/20]'
    ```
18. If 2-lane 66.7fps mode (mode index 0 in `supported_modes_2lane`) is physically wired on this board, test it:
    ```bash
    # Check if 2-lane CSI path is accessible
    ls /dev/media*
    media-ctl -p -d /dev/media1  # or media0
    ```
19. **Stretch**: If a new sensor mode is needed, add a custom `imx415_linear_10bit_3864x2192_60fps` register table to `kernel-6.1/drivers/media/i2c/imx415.c` and rebuild the kernel. This requires:
    - Calculating new VTS/HTS / VMAX for 60fps at 891 MHz or 1188 MHz link
    - Rebuilding kernel, flashing `boot.img`
    - Re-running Phase 4

---

### Phase 6 — Bitrate, Memory Bandwidth & Endurance

**Goal**: Stress test at the highest achievable fps to confirm DDR bandwidth is not a secondary ceiling, and that thermal/endurance is stable.

**Steps**:
20. Bitrate sweep at max stable fps: 4 Mbps → 8 Mbps → 16 Mbps → 32 Mbps — confirm fps does not decrease as bitrate increases (i.e., DDR bandwidth is not saturated).
21. NPU co-load test: run a model at 10fps inference simultaneously with the max-fps encoding pipeline — record any fps degradation.
22. **10-minute endurance run** at the highest proven stable fps:
    ```bash
    ./03_mpp_encoder_benchmark -w 3840 -H 2160 -r <MAX_FPS> -c h265 -b 8000000 -t 600
    # Monitor simultaneously:
    watch -n1 'cat /proc/mpp_service/load; cat /sys/class/thermal/thermal_zone*/temp'
    ```

**Pass criteria**: 0 dropped frames, VPU load stable, temperature < 80°C throughout 10 minutes.

---

### Phase 7 — Document Results

**Goal**: Create reproducible benchmark documentation and a recommended configuration snippet.

**Steps**:
23. Create `RV1126B-P-Docs/guides/mpp-encoder-benchmark.md` with:
    - Results matrix: resolution × fps × codec × bottleneck × VEPU load
    - Comparison table: software source vs zero-copy camera pipeline
    - Maximum stable configuration for production use
    - Known limitations
24. Update `RV1126B-P-Camera-Pipeline/README.md` with the max proven fps capability.
25. Add `RV1126B-P-Camera-Pipeline/scripts/benchmark_encoder.sh` — a single-command benchmark runner for field verification.

---

## Files Produced by This Work

| File | Phase | Purpose |
|------|-------|---------|
| `RV1126B-P-Camera-Pipeline/examples/03_mpp_encoder_benchmark.c` | 4 | Zero-copy camera benchmark program |
| `RV1126B-P-Camera-Pipeline/examples/03_mpp_encoder_benchmark.py` | 4 | Python equivalent |
| `RV1126B-P-Camera-Pipeline/scripts/benchmark_encoder.sh` | 4, 7 | Shell benchmark runner |
| `RV1126B-P-Docs/guides/mpp-encoder-benchmark.md` | 7 | Results & recommendations doc |
| `kernel-6.1/drivers/media/i2c/imx415.c` | 5 (stretch) | New 60fps sensor mode (if needed) |

---

## Key Reference Files

| File | Purpose |
|------|---------|
| [`mpp/test/mpi_enc_test.c`](../../mpp/test/mpi_enc_test.c) | Primary synthetic encoder benchmark |
| [`mpp/utils/mpi_enc_utils.h`](../../mpp/utils/mpi_enc_utils.h) | mpi_enc_test CLI arg definitions |
| [`RV1126B-P-Camera-Pipeline/examples/02_gstreamer_capture_with_args.c`](../../RV1126B-P-Camera-Pipeline/examples/02_gstreamer_capture_with_args.c) | Reference implementation for benchmark program |
| [`kernel-6.1/drivers/media/i2c/imx415.c`](../../RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/kernel-6.1/drivers/media/i2c/imx415.c) | IMX415 sensor modes, lines 1085–1360 |
| [`RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/docs/en/Common/MPP/Rockchip_Developer_Guide_MPP_EN.md`](../../RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/docs/en/Common/MPP/Rockchip_Developer_Guide_MPP_EN.md) | MPP developer reference |

---

## Risk Register

| Risk | Likelihood | Mitigation |
|------|------------|------------|
| VEPU511 hardware ceiling is <60fps at 4K | Medium | Document max fps instead; 50fps (HDR_X3 mode) may be achievable |
| IMX415 has no linear >33fps mode in this SDK | High | Investigate HDR_X3 as high-fps source; or add custom register table (Phase 5 stretch) |
| DDR bandwidth saturates at high bitrate + high fps | Low | Test in Phase 6; reduce bitrate or use more aggressive VBR |
| Thermal throttling under sustained 4K encoding | Low | VPU runs at 66% load at 4K@30fps; headroom exists |
| Kernel rebuild required for sensor mode change | Medium | Phase 5 step 19 — planned but classified as stretch |
| 2-lane 66.7fps mode is not wired on this board variant | High | Board uses 4-lane CSI2; 2-lane path likely absent |

---

## Definition of Success

| Outcome | Status | Description |
|---------|--------|-------------|
| 🏆 Full success | ❌ Impossible | 4K@60fps, 0 dropped frames — VEPU ceiling is ~36fps |
| ✅ Partial success | ❌ Impossible | 4K@50fps or 4K@45fps — VEPU ceiling is ~36fps |
| 📊 Informative outcome | ✅ **Achieved** | True ceiling documented: VEPU ~36fps, sensor 30fps, both bottlenecks confirmed |
| 🔧 Revised goal | **Next step** | Explore 4K@33–36fps via kernel sensor mode addition (Phase 5 stretch) |

## Next Steps (post Phase 1–3)

Given the confirmed findings, the recommended path forward is:

1. **Phase 4 (modified scope)** — Run the real camera zero-copy GStreamer pipeline at 4K@30fps and compare its VEPU load (expected: ~36fps possible at 100% VEPU with DMA-BUF vs 26.7fps at 73% with CPU frames). Create `03_mpp_encoder_benchmark.c`.

2. **Phase 5 stretch** — Add a custom `imx415_linear_10bit_3864x2192_33fps` register table to the sensor driver. The VTS can be reduced slightly: current VTS=2250; for 33fps at same pixel clock: VTS = 356.8M / (33 × 8800) = ~1229. This requires rebuilding the kernel. Even at 36fps the encoder would only just manage it at 100% VEPU.

3. **Alternative target** — Consider **2688×1520 @ 60fps**: 2688×1520 is ~39% of 4K pixels. At the confirmed 36fps ceiling for 4K, the ceiling for 2688×1520 would be approximately 36 × (3840×2160) / (2688×1520) ≈ **73fps**. The sensor also has no 60fps mode at any resolution currently, so the same driver modification approach applies — but the VEPU could handle it.

