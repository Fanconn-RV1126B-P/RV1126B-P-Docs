# MPP Version and Build Status

Documentation of the Rockchip MPP (Media Process Platform) version and build status in the RV1126B-P SDK.

## 📊 MPP Version Information

### Version Details

| Property | Value |
|----------|-------|
| **Git Version** | `v1.1.0-1-g50fc6082e` |
| **Latest Release** | v1.0.11 |
| **Release Date** | September 10, 2025 |
| **Git Commit** | 50fc6082e |
| **Branch** | RV1126BP-8-feature-setup-build-env |
| **Repository** | https://github.com/Fanconn-RV1126B-P/RV1126B-P-SDK.git |

### Source Location

```
/home/csvke/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/external/mpp
```

### Git Information

```bash
# Check current version
cd $SDK_ROOT/external/mpp
git describe --tags --always
# Output: v1.1.0-1-g50fc6082e

# View recent commits
git log --oneline -5
# 50fc6082e - RV1126BP-8: Add automated build environment setup script
# ea56747bc - (tag: v1.0.11) RV1126BP-7: Add Linux kernel 6.1
# ...
```

---

## 🏗️ Build Status

### ✅ MPP is Built and Ready!

MPP has been successfully built and is available in multiple locations:

### 1. Target Root Filesystem (Device)

**Location:** `buildroot/output/rockchip_rv1126b/target/usr/lib/`

```bash
librockchip_mpp.so -> librockchip_mpp.so.1
librockchip_mpp.so.1 -> librockchip_mpp.so.0
librockchip_mpp.so.0  # 2.3 MB (stripped, for runtime)
```

**Architecture:**
```
ELF 64-bit LSB shared object, ARM aarch64, version 1 (GNU/Linux), dynamically linked, stripped
```

**Purpose:** This is what gets deployed to the actual RV1126B device in the rootfs image.

---

### 2. Staging/Sysroot (Cross-Compilation)

**Location:** `buildroot/output/rockchip_rv1126b/host/aarch64-buildroot-linux-gnu/sysroot/usr/`

#### Libraries

```bash
# Path: sysroot/usr/lib/
librockchip_mpp.so -> librockchip_mpp.so.1
librockchip_mpp.so.1 -> librockchip_mpp.so.0
librockchip_mpp.so.0      # 10.6 MB (with debug symbols)
librockchip_mpp.a         # 32.8 MB (static library)
```

#### Headers

```bash
# Path: sysroot/usr/include/rockchip/
rk_mpi.h              # Main MPP API
rk_mpi_cmd.h          # Command definitions
rk_type.h             # Type definitions
rk_venc_cmd.h         # Encoder commands
rk_vdec_cmd.h         # Decoder commands
mpp_buffer.h          # Buffer management
mpp_frame.h           # Frame structures
mpp_packet.h          # Packet structures
mpp_meta.h            # Metadata
mpp_task.h            # Task management
...
```

**Purpose:** These are used when cross-compiling applications (like FFmpeg) that need to link against MPP.

---

### 3. Build Directory

**Location:** `buildroot/output/rockchip_rv1126b/build/rockchip-mpp-develop/`

Contains:
- Build artifacts
- Object files
- Test programs
- CMake cache

**Purpose:** Intermediate build files, useful for debugging build issues.

---

## 📦 Package Configuration

### pkg-config Files

**Location:** `sysroot/usr/lib/pkgconfig/rockchip_mpp.pc`

```ini
prefix=/usr
exec_prefix=/usr
libdir=/usr/lib
includedir=/usr/include

Name: rockchip_mpp
Description: Rockchip Media Process Platform
Version: 1.0.11
Libs: -L${libdir} -lrockchip_mpp -lpthread
Cflags: -I${includedir}/rockchip
```

**Usage:**
```bash
# Get compiler flags
pkg-config --cflags rockchip_mpp
# Output: -I/usr/include/rockchip

# Get linker flags
pkg-config --libs rockchip_mpp
# Output: -lrockchip_mpp -lpthread
```

---

## 🎯 Key Features in v1.0.11

### New Features

1. **JPEG ROI Support for RV1126B** ✨
   - Region of Interest encoding for JPEG
   - Specific optimization for RV1126B chip

