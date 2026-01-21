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

### Step 1: Set SDK Paths

Create a setup script for convenience:

```bash
#!/bin/bash
# File: ~/ffmpeg-cross-env.sh

# SDK root
export SDK_ROOT=/home/csvke/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0

# Buildroot output
export BUILDROOT_OUTPUT=$SDK_ROOT/buildroot/output/rockchip_rv1126b

# Staging directory (sysroot) for cross-compilation
export STAGING=$BUILDROOT_OUTPUT/host/aarch64-buildroot-linux-gnu/sysroot

# Toolchain directory
export TOOLCHAIN=$BUILDROOT_OUTPUT/host

# Add toolchain to PATH
export PATH=$TOOLCHAIN/bin:$PATH

# Cross-compiler prefix
export CROSS_PREFIX=aarch64-buildroot-linux-gnu-

# pkg-config setup for finding libraries
export PKG_CONFIG_PATH=$STAGING/usr/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=$STAGING
export PKG_CONFIG_LIBDIR=$STAGING/usr/lib/pkgconfig

# FFmpeg source
export FFMPEG_SRC=/home/csvke/RV1126B-P/ffmpeg-rockchip

# Installation prefix (where to install FFmpeg)
export FFMPEG_PREFIX=$HOME/ffmpeg-rv1126b-install

echo "✅ Environment configured for RV1126B-P cross-compilation"
echo "   Toolchain: $TOOLCHAIN/bin"
echo "   Sysroot:   $STAGING"
echo "   FFmpeg:    $FFMPEG_SRC"
echo "   Install:   $FFMPEG_PREFIX"
```

**Load the environment:**

```bash
source ~/ffmpeg-cross-env.sh
```

---

### Step 2: Verify Environment

```bash
# Check toolchain
which ${CROSS_PREFIX}gcc
# Expected: /home/csvke/RV1126B-P/RV1126B-P-SDK/.../host/bin/aarch64-buildroot-linux-gnu-gcc

# Check GCC version
${CROSS_PREFIX}gcc --version
# Expected: aarch64-buildroot-linux-gnu-gcc (Buildroot ...) 11.x or 12.x

# Verify MPP
pkg-config --exists rockchip_mpp && echo "✅ MPP found" || echo "❌ MPP not found"
pkg-config --modversion rockchip_mpp
# Expected: 1.0.11

# Verify RGA
pkg-config --exists librga && echo "✅ RGA found" || echo "❌ RGA not found"
pkg-config --modversion librga
# Expected: 2.1.0

# Check sysroot
ls $STAGING/usr/lib/librockchip_mpp.so
ls $STAGING/usr/lib/librga.so
ls $STAGING/usr/include/rockchip/rk_mpi.h
ls $STAGING/usr/include/rga/im2d.h
```

**All checks should pass!** If any fail, review [MPP Status](./mpp-version-and-status.md) or [RGA Status](./rga-version-and-status.md).

---

## 🔧 FFmpeg Configuration

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
  --pkg-config=pkg-config \
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
  --pkg-config=pkg-config \
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
  --pkg-config=pkg-config \
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

Choose one of the configurations above:

```bash
# Source environment
source ~/ffmpeg-cross-env.sh

# Go to FFmpeg source
cd $FFMPEG_SRC

# Run configure (choose one from above)
./configure \
  --prefix=$FFMPEG_PREFIX \
  --enable-cross-compile \
  --cross-prefix=${CROSS_PREFIX} \
  --arch=aarch64 \
  --target-os=linux \
  --sysroot=$STAGING \
  --enable-gpl \
  --enable-version3 \
  --enable-libdrm \
  --enable-rkmpp \
  --enable-rkrga \
  --disable-static \
  --enable-shared
```

**Check configuration output:**

Look for these lines:
```
Enabled decoders:
h264_rkmpp hevc_rkmpp vp8_rkmpp vp9_rkmpp av1_rkmpp

Enabled encoders:
h264_rkmpp hevc_rkmpp

Enabled filters:
scale_rkrga vpp_rkrga overlay_rkrga transpose_rkrga
```

If you see `rkmpp` and `rkrga` components, configuration succeeded! ✅

---

### Step 2: Compile FFmpeg

```bash
# Build with all available CPU cores
make -j$(nproc)
```

**Compilation time:**
- Fast machine (16+ cores): ~5-10 minutes
- Medium machine (8 cores): ~10-15 minutes
- Slow machine (4 cores): ~20-30 minutes

