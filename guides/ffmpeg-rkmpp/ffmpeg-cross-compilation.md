# FFmpeg-Rockchip Cross-Compilation Guide

Complete guide for cross-compiling FFmpeg with Rockchip MPP and RGA hardware acceleration for the RV1126B-P.

## 📋 Prerequisites

Before starting, ensure you have:

1. ✅ **RV1126B-P SDK** - Built and ready
2. ✅ **MPP v1.0.11** - Confirmed built (see [MPP Status](./mpp-version-and-status.md))
3. ✅ **RGA v2.1.0** - Confirmed built (see [RGA Status](./rga-version-and-status.md))
4. ✅ **FFmpeg-Rockchip** - Cloned to `/home/csvke/RV1126B-P/ffmpeg-rockchip/`
5. ✅ **Cross-Compilation Toolchain** - Available in SDK buildroot output

---

## 🎯 Why Cross-Compilation?

### Option Comparison

| Method | Speed | Convenience | Use Case |
|--------|-------|-------------|----------|
| **Native Compilation** | Very Slow (~4-6 hours) | Easy | Testing only |
| **Cross-Compilation** | Fast (~10-15 min) | Requires setup | Development ⭐ |
| **Buildroot Integration** | Medium (~30 min) | Complex | Production |

**Cross-compilation is the recommended approach** for development because:
- ✅ **Fast builds** on your development machine
- ✅ **No SDK rebuild** required when testing FFmpeg changes
- ✅ **Full debug symbols** available for troubleshooting
- ✅ **Iterative development** - quick compile-test cycles

---

## 🛠️ Environment Setup

### Understanding the Cross-Compilation Environment

Cross-compilation requires several key components working together:

1. **Toolchain** - The cross-compiler (e.g., `aarch64-buildroot-linux-gnu-gcc`) that runs on your development machine (x86_64) but generates code for the target (ARM64)

2. **Sysroot** - A directory structure containing the target system's headers and libraries. This is where the compiler looks for dependencies like MPP and RGA during compilation.

3. **pkg-config** - A helper tool that tells the compiler where to find library headers and how to link against them. We configure it to search in the sysroot instead of the host system.