2. **KMPP (Kernel MPP) Enhancements**
   - KmppMeta module
   - KmppBuffer module
   - Improved kernel-space MPP interface

3. **Encoder Improvements**
   - H.265/HEVC encoder optimizations
   - Smart v3 frame QP interface
   - Intra refresh (GDR) support

4. **Metadata Enhancements**
   - User data deep copy support
   - Frame buffer key additions
   - NPU object flags

### Bug Fixes

- H.264/H.265 decoder improvements
- DTS transmission fixes
- Fast play mode corrections
- Bit rate calculation fixes in skip mode
- TSVC (Temporal Scalability) improvements

### Platform Support

- ✅ **RV1109/RV1126** (Your chip!)
- RK29XX/RK30XX/RK31XX
- RK3288/RK3368/RK3399
- RK3228/RK3229/RK3228H/RK3328
- RK3528/RK3528A
- RK3562
- RK3566/RK3568
- RK3588

---

## 🔧 Environment Setup for FFmpeg Compilation

### Correct Paths

When cross-compiling FFmpeg with MPP support, use these paths:

```bash
# SDK root
export SDK_ROOT=/home/csvke/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0

# Build output
export BUILDROOT_OUTPUT=$SDK_ROOT/buildroot/output/rockchip_rv1126b

# ⚠️ IMPORTANT: The sysroot is in a subdirectory!
export STAGING=$BUILDROOT_OUTPUT/host/aarch64-buildroot-linux-gnu/sysroot

# Toolchain
export TOOLCHAIN=$BUILDROOT_OUTPUT/host

# Add to PATH
export PATH=$TOOLCHAIN/bin:$PATH

# pkg-config setup
export PKG_CONFIG_PATH=$STAGING/usr/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=$STAGING
export PKG_CONFIG_LIBDIR=$STAGING/usr/lib/pkgconfig
```

### Common Mistake ❌

```bash
# WRONG - This path doesn't exist
export STAGING=$BUILDROOT_OUTPUT/staging

# CORRECT - Use the sysroot subdirectory
export STAGING=$BUILDROOT_OUTPUT/host/aarch64-buildroot-linux-gnu/sysroot
```

---

## ✅ Verification Commands

### Check MPP Libraries

```bash
# List libraries in sysroot
ls -lh $STAGING/usr/lib/librockchip_mpp*

# Expected output:
# -rw-r--r-- 32.8M librockchip_mpp.a         (static)
# lrwxrwxrwx   20  librockchip_mpp.so -> ...
# -rwxr-xr-x 10.6M librockchip_mpp.so.0      (shared)
```

### Check MPP Headers

```bash
# List headers
ls $STAGING/usr/include/rockchip/

# Expected output:
# rk_mpi.h  rk_type.h  mpp_buffer.h  mpp_frame.h  ...
```

### Test pkg-config

```bash
# Test pkg-config
PKG_CONFIG_PATH=$STAGING/usr/lib/pkgconfig \
PKG_CONFIG_SYSROOT_DIR=$STAGING \
pkg-config --exists rockchip_mpp && echo "MPP found" || echo "MPP not found"

# Get version
pkg-config --modversion rockchip_mpp
```

### Check Library Dependencies

```bash
# Check what the library needs
aarch64-buildroot-linux-gnu-readelf -d $STAGING/usr/lib/librockchip_mpp.so.0 | grep NEEDED

# Expected output:
# libpthread.so.0
# libdl.so.2
# libc.so.6
```

---

## 📚 MPP API Overview

### Core Components

1. **MppCtx** - MPP context handle
2. **MppApi** - MPP API function table
3. **MppBuffer** - Buffer management
4. **MppFrame** - Video frame structure
5. **MppPacket** - Encoded packet structure

### Basic Usage Pattern