**If compilation fails**, see [Troubleshooting](#-troubleshooting) section below.

---

### Step 3: Install FFmpeg

```bash
# Install to prefix directory
make install
```

This creates:
```
$FFMPEG_PREFIX/
├── bin/
│   ├── ffmpeg          # Main FFmpeg binary
│   ├── ffprobe         # Media file analyzer
│   └── ffplay          # Simple media player (if built)
├── lib/
│   ├── libavcodec.so.XX
│   ├── libavformat.so.XX
│   ├── libavfilter.so.XX
│   ├── libavutil.so.XX
│   └── ...
└── include/
    └── libav*/
```

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

### Option 1: Create Tarball

```bash
# Create distributable package
cd $FFMPEG_PREFIX
tar czf ~/ffmpeg-rv1126b-$(date +%Y%m%d).tar.gz bin/ lib/

# Size: ~10-20 MB (depending on configuration)
```

**Transfer to device:**
```bash
scp ~/ffmpeg-rv1126b-*.tar.gz root@192.168.1.100:/tmp/
```

---

### Option 2: Install to Device Root

```bash
# On device, extract to /usr/local
ssh root@192.168.1.100
cd /usr/local
tar xzf /tmp/ffmpeg-rv1126b-*.tar.gz

# Add to PATH
export PATH=/usr/local/bin:$PATH
echo 'export PATH=/usr/local/bin:$PATH' >> /etc/profile

# Update library cache
ldconfig
```

---

### Option 3: Direct Install (NFS/Shared Filesystem)

If your device mounts a shared filesystem:

```bash
# Configure with device prefix directly
./configure \
  --prefix=/mnt/shared/ffmpeg \
  ... # other options

make -j$(nproc)
make install

# On device:
export PATH=/mnt/shared/ffmpeg/bin:$PATH
export LD_LIBRARY_PATH=/mnt/shared/ffmpeg/lib:$LD_LIBRARY_PATH
```

---

## 🧪 Testing on Device

### Step 1: Transfer Test File

```bash
# Copy a test video
scp test.mp4 root@192.168.1.100:/tmp/
```

---

### Step 2: Test Hardware Decoder

```bash
# SSH into device
ssh root@192.168.1.100

# Test H.264 hardware decoding
ffmpeg -c:v h264_rkmpp -i /tmp/test.mp4 -f null -

# Expected output:
# frame=  300 fps=240 q=-0.0 Lsize=N/A time=00:00:10.00
```

**If FPS > 100**, hardware decoding is working! 🚀

---

### Step 3: Test Hardware Encoder

```bash
# Decode with MPP, encode with MPP
ffmpeg -c:v h264_rkmpp -i /tmp/test.mp4 \
  -c:v h264_rkmpp -b:v 2M \
  /tmp/output.mp4

# Check encoding speed
# Expected: Real-time or faster for 1080p
```

---

### Step 4: Test RGA Scaling

```bash
# Decode + scale with hardware
ffmpeg -c:v h264_rkmpp -i /tmp/test.mp4 \
  -vf "scale_rkrga=1280:720" \
  -c:v h264_rkmpp -b:v 1M \
  /tmp/scaled.mp4

# Expected: Minimal CPU usage, high FPS
```

---

### Step 5: Monitor CPU Usage

```bash
# In another terminal, watch CPU usage
ssh root@192.168.1.100
top -d 1

# While FFmpeg runs, CPU usage should be low (< 20% for single stream)
```

---

## 🐛 Troubleshooting

### Issue 1: Configure Fails - Cannot Find MPP

**Symptom:**
```
ERROR: rockchip_mpp not found using pkg-config
```

**Solution:**
```bash
# Verify pkg-config setup
echo $PKG_CONFIG_PATH
# Should be: .../sysroot/usr/lib/pkgconfig

# Test manually
pkg-config --exists rockchip_mpp && echo "Found" || echo "Not found"

# If not found, check sysroot
ls $STAGING/usr/lib/pkgconfig/rockchip_mpp.pc

# If missing, MPP wasn't built - see MPP Status guide
```

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
```

**Solution:**
```bash
# Check that libraries are in sysroot
ls -l $STAGING/usr/lib/librockchip_mpp.so
ls -l $STAGING/usr/lib/librga.so

# Verify pkg-config returns correct flags
pkg-config --libs rockchip_mpp
pkg-config --libs librga

# Reconfigure with explicit library paths
./configure \
  ... \
  --extra-ldflags="-L$STAGING/usr/lib" \
  --extra-cflags="-I$STAGING/usr/include"
```

---

### Issue 4: Runtime Error on Device

**Symptom:**
```
error while loading shared libraries: librockchip_mpp.so.0: cannot open shared object file
```

**Solution:**
```bash
# Check if MPP library is on device
ssh root@192.168.1.100 "ls -l /usr/lib/librockchip_mpp.so*"

# If missing, MPP wasn't included in firmware
# You need to rebuild SDK firmware or manually copy:
scp $STAGING/usr/lib/librockchip_mpp.so.0 root@192.168.1.100:/usr/lib/
ssh root@192.168.1.100 "ldconfig"
```

---

### Issue 5: Segmentation Fault on Device

**Symptom:**
```
Segmentation fault (core dumped)
```

**Possible causes:**
1. **MPP version mismatch** - FFmpeg compiled against different MPP version
2. **Missing device files** - `/dev/mpp_service` not available
3. **Insufficient permissions** - Need root or video group

**Solution:**
```bash
# Check MPP device
ls -l /dev/mpp_service
# Should exist and be readable

# Check MPP version on device
grep MPP_VERSION /usr/include/rockchip/rk_mpi.h

# Compare with sysroot version
grep MPP_VERSION $STAGING/usr/include/rockchip/rk_mpi.h

# If versions don't match, rebuild SDK firmware
cd $SDK_ROOT
./build.sh
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
