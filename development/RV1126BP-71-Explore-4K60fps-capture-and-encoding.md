# RV1126BP-71 — Explore 4K@60fps Capture and Encoding

**Branch**: `RV1126BP-71-Explore-4K60fps-capture-and-encoding`  
**Commit subject prefix**: `RV1126Bp-71`  
**Status**: Planning

---

## Goal

Determine whether the Rockchip RV1126B SoC can capture from the Sony IMX415 sensor and encode video at **4K (3840×2160) @ 60 fps** using the MPP hardware encoder (VEPU511). Document the true performance ceiling across all pipeline bottlenecks and produce a reproducible benchmark suite.

---

## Background & Hypothesis

Existing benchmarks show the VEPU511 encoding 4K H.264 at only ~28 fps. However, that test used a software-generated frame source, and at that point the VEPU was only at **66% utilization** — meaning the **CPU was the bottleneck**, not the encoder hardware.

With a real camera pipeline using zero-copy DMA-BUF memory (sensor → ISP → encoder with no CPU copy), the VEPU can be fed at its true throughput capacity. If the 28 fps ceiling is entirely CPU-feed limited, the hardware may reach closer to 50–60 fps at 4K.

A secondary unknown is the IMX415 sensor: the kernel driver registers no full-resolution linear mode above 33.3 fps. The HDR_X3 modes reach 50 fps, but those deliver three-exposure interleaved frames, not a standard linear stream. This must be investigated.

### Key Questions

| # | Question | How Answered |
|---|----------|-------------|
| 1 | Is 28 fps at 4K a CPU bottleneck or a VEPU hardware limit? | Phase 2 synthetic benchmark with DMA-BUF input |
| 2 | What is the true VEPU511 ceiling at 4K (fps at 100% VPU load)? | Phase 2 fps sweep |
| 3 | Does the IMX415 expose a >33fps linear mode at 3864×2192 on this board? | Phase 3 `v4l2-ctl` enumeration |
| 4 | Can a real camera zero-copy pipeline deliver frames faster than 30 fps to the encoder? | Phase 4 GStreamer pipeline test |
| 5 | Is DDR bandwidth a secondary ceiling at higher bitrates? | Phase 6 bitrate sweep |
| 6 | Is 4K@60fps thermally sustainable for 10 minutes? | Phase 6 endurance test |

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

### Phase 1 — Baseline & Environment Setup

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

**Pass criteria**: Numbers within ±5% of documented baseline.

---

### Phase 2 — Encoder Hardware Ceiling (Synthetic Source, DMA-BUF)

**Goal**: Isolate the VEPU511 hardware ceiling at 4K by eliminating the CPU frame-feed bottleneck. Use pre-allocated DMA-BUF buffers fed directly to the encoder without CPU copies.

**Steps**:
6. Run `mpi_enc_test` at 4K with FIXQP mode (disables rate-control CPU overhead):
   ```bash
   # H.264 FIXQP, unlimited fps input
   mpi_enc_test -w 3840 -h 2160 -f 0 -t H264 -n 300 --rc fixqp -q 28
   # H.265 FIXQP
   mpi_enc_test -w 3840 -h 2160 -f 0 -t H265 -n 300 --rc fixqp -q 28
   ```
7. Sweep fps inputs `--fps-in-num 30 45 60 90` with `--fps-out-num` matching — observe at what target fps the actual output fps stops increasing (VEPU at 100%).
8. Record VEPU load at each point from `/proc/mpp_service/load`.

**Pass criteria**: Identify fps at which VEPU load reaches 100% — that is the true hardware ceiling.

---

### Phase 3 — Sensor & ISP Throughput Probe

**Goal**: Determine what fps the IMX415 actually exposes on this board via V4L2 and whether any >33fps linear mode is accessible.

**Steps**:
9. Enumerate sensor modes:
   ```bash
   v4l2-ctl --device /dev/v4l-subdev0 --list-formats-ext
   v4l2-ctl -d /dev/v4l-subdev0 --list-framesizes pixel=SGBRG10P
   v4l2-ctl -d /dev/v4l-subdev0 --list-framesizes pixel=SGBRG12P
   ```
10. Check current vblank range (controls the actual sensor fps):
    ```bash
    v4l2-ctl -d /dev/v4l-subdev0 --list-ctrls | grep -i vblank
    v4l2-ctl -d /dev/v4l-subdev0 --get-ctrl=vblank
    ```
11. Check if MIPI link frequency can be raised for the board: inspect active DTS / DTB:
    ```bash
    # Examine the compiled DTB for imx415 link-frequencies
    dtc -I dtb -O dts /sys/firmware/fdt 2>/dev/null | grep -A5 -i "imx415\|link-frequencies"
    ```
12. Record active sensor mode at boot (`dmesg | grep imx415`).

**Pass criteria**: Know the maximum fps the sensor hardware can deliver at 3864×2192 without driver changes.

---

### Phase 4 — Real Camera Zero-Copy Pipeline Benchmark

**Goal**: Test the full pipeline (sensor → ISP → encoder) with zero-copy DMA-BUF at progressively higher frame rates. This is the primary 4K@60fps feasibility test.

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

| Outcome | Description |
|---------|-------------|
| 🏆 Full success | 4K@60fps, 0 dropped frames, 10 min thermally stable |
| ✅ Partial success | 4K@50fps or 4K@45fps with 0 dropped frames |
| 📊 Informative failure | Document true ceiling fps and identify primary bottleneck (VEPU / ISP / Sensor / DDR / CPU) |