```c
#include <rockchip/rk_mpi.h>

// Initialize
MppCtx ctx = NULL;
MppApi *mpi = NULL;

MPP_RET ret = mpp_create(&ctx, &mpi);
if (ret != MPP_OK) {
    // Handle error
}

// Configure codec type
ret = mpi->control(ctx, MPP_SET_INPUT_BLOCK, &block_mode);
ret = mpi->control(ctx, MPP_SET_OUTPUT_BLOCK, &block_mode);

// Initialize codec
MppParam param = NULL;
ret = mpp_init(ctx, MPP_CTX_DEC, MPP_VIDEO_CodingAVC); // H.264 decoder

// Decode frames...

// Cleanup
mpp_destroy(ctx);
```

---

## 🔗 Upstream References

### Official Repositories

| Repository | URL | Branch | Purpose |
|------------|-----|--------|---------|
| **Official** | https://github.com/rockchip-linux/mpp | master | Stable releases |
| **Development** | https://github.com/HermanChen/mpp | develop | Latest features |
| **Jellyfin Fork** | https://gitee.com/nyanmisaka/mpp | jellyfin-mpp | For FFmpeg integration |

### Documentation

- **Official Wiki**: http://opensource.rock-chips.com/wiki_Mpp
- **API Reference**: See `external/mpp/doc/` directory
- **Examples**: See `external/mpp/test/` directory

---

## 🚀 Next Steps for FFmpeg-Rockchip

Now that we've confirmed MPP v1.0.11 is built and ready, you can proceed with FFmpeg-Rockchip compilation:

1. ✅ **MPP is ready** - No rebuild needed
2. ✅ **Headers are available** - In sysroot/usr/include/rockchip/
3. ✅ **Libraries are built** - Both shared and static available
4. ⏭️ **Next:** Build FFmpeg-Rockchip with `--enable-rkmpp`

See: [FFmpeg Hardware Acceleration Guide](../ffmpeg-hardware-acceleration.md)

---

## 📝 Build Configuration

### Buildroot Config

MPP was enabled in buildroot with:

```kconfig
BR2_PACKAGE_ROCKCHIP_MPP=y
BR2_PACKAGE_ROCKCHIP_MPP_ALLOCATOR_DRM=y
BR2_PACKAGE_ROCKCHIP_MPP_TESTS=y
```

**Config file:** `buildroot/configs/rockchip/multimedia/mpp.config`

### CMake Build Options

MPP was built with:

```cmake
-DRKPLATFORM=ON
-DHAVE_DRM=ON
-DBUILD_TEST=ON
-DCMAKE_BUILD_TYPE=Release
```

---

## 🐛 Troubleshooting

### Issue: Cannot Find MPP Library

**Symptom:**
```
/usr/bin/ld: cannot find -lrockchip_mpp
```

**Solution:**
```bash
# Check if STAGING is set correctly
echo $STAGING
# Should be: .../host/aarch64-buildroot-linux-gnu/sysroot

# Verify library exists
ls -l $STAGING/usr/lib/librockchip_mpp.so
```

### Issue: Cannot Find MPP Headers

**Symptom:**
```
fatal error: rockchip/rk_mpi.h: No such file or directory
```

**Solution:**
```bash
# Check include path
ls $STAGING/usr/include/rockchip/

# Add to compiler flags
-I$STAGING/usr/include
```

### Issue: Wrong Architecture

**Symptom:**
```
error: incompatible target
```

**Solution:**
```bash
# Verify library architecture
file $STAGING/usr/lib/librockchip_mpp.so.0
# Should show: ARM aarch64

# Ensure cross-compiler is in PATH
which aarch64-buildroot-linux-gnu-gcc
```

---

## 📊 Summary

| Aspect | Status | Details |
|--------|--------|---------|
| **Version** | ✅ v1.0.11 | Latest stable release |
| **Build Status** | ✅ Built | All libraries and headers present |
| **Target Device** | ✅ Ready | 2.3 MB shared library in rootfs |
| **Cross-Compile** | ✅ Ready | 10.6 MB shared + 32.8 MB static |
| **Headers** | ✅ Available | Complete API headers in sysroot |
| **pkg-config** | ✅ Configured | rockchip_mpp.pc available |
| **FFmpeg Ready** | ✅ Yes | Can build FFmpeg-Rockchip now |

---

**Last Updated:** January 21, 2026  
**SDK Version:** rv1126b_linux6.1_sdk_v1.1.0  
**MPP Version:** v1.0.11
