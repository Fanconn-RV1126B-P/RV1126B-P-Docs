# Building Software with RV1126B-P SDK

Complete guide for understanding and using the Rockchip RV1126B-P SDK to build custom software.

## 📑 Table of Contents

- [SDK Overview](#sdk-overview)
- [Build Approaches](#build-approaches)
- [Toolchain Setup](#toolchain-setup)
- [Docker Environment](#docker-environment)
- [Best Practices](#best-practices)

---

## SDK Overview

### Directory Structure

```
RV1126B-P-SDK/
├── rv1126b_linux6.1_sdk_v1.1.0/
│   ├── buildroot/               # Buildroot build system
│   │   ├── configs/            # Board configurations
│   │   ├── package/            # Package definitions
│   │   └── output/             # Build output
│   │       └── rockchip_rv1126b/
│   │           ├── host/       # Cross-compilation toolchain
│   │           ├── staging/    # Headers and libraries
│   │           ├── target/     # Target rootfs
│   │           └── images/     # Final images
│   ├── external/               # External packages
│   │   ├── mpp/               # Media Process Platform (MPP)
│   │   ├── linux-rga/         # 2D Graphics Acceleration (RGA)
│   │   └── ...
│   ├── kernel-6.1/            # Linux kernel 6.1
│   ├── u-boot/                # U-Boot bootloader
│   └── device/rockchip/       # Device-specific configs
└── setup-build-env.sh         # Environment setup script
```

### Key Components

1. **Buildroot**: Main build system that creates complete Linux system
2. **External Libraries**: 
   - **MPP** (Media Process Platform): Hardware video codecs (H.264, HEVC, VP9, AV1)
   - **RGA** (Raster Graphics Acceleration): 2D graphics operations
   - **RKAIQ**: Camera ISP and image quality
3. **Kernel**: Linux 6.1 with Rockchip BSP patches
4. **U-Boot**: Bootloader for RV1126B platform

---

## Build Approaches

There are **three main approaches** for building software for RV1126B-P:

### Option 1: Buildroot Integration ⭐ (Production)

**Best for:** Final production builds, rootfs integration

**How it works:**
- Add/modify packages in `buildroot/package/`
- Enable packages in buildroot configuration
- Full SDK rebuild includes your software
- Everything integrated into final rootfs image

**Pros:**
- ✅ Clean, reproducible builds
- ✅ Automatic dependency management
- ✅ Integrated into rootfs image
- ✅ Cross-compilation handled automatically

**Cons:**
- ❌ Slower iteration (full rebuilds)
- ❌ Requires understanding buildroot package system
- ❌ More complex initial setup

**When to use:**
- Final production image
- System-integrated services
- When you need automatic startup/init scripts

---

### Option 2: Cross-Compilation ⭐⭐ (Development)

**Best for:** Development, testing, rapid iteration

**How it works:**
- Use SDK's pre-built cross-compiler toolchain
- Build software standalone outside SDK
- Manually copy binaries to device or rootfs
- Link against libraries in `staging/` directory

**Pros:**
- ✅ Fast iteration during development
- ✅ No need to rebuild entire SDK
- ✅ Easy to test different configurations
- ✅ Can update running device without reflashing

**Cons:**
- ❌ Manual dependency management
- ❌ Must ensure library compatibility
- ❌ Not automatically in rootfs

**When to use:**
- Development and testing
- Quick prototyping
- FFmpeg/custom applications
- When you need fast build cycles

---

### Option 3: Native Compilation (Not Recommended)

**Best for:** Simple scripts, debugging on device

**How it works:**
- Install GCC and build tools on RV1126B device
- Compile directly on target hardware

**Pros:**
- ✅ No cross-compilation complexity

**Cons:**
- ❌ Very slow (ARM Cortex-A7 @ 1.5GHz)
- ❌ Limited RAM (can cause build failures)
- ❌ Need to install dev packages on device
- ❌ Not practical for large projects

**When to use:**
- Simple scripts or very small programs
- Debugging on actual hardware
- Educational purposes

---

## Toolchain Setup

### Understanding the SDK Toolchain

The SDK provides a complete cross-compilation environment:

```bash
SDK_ROOT=/path/to/rv1126b_linux6.1_sdk_v1.1.0
BUILDROOT_OUTPUT=$SDK_ROOT/buildroot/output/rockchip_rv1126b

# Toolchain location
TOOLCHAIN=$BUILDROOT_OUTPUT/host

# Cross-compiler
$TOOLCHAIN/bin/aarch64-buildroot-linux-gnu-gcc

# Sysroot (target libraries and headers)
STAGING=$BUILDROOT_OUTPUT/staging
```

### Toolchain Details

| Component | Path | Description |
|-----------|------|-------------|
| **Cross-compiler** | `host/bin/aarch64-buildroot-linux-gnu-gcc` | GCC for ARM64 |
| **Headers** | `staging/usr/include/` | System headers |
| **Libraries** | `staging/usr/lib/` | System libraries |
| **pkg-config** | `host/bin/pkg-config` | Package configuration |
| **MPP** | `staging/usr/include/rockchip/` | MPP headers |
| **RGA** | `staging/usr/include/rga/` | RGA headers |

### Environment Variables

```bash
# Set up cross-compilation environment
export PATH=$BUILDROOT_OUTPUT/host/bin:$PATH
export PKG_CONFIG_PATH=$BUILDROOT_OUTPUT/staging/usr/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=$BUILDROOT_OUTPUT/staging

# Compiler flags
export CC=aarch64-buildroot-linux-gnu-gcc
export CXX=aarch64-buildroot-linux-gnu-g++
export AR=aarch64-buildroot-linux-gnu-ar
export STRIP=aarch64-buildroot-linux-gnu-strip

# Additional flags for configure scripts
export CFLAGS="--sysroot=$BUILDROOT_OUTPUT/staging"
export LDFLAGS="--sysroot=$BUILDROOT_OUTPUT/staging"
```

---

## Docker Environment

### Why Use Docker?

✅ **Advantages:**
1. **Clean environment** - No conflicts with host system
2. **Reproducible** - Same environment every time
3. **Pre-configured** - All dependencies installed
4. **Easy reset** - Just rebuild container if broken
5. **SDK mounted** - Direct access to your code

### Docker Setup

The Docker container is already configured with:
- Ubuntu 24.04 base
- All build dependencies
- Cross-compilation tools
- SDK mounted at `/workspace`

**Start the container:**

```bash
cd /home/csvke/RV1126B-P/RV1126B-P-SDK-Docker
docker-compose up -d
docker-compose exec rv1126b-builder bash
```

**Inside container:**

```bash
# You're now in /workspace (mounted SDK directory)
cd rv1126b_linux6.1_sdk_v1.1.0

# Access buildroot toolchain
export PATH=/workspace/rv1126b_linux6.1_sdk_v1.1.0/buildroot/output/rockchip_rv1126b/host/bin:$PATH

# Verify toolchain
aarch64-buildroot-linux-gnu-gcc --version
```

### Docker vs Host

| Aspect | Docker Container | Host System |
|--------|-----------------|-------------|
| **Environment** | Clean, isolated | May have conflicts |
| **Dependencies** | Pre-installed | Manual installation |
| **Reproducibility** | High | Variable |
| **Performance** | Native (on Linux) | Native |
| **Reset** | Easy (rebuild) | Difficult |
| **Recommended for** | Development | Production CI/CD |

---

## Best Practices

### 1. Development Workflow

**Recommended approach:**

```
[Option 2] Cross-Compile → Test on Device → [Option 1] Integrate into Buildroot
```

1. **Start with cross-compilation** (Option 2)
   - Fast iteration
   - Test different configurations
   - Debug and optimize
   
2. **Test on actual hardware**
   - Copy binary to device via scp/rsync
   - Test functionality
   - Verify hardware acceleration
   
3. **Integrate into buildroot** (Option 1)
   - Once stable, create buildroot package
   - Automate deployment
   - Include in production image

### 2. Library Dependencies

**Always check library versions:**

```bash
# In staging directory
ls -l $BUILDROOT_OUTPUT/staging/usr/lib/librockchip_mpp.so*
ls -l $BUILDROOT_OUTPUT/staging/usr/lib/librga.so*

# Check library dependencies of your binary
aarch64-buildroot-linux-gnu-readelf -d your-binary
```

**Important:** If you rebuild MPP or RGA in buildroot, you must recompile your application!

### 3. Testing Strategy

1. **Unit tests**: Test individual components
2. **Cross-compilation build**: Verify it compiles
3. **Device testing**: Run on actual RV1126B
4. **Performance testing**: Check hardware acceleration is working
5. **Integration testing**: Test in final system

### 4. Debugging Tips

**Check architecture:**
```bash
file your-binary
# Should show: ELF 64-bit LSB executable, ARM aarch64
```

**Check dependencies:**
```bash
aarch64-buildroot-linux-gnu-ldd your-binary
```

**Verify device files exist:**
```bash
# On device
ls -l /dev/mpp_service /dev/rga /dev/dri
```

---

## Common Issues

### Issue 1: Library Not Found

**Symptom:** `error while loading shared libraries: librockchip_mpp.so.0`

**Solution:**
```bash
# Check library path on device
export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH

# Or install properly in /usr/lib
```

### Issue 2: Wrong Architecture

**Symptom:** `cannot execute binary file: Exec format error`

**Cause:** Compiled for wrong architecture (x86_64 instead of aarch64)

**Solution:**
- Verify cross-compiler is in PATH
- Check CC environment variable
- Ensure configure script detects cross-compilation

### Issue 3: Buildroot Rebuild Takes Forever

**Symptom:** Even small changes trigger long rebuilds

**Solution:**
- Use `make <package>-rebuild` for specific packages
- Use `make <package>-dirclean && make <package>` to rebuild one package
- During development, use cross-compilation (Option 2) instead

### Issue 4: MPP/RGA Not Working

**Symptom:** Hardware acceleration not working, errors accessing devices

**Solution:**
```bash
# Check kernel modules loaded
lsmod | grep -E "mpp|rga"

# Check device permissions
ls -l /dev/mpp_service /dev/rga

# Add user to video group
usermod -a -G video your-user

# Check kernel version (must be Rockchip BSP kernel)
uname -r
```

---

## Example: Cross-Compiling a Simple Program

### 1. Setup Environment

```bash
# In Docker container or host
cd /home/csvke/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0
export BUILDROOT_OUTPUT=$(pwd)/buildroot/output/rockchip_rv1126b
export PATH=$BUILDROOT_OUTPUT/host/bin:$PATH
```

### 2. Create Test Program

```c
// test_mpp.c
#include <stdio.h>
#include <rockchip/rk_mpi.h>

int main() {
    printf("Testing MPP library...\n");
    
    // Initialize MPP context
    MppCtx ctx = NULL;
    MppApi *mpi = NULL;
    
    RK_U32 need_split = 1;
    MPP_RET ret = mpp_create(&ctx, &mpi);
    
    if (ret == MPP_OK) {
        printf("MPP initialized successfully!\n");
        mpp_destroy(ctx);
    } else {
        printf("MPP initialization failed: %d\n", ret);
    }
    
    return 0;
}
```

### 3. Compile

```bash
aarch64-buildroot-linux-gnu-gcc \
    --sysroot=$BUILDROOT_OUTPUT/staging \
    -I$BUILDROOT_OUTPUT/staging/usr/include \
    -L$BUILDROOT_OUTPUT/staging/usr/lib \
    -o test_mpp test_mpp.c \
    -lrockchip_mpp
```

### 4. Deploy to Device

```bash
# Copy to device
scp test_mpp root@192.168.1.100:/root/

# Run on device
ssh root@192.168.1.100 "./test_mpp"
```

---

## Next Steps

- **[FFmpeg Hardware Acceleration](./ffmpeg-hardware-acceleration.md)** - Build FFmpeg with MPP/RGA support
- **[Cross-Compilation Guide](./cross-compilation-guide.md)** - Detailed cross-compilation workflows
- **[Buildroot Integration](./buildroot-integration.md)** - Create custom buildroot packages

---

## References

- [Buildroot User Manual](https://buildroot.org/downloads/manual/manual.html)
- [Rockchip MPP Documentation](http://opensource.rock-chips.com/wiki_Mpp)
- [RV1126 Datasheet](../From%20wiki.fanconn.com/00.规格书/)

---

**Last Updated:** January 21, 2026