4. **Environment Variables** - These tell the build system (FFmpeg's configure script) how to find and use all these components.

### Step 1: Use the Automated Setup Script

The FFmpeg-Rockchip repository includes an automated environment setup script that detects your SDK location and configures everything correctly.

**Expected Directory Structure:**

```
parent/
├── ffmpeg-rockchip/              # This repository
│   └── ffmpeg-rockchip-cross-compile-env.sh
└── RV1126B-P-SDK/
    └── rv1126b_linux6.1_sdk_v1.1.0/
        └── buildroot/
            └── output/
                └── rockchip_rv1126b/
                    ├── host/              # Toolchain
                    │   └── aarch64-buildroot-linux-gnu/
                    │       └── sysroot/   # Headers and libraries
                    └── target/            # Device rootfs
```

**Load the environment:**

```bash
cd ffmpeg-rockchip
source ./ffmpeg-rockchip-cross-compile-env.sh
```

The script will:
- ✅ Auto-detect the SDK location (assumes it's a sibling directory)
- ✅ Verify the toolchain exists and is functional
- ✅ Check for MPP v1.0.11 libraries and headers
- ✅ Check for RGA v2.1.0 libraries and headers
- ✅ Configure pkg-config for cross-compilation
- ✅ Set all required environment variables

**What the script configures:**

| Variable | Purpose | Example Value |
|----------|---------|---------------|
| `SDK_ROOT` | SDK root directory | `../RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0` |
| `TOOLCHAIN` | Cross-compiler location | `$SDK_ROOT/buildroot/output/.../host` |
| `STAGING` | Sysroot with headers/libs | `$TOOLCHAIN/aarch64-buildroot-linux-gnu/sysroot` |
| `CROSS_PREFIX` | Compiler prefix | `aarch64-buildroot-linux-gnu-` |
| `PKG_CONFIG_*` | Library discovery | Points to sysroot packages |
| `PKG_CONFIG` | pkg-config binary | Full path to pkg-config tool |
| `FFMPEG_PREFIX` | Install destination | `./install` (in ffmpeg-rockchip) |

---

### Step 2: Verify Environment

After sourcing the setup script, verify everything is configured correctly:

```bash
# Check toolchain is in PATH
which ${CROSS_PREFIX}gcc
# Should output: .../host/bin/aarch64-buildroot-linux-gnu-gcc

# Check GCC version
${CROSS_PREFIX}gcc --version
# Should show: aarch64-buildroot-linux-gnu-gcc (Buildroot ...) 11.x or 12.x

# Verify MPP (should show version 1.0.11)
pkg-config --modversion rockchip_mpp

# Verify RGA (should show version 2.1.0)
pkg-config --modversion librga

# Check sysroot libraries exist
ls $STAGING/usr/lib/librockchip_mpp.so
ls $STAGING/usr/lib/librga.so

# Check headers exist
ls $STAGING/usr/include/rockchip/rk_mpi.h
ls $STAGING/usr/include/rga/im2d.h
```

**Why these checks matter:**

- **Toolchain check** - Ensures the cross-compiler is accessible
- **pkg-config checks** - Confirms FFmpeg's configure script can find MPP and RGA
- **Library checks** - Verifies the linker can find runtime dependencies
- **Header checks** - Ensures the compiler can find API definitions

If the automated script passed all checks, these manual verifications should all succeed.

---

## 🔧 FFmpeg Configuration

### Understanding Configure Options

FFmpeg's `./configure` script generates build files based on your requirements. For cross-compilation, we need to tell it:

1. **Target Architecture** - `--arch=aarch64` (ARM 64-bit)
2. **Cross-Compilation Mode** - `--enable-cross-compile` (don't try to run target binaries on host)
3. **Toolchain** - `--cross-prefix=` (prefix for gcc, ld, ar, etc.)
4. **Sysroot** - `--sysroot=` (where to find target headers/libraries)
5. **Hardware Acceleration** - `--enable-rkmpp --enable-rkrga` (Rockchip features)

### Minimal Configuration (Hardware Codecs Only)

This builds FFmpeg with ONLY hardware acceleration support - smallest binary:

```bash
cd $FFMPEG_SRC

./configure \
  --prefix=$FFMPEG_PREFIX \
  --enable-cross-compile \
  --cross-prefix=${CROSS_PREFIX} \
  --arch=aarch64 \
  --target-os=linux \
  --sysroot=$STAGING \
  --pkg-config=$PKG_CONFIG \
  --enable-gpl \
  --enable-version3 \
  --enable-nonfree \
  --enable-libdrm \
  --enable-rkmpp \
  --enable-rkrga \
  --disable-static \
  --enable-shared \
  --disable-doc \
  --disable-htmlpages \
  --disable-manpages \
  --disable-podpages \
  --disable-txtpages
```

**Why each option:**

- `--prefix=$FFMPEG_PREFIX` - Where `make install` will place binaries
- `--pkg-config=$PKG_CONFIG` - Explicit path to pkg-config (required for cross-compilation)
- `--enable-gpl/version3/nonfree` - Licensing (required for some features)
- `--enable-libdrm` - Direct Rendering Manager support (required for MPP/RGA)
- `--enable-rkmpp` - Rockchip MPP hardware codecs
- `--enable-rkrga` - Rockchip RGA hardware filters
- `--disable-static` - Build shared libraries only (saves space)
- `--disable-doc` - Skip documentation generation (faster build)

**Binary size: ~10-15 MB** (stripped)

---

### Recommended Configuration (Balanced)

Includes common encoders, filters, and protocols:

```bash
cd $FFMPEG_SRC

./configure \
  --prefix=$FFMPEG_PREFIX \
  --enable-cross-compile \
  --cross-prefix=${CROSS_PREFIX} \
  --arch=aarch64 \
  --target-os=linux \
  --sysroot=$STAGING \
  --pkg-config=$PKG_CONFIG \
  --enable-gpl \
  --enable-version3 \
  --enable-nonfree \
  --enable-libdrm \
  --enable-rkmpp \
  --enable-rkrga \
  --disable-static \
  --enable-shared \
  --enable-pthreads \
  --enable-network \
  --enable-protocol=file,pipe,tcp,udp,rtp,rtsp,http,https \
  --enable-demuxer=mov,mp4,h264,hevc,matroska,flv,mpegts \
  --enable-muxer=mp4,mov,matroska,flv,mpegts,null \
  --enable-parser=h264,hevc,vp8,vp9,av1 \
  --enable-bsf=h264_mp4toannexb,hevc_mp4toannexb,extract_extradata \
  --enable-filter=scale,format,fps,null \
  --disable-doc \
  --disable-htmlpages \
  --disable-manpages \
  --disable-podpages \
  --disable-txtpages
```

**Binary size: ~15-20 MB** (stripped)

---

### Full Configuration (Development)

Includes everything for maximum flexibility:

```bash
cd $FFMPEG_SRC

./configure \
  --prefix=$FFMPEG_PREFIX \
  --enable-cross-compile \
  --cross-prefix=${CROSS_PREFIX} \
  --arch=aarch64 \
  --target-os=linux \
  --sysroot=$STAGING \
  --pkg-config=$PKG_CONFIG \
  --enable-gpl \
  --enable-version3 \
  --enable-nonfree \
  --enable-libdrm \
  --enable-rkmpp \
  --enable-rkrga \
  --disable-static \
  --enable-shared \
  --enable-pthreads \
  --enable-network \
  --enable-debug \
  --disable-optimizations \
  --disable-stripping \
  --disable-doc
```

**Binary size: ~40-50 MB** (with debug symbols)

---

## 🏗️ Build Process

### Step 1: Configure FFmpeg

Choose one of the configurations above and run it in the FFmpeg source directory:

```bash
# Ensure environment is loaded
if [ -z "$FFMPEG_ROCKCHIP_ENV_LOADED" ]; then
    echo "Environment not loaded! Run: source ./ffmpeg-rockchip-cross-compile-env.sh"
    exit 1
fi

# Go to FFmpeg source
cd $FFMPEG_SRC

# Run configure (minimal example - adjust as needed)
./configure \
  --prefix=$FFMPEG_PREFIX \
  --enable-cross-compile \
  --cross-prefix=${CROSS_PREFIX} \
  --arch=aarch64 \
  --target-os=linux \
  --sysroot=$STAGING \
  --pkg-config=$PKG_CONFIG \
  --enable-gpl \
  --enable-version3 \
  --enable-libdrm \
  --enable-rkmpp \
  --enable-rkrga \
  --disable-static \
  --enable-shared
```

**Check configuration output:**

Look for these lines in the configure output:
```
Enabled decoders:
h264_rkmpp hevc_rkmpp vp8_rkmpp vp9_rkmpp av1_rkmpp

Enabled encoders:
h264_rkmpp hevc_rkmpp

Enabled filters:
scale_rkrga vpp_rkrga overlay_rkrga transpose_rkrga
```

If you see `rkmpp` and `rkrga` components, configuration succeeded! ✅

**If configuration fails:**
- Ensure the environment script was sourced correctly
- Check that pkg-config can find MPP and RGA (see Step 2 verification)
- Review the `ffbuild/config.log` file for detailed error messages

---

### Step 2: Compile FFmpeg

Once configuration succeeds, build FFmpeg using all available CPU cores:

```bash
# Build with parallel compilation (much faster)
make -j$(nproc)
```

**What happens during compilation:**

1. **Header Processing** - Compiler reads MPP/RGA headers from `$STAGING/usr/include`
2. **Source Compilation** - Each `.c` file is compiled to `.o` object files
3. **Linking** - Object files are linked with MPP/RGA libraries from `$STAGING/usr/lib`
4. **Library Generation** - Shared libraries (`.so`) are created for libavcodec, libavformat, etc.
5. **Binary Creation** - Final `ffmpeg` and `ffprobe` executables are generated

**Compilation time:**
- Fast machine (16+ cores): ~5-10 minutes
- Medium machine (8 cores): ~10-15 minutes  
- Slow machine (4 cores): ~20-30 minutes

**Progress indicators:**
- You'll see many `CC` (compile) and `LD` (link) messages scrolling by
- The percentage completion is typically not shown, but you can estimate progress by watching the output
- Warnings are normal; errors will stop the build

**If compilation fails**, see [Troubleshooting](#-troubleshooting) section below.

---

### Step 3: Install FFmpeg

After successful compilation, install the binaries and libraries:

```bash
# Install to the prefix directory specified during configure
make install
```

**What gets installed:**

This creates a complete FFmpeg installation at `$FFMPEG_PREFIX` (default: `./install` in the ffmpeg-rockchip directory):

```
install/
├── bin/
│   ├── ffmpeg          # Main FFmpeg binary (transcoding, processing)
│   ├── ffprobe         # Media file analyzer (metadata, streams)
│   └── ffplay          # Simple media player (if SDL2 was available)
├── lib/
│   ├── libavcodec.so.XX      # Codec library (MPP decoders/encoders)
│   ├── libavformat.so.XX     # Container format handling
│   ├── libavfilter.so.XX     # Filter library (RGA filters)
│   ├── libavutil.so.XX       # Utility functions
│   ├── libswscale.so.XX      # Software scaling (fallback)
│   ├── libswresample.so.XX   # Audio resampling
│   └── pkgconfig/            # For applications linking against FFmpeg
├── include/
│   └── libav*/               # Development headers (if needed)
└── share/
    └── ffmpeg/               # Examples and documentation (if enabled)
```

**Library dependencies:**

The installed FFmpeg binaries depend on:
- **On your host** - Nothing (they won't run on x86_64)
- **On the device** - `librockchip_mpp.so`, `librga.so`, `libdrm.so`, standard C library

These dependencies should already be in the device rootfs if the SDK was built correctly.

---

### Step 4: Verify Build

```bash
# Check built binary architecture
file $FFMPEG_PREFIX/bin/ffmpeg
# Expected: ELF 64-bit LSB executable, ARM aarch64

# Check MPP decoders
$FFMPEG_PREFIX/bin/ffmpeg -decoders | grep rkmpp
# Expected:
# V..... h264_rkmpp         H.264 / AVC / MPEG-4 AVC (Rockchip MPP)
# V..... hevc_rkmpp         HEVC (Rockchip MPP)
# V..... vp8_rkmpp          VP8 (Rockchip MPP)
# V..... vp9_rkmpp          VP9 (Rockchip MPP)

# Check MPP encoders
$FFMPEG_PREFIX/bin/ffmpeg -encoders | grep rkmpp
# Expected:
# V..... h264_rkmpp         H.264 / AVC / MPEG-4 AVC (Rockchip MPP)
# V..... hevc_rkmpp         HEVC (Rockchip MPP)

# Check RGA filters
$FFMPEG_PREFIX/bin/ffmpeg -filters | grep rkrga
# Expected:
# ... scale_rkrga        V->V  Scale video using Rockchip RGA
# ... vpp_rkrga          V->V  Apply HW video post-processing
# ... overlay_rkrga      VV->V Video overlay using RGA
# ... transpose_rkrga    V->V  Transpose/rotate video using RGA
```

**If all checks pass**, your FFmpeg-Rockchip is successfully built! 🎉

---

## 📦 Packaging for Device

### Understanding Deployment Options

Since we cross-compiled FFmpeg on a development machine, we need to transfer it to the target device. There are several approaches:

### Option 1: Create Tarball (Recommended for Testing)

Package the installation into a portable archive:

```bash
# Go to the install directory
cd $FFMPEG_PREFIX

# Create timestamped tarball
tar czf ffmpeg-rv1126b-$(date +%Y%m%d).tar.gz bin/ lib/

# The tarball will be in the install directory
# Size: ~10-20 MB (depending on configuration)
ls -lh ffmpeg-rv1126b-*.tar.gz
```

**Transfer to device:**
```bash
# Replace with your device's IP address
DEVICE_IP=192.168.1.100

# Copy tarball to device
scp ffmpeg-rv1126b-*.tar.gz root@$DEVICE_IP:/tmp/
```

**Why tarball is good for testing:**
- ✅ Easy to transfer and extract
- ✅ Doesn't modify system directories during testing
- ✅ Can install to user-writable location (`/tmp`, `/home`)
- ✅ Easy to remove if something goes wrong

---

### Option 2: Install to Device System Directories

For permanent installation on the device:

```bash
# On device, extract to system location
ssh root@$DEVICE_IP
cd /usr/local
tar xzf /tmp/ffmpeg-rv1126b-*.tar.gz

# Add to PATH permanently
echo 'export PATH=/usr/local/bin:$PATH' >> /etc/profile

# Update library cache (so system can find libavcodec.so, etc.)
ldconfig

# Verify installation
ffmpeg -version
```

**Why install to /usr/local:**
- System directory `/usr` may be read-only or managed by package system
- `/usr/local` is the standard location for locally compiled software
- Won't conflict with any potential system FFmpeg packages
- Survives reboots (unlike `/tmp`)

**Important:** Ensure `/usr/local` has enough space for the installation (~20-50 MB depending on configuration).

---

### Option 3: Direct Install via Network Filesystem

If your device mounts a shared filesystem (NFS, CIFS) from your development machine:

```bash
# Configure FFmpeg to install directly to a shared location
# (Do this BEFORE running ./configure)
export FFMPEG_PREFIX=/path/to/nfs/mount/ffmpeg

# Then configure and build as normal
./configure \
  --prefix=$FFMPEG_PREFIX \
  ... # other options

make -j$(nproc)
make install

# On device, add to PATH
export PATH=/path/to/nfs/mount/ffmpeg/bin:$PATH
export LD_LIBRARY_PATH=/path/to/nfs/mount/ffmpeg/lib:$LD_LIBRARY_PATH
```

**Benefits of network filesystem approach:**
- ✅ No manual file transfer needed
- ✅ Can iterate quickly: rebuild on host, run immediately on device
- ✅ Saves device storage space
- ⚠️ Requires reliable network connection
- ⚠️ Performance may be slower than local installation

---

## 🧪 Testing on Device

### Step 1: Transfer Test Media

Get a test video file onto the device:

```bash
# From your development machine
DEVICE_IP=192.168.1.100

# Copy a test video (H.264 recommended for initial testing)
scp test_video.mp4 root@$DEVICE_IP:/tmp/
```

**Test file recommendations:**
- **Codec:** H.264 or H.265 (HEVC) - supported by MPP hardware
- **Resolution:** 1080p or 720p
- **Duration:** 10-30 seconds (for quick tests)
- **Container:** MP4 or MKV

---

### Step 2: Test Hardware Decoder

SSH into the device and test MPP hardware decoding:

```bash
# Connect to device
ssh root@$DEVICE_IP

# Test H.264 hardware decoding (output to null sink for benchmarking)
ffmpeg -c:v h264_rkmpp -i /tmp/test_video.mp4 -f null -

# Expected output shows frame processing:
# frame=  300 fps=240 q=-0.0 Lsize=N/A time=00:00:10.00 bitrate=N/A speed=8.0x
```

**What to look for:**

- **`fps=240` or higher** - Indicates hardware decoding is working! 🚀
- **`speed=8.0x` or higher** - Processing faster than real-time
- **No error messages** - MPP initialized successfully

**If FPS is low (< 30):**
- May be using software decoder instead of hardware
- Check for error messages about MPP initialization
- Verify `/dev/mpp_service` device exists: `ls -l /dev/mpp_service`

**Understanding the output:**
- `-f null -` means "decode but don't write output" (pure benchmark)
- `-c:v h264_rkmpp` explicitly requests MPP hardware decoder
- Without `-c:v`, FFmpeg would auto-select software decoder

---

### Step 3: Test Hardware Encoder

Test MPP hardware encoding by doing a full decode→encode cycle:

```bash
# Decode with MPP hardware, encode with MPP hardware
ffmpeg -c:v h264_rkmpp -i /tmp/test_video.mp4 \
  -c:v h264_rkmpp -b:v 2M \
  /tmp/output.mp4

# Check encoding speed
# Expected: Real-time (1.0x speed) or faster for 1080p
```

**Encoder parameters explained:**

- `-c:v h264_rkmpp` (input) - Use MPP to decode
- `-c:v h264_rkmpp` (output) - Use MPP to encode  
- `-b:v 2M` - Target bitrate of 2 Mbps
- Output file must have proper extension (`.mp4`, `.mkv`, etc.)

**What real-time means:**
- `speed=1.0x` - Processing at same speed as playback (30fps video → 30fps encode)
- `speed=2.0x` - 2× faster than real-time (can encode 2 videos simultaneously)
- `speed=0.5x` - Slower than real-time (may not be suitable for live streaming)

**Encoder quality options:**

```bash
# Higher quality (larger file, slower)
-b:v 4M -maxrate 4M -bufsize 8M

# Lower quality (smaller file, faster)
-b:v 1M -maxrate 1M -bufsize 2M

# Constant quality mode (variable bitrate)
-qp 25  # (lower = better quality, 20-30 is typical)
```

---

### Step 4: Test RGA Hardware Scaling

Test RGA 2D hardware acceleration by scaling video:

```bash
# Decode with MPP + scale with RGA hardware
ffmpeg -c:v h264_rkmpp -i /tmp/test_video.mp4 \
  -vf "scale_rkrga=1280:720" \
  -c:v h264_rkmpp -b:v 1M \
  /tmp/scaled.mp4

# Expected: Minimal CPU usage, high FPS
```

**RGA filter syntax:**

```bash
# Scale to specific resolution
-vf "scale_rkrga=WIDTH:HEIGHT"

# Scale maintaining aspect ratio
-vf "scale_rkrga=1280:-1"    # Height calculated automatically
-vf "scale_rkrga=-1:720"     # Width calculated automatically

# Multiple filters (RGA → software → RGA not efficient)
-vf "scale_rkrga=1280:720,format=yuv420p"
```

**Performance comparison:**

Run both software and hardware scaling to see the difference:

```bash
# Software scaling (CPU-intensive)
time ffmpeg -i /tmp/test_video.mp4 -vf "scale=1280:720" -f null -

# Hardware scaling (RGA offload)
time ffmpeg -c:v h264_rkmpp -i /tmp/test_video.mp4 \
  -vf "scale_rkrga=1280:720" -f null -
```

You should see **10-50× faster** processing with RGA!

---

### Step 5: Monitor System Resource Usage

While FFmpeg is running, monitor CPU and memory usage to verify hardware acceleration:

```bash
# In another SSH session to the device
ssh root@$DEVICE_IP

# Watch CPU usage in real-time
top -d 1

# Or use htop for a better view (if available)
htop
```

**Expected CPU usage with hardware acceleration:**

| Operation | Software (CPU) | Hardware (MPP/RGA) |
|-----------|---------------|-------------------|
| H.264 Decode 1080p | 300-400% (3-4 cores) | < 20% (mostly overhead) |
| H.265 Encode 1080p | 400%+ (all cores) | < 30% |
| Scaling 1080p→720p | 80-100% | < 10% |

**If CPU usage is high (> 100% for single stream):**
- Hardware acceleration may not be working
- Check FFmpeg output for error messages
- Verify hardware devices exist (see troubleshooting below)

**Memory usage:**
- FFmpeg itself: 20-50 MB
- Per stream buffer: 10-30 MB
- Total for single 1080p stream: < 100 MB

---

## 🐛 Troubleshooting

### Issue 1: Configure Fails - Cannot Find MPP

**Symptom:**
```
ERROR: rockchip_mpp not found using pkg-config
```

**Root cause:** FFmpeg's configure script uses pkg-config to find MPP, but pkg-config isn't looking in the right place (the sysroot).

**Solution:**
```bash
# 1. Verify environment is loaded
echo $PKG_CONFIG_PATH
# Should output: .../sysroot/usr/lib/pkgconfig

# 2. Test pkg-config manually
pkg-config --exists rockchip_mpp && echo "Found" || echo "Not found"

# 3. If not found, check sysroot has the .pc file
ls $STAGING/usr/lib/pkgconfig/rockchip_mpp.pc

# 4. If file exists but pkg-config can't find it, reload environment
source ./ffmpeg-rockchip-cross-compile-env.sh

# 5. If file doesn't exist, MPP wasn't built - rebuild SDK
cd $SDK_ROOT
./build.sh
```

**Understanding pkg-config:**
- `.pc` files contain library metadata (version, include paths, link flags)
- `PKG_CONFIG_PATH` tells pkg-config where to find `.pc` files
- `PKG_CONFIG_SYSROOT_DIR` prepends sysroot path to all paths in `.pc` files

---

### Issue 2: Configure Fails - Cannot Find RGA

**Symptom:**
```
ERROR: librga not found using pkg-config
```

**Solution:**
```bash
# Check RGA
pkg-config --exists librga && echo "Found" || echo "Not found"

# Verify RGA files
ls $STAGING/usr/lib/pkgconfig/librga.pc
ls $STAGING/usr/lib/librga.so

# If missing, RGA wasn't built - see RGA Status guide
```

---

### Issue 3: Compilation Error - Undefined References

**Symptom:**
```
undefined reference to `mpp_create'
undefined reference to `imresize'
/usr/bin/ld: cannot find -lrockchip_mpp
```

**Root cause:** The linker can't find MPP/RGA libraries in the sysroot, even though configure succeeded.

**Solution:**
```bash
# 1. Verify libraries exist in sysroot
ls -l $STAGING/usr/lib/librockchip_mpp.so
ls -l $STAGING/usr/lib/librga.so

# 2. Check if pkg-config returns correct linker flags
pkg-config --libs rockchip_mpp
# Should output: -lrockchip_mpp -lpthread

pkg-config --libs librga
# Should output: -lrga

# 3. If libraries are missing, rebuild SDK
cd $SDK_ROOT
./build.sh

# 4. If libraries exist but linker can't find them, reconfigure with explicit paths
./configure \
  --prefix=$FFMPEG_PREFIX \
  --enable-cross-compile \
  --cross-prefix=${CROSS_PREFIX} \
  --arch=aarch64 \
  --target-os=linux \
  --sysroot=$STAGING \
  --extra-ldflags="-L$STAGING/usr/lib" \
  --extra-cflags="-I$STAGING/usr/include" \
  --enable-gpl \
  --enable-version3 \
  --enable-libdrm \
  --enable-rkmpp \
  --enable-rkrga
```

**Understanding linker flags:**
- `-L<path>` adds a directory to library search path
- `-l<name>` links against lib<name>.so (e.g., `-lrga` → `librga.so`)
- Sysroot should automatically add `-L$STAGING/usr/lib`, but explicit flags ensure it

---

### Issue 4: Runtime Error on Device - Library Not Found

**Symptom:**
```
ffmpeg: error while loading shared libraries: librockchip_mpp.so.0: cannot open shared object file: No such file or directory
```

**Root cause:** The FFmpeg binary was compiled successfully, but the device doesn't have the required MPP/RGA libraries in its rootfs.

**Solution:**

**Option A: Check if libraries are already on device**
```bash
# Connect to device
ssh root@$DEVICE_IP

# Check for MPP
ls -l /usr/lib/librockchip_mpp.so*
# Should show: librockchip_mpp.so.0, librockchip_mpp.so.1, etc.

# Check for RGA
ls -l /usr/lib/librga.so*

# If missing, check alternative locations
find /usr -name "librockchip_mpp.so*" 2>/dev/null
find /usr -name "librga.so*" 2>/dev/null
```

**Option B: Update device rootfs (recommended)**
```bash
# On your development machine, rebuild and flash firmware
cd $SDK_ROOT
./build.sh

# Flash to device (method depends on your setup)
# - SD card image: write to SD card
# - USB/Rockusb: use rkdeveloptool or upgrade_tool
# - Network: use custom OTA update mechanism
```

**Option C: Manually copy libraries (temporary solution)**
```bash
# From development machine, copy libraries to device
scp $STAGING/usr/lib/librockchip_mpp.so.0 root@$DEVICE_IP:/usr/lib/
scp $STAGING/usr/lib/librga.so.2.1.0 root@$DEVICE_IP:/usr/lib/

# On device, create symlinks and update cache
ssh root@$DEVICE_IP
cd /usr/lib
ln -sf librockchip_mpp.so.0 librockchip_mpp.so
ln -sf librga.so.2.1.0 librga.so.2
ln -sf librga.so.2 librga.so
ldconfig

# Verify FFmpeg can now find libraries
ldd /usr/local/bin/ffmpeg | grep -E "mpp|rga"
```

**Why Option C is not ideal:**
- Manual changes may be overwritten during firmware updates
- Doesn't address potential version mismatches
- Error-prone (easy to forget a symlink)

---

### Issue 5: Segmentation Fault on Device

**Symptom:**
```
Segmentation fault (core dumped)
```

**Possible causes and solutions:**

**Cause 1: MPP Version Mismatch**

The FFmpeg binary was compiled against a different MPP version than what's on the device.

```bash
# On device, check installed MPP version
grep -r "MPP_VERSION" /usr/include/rockchip/ 2>/dev/null || \
  echo "MPP headers not on device"

# On development machine, check compiled version
grep MPP_VERSION $STAGING/usr/include/rockchip/rk_mpi.h

# If versions don't match, rebuild SDK firmware to ensure consistency
cd $SDK_ROOT
./build.sh
```

**Cause 2: Missing Hardware Device Files**

MPP requires `/dev/mpp_service` device node to communicate with the kernel driver.

```bash
# On device, check for MPP device
ls -l /dev/mpp_service
# Should show: crw-rw---- 1 root video 241, 0 ...

# If missing, check if kernel module is loaded
lsmod | grep mpp
# Should show: mpp_service, rockchip_mpp, or similar

# If module not loaded, try loading it
modprobe rockchip_mpp
# or
modprobe mpp_service

# If module doesn't exist, kernel wasn't built with MPP support
# Check kernel config: CONFIG_ROCKCHIP_MPP_SERVICE=y
```

**Cause 3: Insufficient Permissions**

User doesn't have permission to access `/dev/mpp_service`.

```bash
# Check device permissions
ls -l /dev/mpp_service
# Should be readable/writable by 'video' group

# Add user to video group (if not root)
usermod -a -G video your_username

# Or temporarily change device permissions (not recommended for production)
chmod 666 /dev/mpp_service

# Or run as root
sudo ffmpeg ...
```

**Cause 4: Memory Issues**

Device ran out of memory or there's memory corruption.

```bash
# Check available memory
free -h

# Check kernel log for OOM (out of memory) or MPP errors
dmesg | tail -50
dmesg | grep -i mpp
dmesg | grep -i "out of memory"

# Try reducing buffer sizes
ffmpeg -c:v h264_rkmpp \
  -rkmpp_buffer_size 20000000 \  # Reduce MPP buffer (default: 30MB)
  -i input.mp4 ...
```

---

### Issue 6: Hardware Acceleration Not Working

**Symptom:**
```
ffmpeg uses software decoder despite -c:v h264_rkmpp
```

**Debugging:**
```bash
# Enable FFmpeg debug output
ffmpeg -loglevel debug -c:v h264_rkmpp -i test.mp4 -f null -

# Look for:
# "Using Rockchip MPP decoder" - Good!
# "Failed to initialize MPP" - Problem!

# Check kernel modules
lsmod | grep -E 'mpp|rga|rockchip'

# Should show:
# mpp_service, rga, ...

# If missing, load modules
modprobe rockchip_mpp
modprobe rga
```

---

## 📊 Performance Validation

### Benchmark Script

```bash
#!/bin/bash
# File: benchmark.sh

TEST_FILE="/tmp/test_1080p.mp4"

echo "=== Software Decoding ==="
time ffmpeg -c:v h264 -i $TEST_FILE -f null -

echo ""
echo "=== Hardware Decoding (MPP) ==="
time ffmpeg -c:v h264_rkmpp -i $TEST_FILE -f null -

echo ""
echo "=== Software Scaling ==="
time ffmpeg -i $TEST_FILE -vf scale=1280:720 -f null -

echo ""
echo "=== Hardware Scaling (RGA) ==="
time ffmpeg -c:v h264_rkmpp -i $TEST_FILE -vf scale_rkrga=1280:720 -f null -
```

**Expected results:**
- Hardware decoding: **5-10x faster** than software
- Hardware scaling: **10-50x faster** than software
- CPU usage: **< 20%** vs **400%** for software

---

## 🔄 Rebuild After Changes

### Quick Rebuild (No Config Changes)

```bash
cd $FFMPEG_SRC
make clean
make -j$(nproc)
make install
```

### Full Rebuild (Config Changes)

```bash
cd $FFMPEG_SRC
make distclean
./configure [options...]
make -j$(nproc)
make install
```

---

## 📝 Configuration Examples

### Example 1: RTSP Streaming Server

```bash
./configure \
  --prefix=$FFMPEG_PREFIX \
  --enable-cross-compile \
  --cross-prefix=${CROSS_PREFIX} \
  --arch=aarch64 \
  --target-os=linux \
  --sysroot=$STAGING \
  --pkg-config=$PKG_CONFIG \
  --enable-gpl \
  --enable-version3 \
  --enable-libdrm \
  --enable-rkmpp \
  --enable-rkrga \
  --enable-protocol=rtsp,rtp,tcp,udp \
  --enable-demuxer=rtsp \
  --enable-muxer=rtsp \
  --disable-static \
  --enable-shared
```

---

### Example 2: Security Camera

```bash
./configure \
  --prefix=$FFMPEG_PREFIX \
  --enable-cross-compile \
  --cross-prefix=${CROSS_PREFIX} \
  --arch=aarch64 \
  --target-os=linux \
  --sysroot=$STAGING \
  --pkg-config=$PKG_CONFIG \
  --enable-gpl \
  --enable-version3 \
  --enable-libdrm \
  --enable-rkmpp \
  --enable-rkrga \
  --enable-protocol=file,http,https,rtsp \
  --enable-demuxer=v4l2 \
  --enable-muxer=mp4,hls,dash \
  --enable-encoder=h264_rkmpp,hevc_rkmpp \
  --enable-filter=scale_rkrga,overlay_rkrga \
  --disable-static \
  --enable-shared
```

---

## ✅ Success Checklist

- [ ] Environment variables set correctly (`source ~/ffmpeg-cross-env.sh`)
- [ ] MPP v1.0.11 confirmed in sysroot
- [ ] RGA v2.1.0 confirmed in sysroot
- [ ] Toolchain accessible (`which aarch64-buildroot-linux-gnu-gcc`)
- [ ] pkg-config finds MPP and RGA
- [ ] `./configure` succeeds with `--enable-rkmpp --enable-rkrga`
- [ ] `make -j$(nproc)` completes without errors
- [ ] FFmpeg binary is ARM aarch64 architecture
- [ ] `ffmpeg -decoders` shows `h264_rkmpp` and `hevc_rkmpp`
- [ ] `ffmpeg -encoders` shows `h264_rkmpp` and `hevc_rkmpp`
- [ ] `ffmpeg -filters` shows `scale_rkrga` and `vpp_rkrga`
- [ ] Binary transferred to device successfully
- [ ] Hardware decoding works on device (FPS > 100)
- [ ] CPU usage is low (< 20%) during hardware operations

---

## 📚 Additional Resources

- [MPP Version and Status](./mpp-version-and-status.md)
- [RGA Version and Status](./rga-version-and-status.md)
- [FFmpeg Hardware Acceleration](./ffmpeg-hardware-acceleration.md)
- [Building Software with SDK](../building-software-with-sdk.md)
- [Cross-Compilation Guide](../cross-compilation-guide.md)

---

**Last Updated:** January 21, 2026  
**SDK Version:** rv1126b_linux6.1_sdk_v1.1.0  
**FFmpeg-Rockchip:** nyanmisaka/ffmpeg-rockchip  
**MPP Version:** v1.0.11  
**RGA Version:** v2.1.0
