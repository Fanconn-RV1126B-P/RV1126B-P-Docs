# Cross-Compilation Guide for RV1126B-P

Detailed guide for cross-compiling software for RV1126B-P using the SDK toolchain.

## 📑 Table of Contents

- [Introduction](#introduction)
- [Toolchain Overview](#toolchain-overview)
- [Environment Setup](#environment-setup)
- [Cross-Compiling Applications](#cross-compiling-applications)
- [Common Build Systems](#common-build-systems)
- [Linking Libraries](#linking-libraries)
- [Debugging](#debugging)

---

## Introduction

Cross-compilation allows you to build ARM64 binaries on your x86_64 development machine, which is much faster than compiling natively on the RV1126B device.

### Why Cross-Compile?

- ⚡ **10-50x faster** than native compilation
- 💪 **More resources** available (RAM, CPU, storage)
- 🔄 **Quick iteration** during development
- 🧪 **Easy testing** of different configurations

---

## Toolchain Overview

### What's Included

The SDK provides a complete cross-compilation toolchain built by buildroot:

```
$SDK_ROOT/buildroot/output/rockchip_rv1126b/host/
├── bin/                                    # Toolchain binaries
│   ├── aarch64-buildroot-linux-gnu-gcc    # C compiler
│   ├── aarch64-buildroot-linux-gnu-g++    # C++ compiler
│   ├── aarch64-buildroot-linux-gnu-ar     # Archiver
│   ├── aarch64-buildroot-linux-gnu-ld     # Linker
│   ├── aarch64-buildroot-linux-gnu-strip  # Strip utility
│   ├── pkg-config                         # Package config
│   └── ...
├── lib/                                    # Host libraries
├── share/                                  # Data files
└── usr/                                    # Additional tools

$SDK_ROOT/buildroot/output/rockchip_rv1126b/staging/
├── usr/
│   ├── include/                           # Target headers
│   │   ├── rockchip/                      # MPP headers
│   │   ├── rga/                           # RGA headers
│   │   └── ...
│   └── lib/                               # Target libraries
│       ├── librockchip_mpp.so             # MPP library
│       ├── librga.so                      # RGA library
│       └── ...
```

### Toolchain Specifications

| Component | Value |
|-----------|-------|
| **Target Architecture** | aarch64 (ARM64) |
| **CPU** | Cortex-A7 |
| **ABI** | LP64 |
| **C Library** | glibc or musl |
| **GCC Version** | 11.x or 12.x |
| **Kernel Headers** | 6.1.x |

---

## Environment Setup

### Method 1: Docker Container (Recommended)

```bash
# Start Docker container
cd /home/csvke/RV1126B-P/RV1126B-P-SDK-Docker
docker-compose up -d
docker-compose exec rv1126b-builder bash

# Inside container, set up environment
export SDK_ROOT=/workspace/rv1126b_linux6.1_sdk_v1.1.0
export BUILDROOT_OUTPUT=$SDK_ROOT/buildroot/output/rockchip_rv1126b
export STAGING=$BUILDROOT_OUTPUT/staging
export TOOLCHAIN=$BUILDROOT_OUTPUT/host

# Add toolchain to PATH
export PATH=$TOOLCHAIN/bin:$PATH

# pkg-config configuration
export PKG_CONFIG_PATH=$STAGING/usr/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=$STAGING
export PKG_CONFIG_LIBDIR=$STAGING/usr/lib/pkgconfig
```

### Method 2: Host System

If not using Docker, add this to `~/.bashrc` or create a setup script:

```bash
#!/bin/bash
# save as: ~/rv1126b-env.sh

export SDK_ROOT=/home/csvke/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0
export BUILDROOT_OUTPUT=$SDK_ROOT/buildroot/output/rockchip_rv1126b
export STAGING=$BUILDROOT_OUTPUT/staging
export TOOLCHAIN=$BUILDROOT_OUTPUT/host

export PATH=$TOOLCHAIN/bin:$PATH
export PKG_CONFIG_PATH=$STAGING/usr/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=$STAGING
export PKG_CONFIG_LIBDIR=$STAGING/usr/lib/pkgconfig

# Cross-compilation variables
export CROSS_COMPILE=aarch64-buildroot-linux-gnu-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export AR=${CROSS_COMPILE}ar
export LD=${CROSS_COMPILE}ld
export STRIP=${CROSS_COMPILE}strip
export RANLIB=${CROSS_COMPILE}ranlib

# Compiler flags
export CFLAGS="--sysroot=$STAGING"
export CXXFLAGS="--sysroot=$STAGING"
export LDFLAGS="--sysroot=$STAGING"

echo "RV1126B cross-compilation environment ready!"
echo "Toolchain: $(${CC} --version | head -n1)"
```

**Usage:**
```bash
source ~/rv1126b-env.sh
```

### Verify Setup

```bash
# Check compiler
${CC} --version
# Should output: aarch64-buildroot-linux-gnu-gcc ...

# Check architecture
${CC} -dumpmachine
# Should output: aarch64-buildroot-linux-gnu

# Test compilation
echo 'int main() { return 0; }' | ${CC} -x c - -o test
file test
# Should output: ELF 64-bit LSB executable, ARM aarch64
```

---

## Cross-Compiling Applications

### Example 1: Simple C Program

**hello.c:**
```c
#include <stdio.h>

int main() {
    printf("Hello from RV1126B!\n");
    return 0;
}
```

**Compile:**
```bash
aarch64-buildroot-linux-gnu-gcc \
    --sysroot=$STAGING \
    -o hello hello.c

# Strip to reduce size
aarch64-buildroot-linux-gnu-strip hello

# Verify
file hello
# Output: ELF 64-bit LSB executable, ARM aarch64

# Deploy
scp hello root@<device-ip>:/root/
```

### Example 2: With External Library (MPP)

**test_mpp.c:**
```c
#include <stdio.h>
#include <rockchip/rk_mpi.h>

int main() {
    MppCtx ctx = NULL;
    MppApi *mpi = NULL;
    
    MPP_RET ret = mpp_create(&ctx, &mpi);
    if (ret == MPP_OK) {
        printf("MPP initialized successfully!\n");
        mpp_destroy(ctx);
        return 0;
    } else {
        printf("MPP initialization failed: %d\n", ret);
        return 1;
    }
}
```

**Compile:**
```bash
aarch64-buildroot-linux-gnu-gcc \
    --sysroot=$STAGING \
    -I$STAGING/usr/include \
    -L$STAGING/usr/lib \
    -o test_mpp test_mpp.c \
    -lrockchip_mpp \
    -lpthread

# Check dependencies
aarch64-buildroot-linux-gnu-readelf -d test_mpp | grep NEEDED
```

### Example 3: C++ Application

**hello.cpp:**
```cpp
#include <iostream>
#include <string>

int main() {
    std::string msg = "Hello from C++ on RV1126B!";
    std::cout << msg << std::endl;
    return 0;
}
```

**Compile:**
```bash
aarch64-buildroot-linux-gnu-g++ \
    --sysroot=$STAGING \
    -o hello_cpp hello.cpp \
    -lstdc++

aarch64-buildroot-linux-gnu-strip hello_cpp
```

---

## Common Build Systems

### Autotools (./configure)

Most common for GNU software.

**Example: Generic autotools project**

```bash
# Configure for cross-compilation
./configure \
    --host=aarch64-buildroot-linux-gnu \
    --prefix=/usr \
    --sysroot=$STAGING \
    PKG_CONFIG_PATH=$PKG_CONFIG_PATH \
    PKG_CONFIG_SYSROOT_DIR=$PKG_CONFIG_SYSROOT_DIR \
    CC=aarch64-buildroot-linux-gnu-gcc \
    CXX=aarch64-buildroot-linux-gnu-g++ \
    CFLAGS="--sysroot=$STAGING" \
    LDFLAGS="--sysroot=$STAGING"

# Build
make -j$(nproc)

# Install to staging (optional)
make DESTDIR=$STAGING install
```

**Common autotools options:**

| Option | Description |
|--------|-------------|
| `--host` | Target system (cross-compilation) |
| `--build` | Build system (auto-detected) |
| `--prefix` | Installation prefix |
| `--enable-static` | Build static libraries |
| `--disable-shared` | Don't build shared libraries |

### CMake

Modern build system, very common.

**Example: CMake project**

```bash
# Create build directory
mkdir build && cd build

# Configure with toolchain file
cmake .. \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_C_COMPILER=aarch64-buildroot-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-buildroot-linux-gnu-g++ \
    -DCMAKE_SYSROOT=$STAGING \
    -DCMAKE_FIND_ROOT_PATH=$STAGING \
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Install
make DESTDIR=$STAGING install
```

**Better: Use toolchain file**

Create `toolchain-rv1126b.cmake`:

```cmake
# CMake toolchain file for RV1126B cross-compilation

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Toolchain paths
set(CROSS_COMPILE aarch64-buildroot-linux-gnu)
set(CMAKE_C_COMPILER ${CROSS_COMPILE}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}-g++)
set(CMAKE_AR ${CROSS_COMPILE}-ar)
set(CMAKE_RANLIB ${CROSS_COMPILE}-ranlib)
set(CMAKE_STRIP ${CROSS_COMPILE}-strip)

# Sysroot
set(CMAKE_SYSROOT $ENV{STAGING})
set(CMAKE_FIND_ROOT_PATH $ENV{STAGING})

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# pkg-config
set(ENV{PKG_CONFIG_PATH} "$ENV{STAGING}/usr/lib/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "$ENV{STAGING}")
```

**Usage:**
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-rv1126b.cmake ..
make -j$(nproc)
```

### Meson

Modern, Python-based build system.

**Create cross-file:** `rv1126b-cross.txt`

```ini
[binaries]
c = 'aarch64-buildroot-linux-gnu-gcc'
cpp = 'aarch64-buildroot-linux-gnu-g++'
ar = 'aarch64-buildroot-linux-gnu-ar'
strip = 'aarch64-buildroot-linux-gnu-strip'
pkgconfig = 'pkg-config'

[properties]
sys_root = '/workspace/rv1126b_linux6.1_sdk_v1.1.0/buildroot/output/rockchip_rv1126b/staging'
pkg_config_libdir = '/workspace/rv1126b_linux6.1_sdk_v1.1.0/buildroot/output/rockchip_rv1126b/staging/usr/lib/pkgconfig'

[host_machine]
system = 'linux'
cpu_family = 'aarch64'
cpu = 'cortex-a7'
endian = 'little'
```

**Usage:**
```bash
meson setup build --cross-file rv1126b-cross.txt
ninja -C build
```

### Make (Simple Makefile)

**Example Makefile:**

```makefile
# Makefile for RV1126B cross-compilation

# Toolchain
CROSS_COMPILE ?= aarch64-buildroot-linux-gnu-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip

# Paths
STAGING ?= /workspace/rv1126b_linux6.1_sdk_v1.1.0/buildroot/output/rockchip_rv1126b/staging

# Flags
CFLAGS = --sysroot=$(STAGING) -I$(STAGING)/usr/include
LDFLAGS = --sysroot=$(STAGING) -L$(STAGING)/usr/lib

# Targets
TARGET = myapp
SRCS = main.c utils.c
OBJS = $(SRCS:.c=.o)
LIBS = -lrockchip_mpp -lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
```

---

## Linking Libraries

### Static vs Dynamic Linking

**Dynamic (default):**
```bash
${CC} -o myapp main.c -lrockchip_mpp
```
- Smaller binary
- Requires .so files on device
- Updates don't require recompilation

**Static:**
```bash
${CC} -o myapp main.c -static -lrockchip_mpp
```
- Larger binary
- Self-contained (no .so dependencies)
- No runtime library conflicts

### Check Dependencies

```bash
# Show required libraries
aarch64-buildroot-linux-gnu-readelf -d myapp | grep NEEDED

# Alternative
aarch64-buildroot-linux-gnu-ldd myapp
```

### Common Libraries in SDK

| Library | Header | Link Flag | Description |
|---------|--------|-----------|-------------|
| **MPP** | `rockchip/rk_mpi.h` | `-lrockchip_mpp` | Media codecs |
| **RGA** | `rga/rga.h` | `-lrga` | 2D graphics |
| **DRM** | `drm/drm.h` | `-ldrm` | Display |
| **pthread** | `pthread.h` | `-lpthread` | Threading |
| **math** | `math.h` | `-lm` | Math functions |

### pkg-config

Use pkg-config to get correct flags:

```bash
# Get compiler flags
pkg-config --cflags rockchip_mpp

# Get linker flags
pkg-config --libs rockchip_mpp

# Use in Makefile
CFLAGS += $(shell pkg-config --cflags rockchip_mpp)
LDFLAGS += $(shell pkg-config --libs rockchip_mpp)
```

---

## Debugging

### Check Binary Architecture

```bash
file myapp
# Should show: ELF 64-bit LSB executable, ARM aarch64

# Detailed info
aarch64-buildroot-linux-gnu-readelf -h myapp | grep Machine
# Should show: Machine: AArch64
```

### Check Dependencies

```bash
# List required libraries
aarch64-buildroot-linux-gnu-readelf -d myapp | grep NEEDED

# Check if library exists in staging
ls -l $STAGING/usr/lib/librockchip_mpp.so
```

### Symbol Analysis

```bash
# List all symbols
aarch64-buildroot-linux-gnu-nm myapp

# Find specific symbol
aarch64-buildroot-linux-gnu-nm myapp | grep mpp_create

# Check undefined symbols
aarch64-buildroot-linux-gnu-nm -u myapp
```

### Size Optimization

```bash
# Strip binary (remove debug symbols)
aarch64-buildroot-linux-gnu-strip myapp

# Compare sizes
ls -lh myapp
# Before strip: 500KB
# After strip: 50KB

# Aggressive stripping
aarch64-buildroot-linux-gnu-strip --strip-all myapp
```

### Remote Debugging with GDB

**On host:**
```bash
# Start gdbserver on device
ssh root@device "gdbserver :2345 /root/myapp"
```

**On development machine:**
```bash
aarch64-buildroot-linux-gnu-gdb myapp
(gdb) target remote device-ip:2345
(gdb) continue
```

---

## Common Issues

### Issue 1: Wrong Architecture

**Error:** `cannot execute binary file: Exec format error`

**Cause:** Binary compiled for wrong architecture

**Fix:**
```bash
# Verify CC is set correctly
echo $CC
# Should be: aarch64-buildroot-linux-gnu-gcc

# Check binary
file myapp
```

### Issue 2: Library Not Found (Build Time)

**Error:** `/usr/bin/ld: cannot find -lrockchip_mpp`

**Cause:** Library not in staging directory

**Fix:**
```bash
# Check if library exists
ls -l $STAGING/usr/lib/librockchip_mpp.so

# Rebuild MPP in buildroot if missing
cd $SDK_ROOT/buildroot
make rockchip-mpp-rebuild
```

### Issue 3: Library Not Found (Runtime)

**Error on device:** `error while loading shared libraries: librockchip_mpp.so.0`

**Fix:**
```bash
# On device, check library location
ls -l /usr/lib/librockchip_mpp.so*

# Set LD_LIBRARY_PATH if in different location
export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH

# Or copy library to device
scp $STAGING/usr/lib/librockchip_mpp.so* root@device:/usr/lib/
```

### Issue 4: Header Not Found

**Error:** `fatal error: rockchip/rk_mpi.h: No such file or directory`

**Fix:**
```bash
# Add include path
-I$STAGING/usr/include

# Or check if header exists
find $STAGING -name rk_mpi.h
```

---

## Best Practices

1. **Use Docker** for consistent environment
2. **Always use `--sysroot`** to avoid host library pollution
3. **Strip binaries** before deployment
4. **Check dependencies** with readelf
5. **Use static linking** when possible for simpler deployment
6. **Test on real hardware** early and often
7. **Keep SDK in sync** with device firmware

---

## Next Steps

- **[Buildroot Integration](./buildroot-integration.md)** - Package your software properly
- **[FFmpeg Hardware Acceleration](./ffmpeg-hardware-acceleration.md)** - Build FFmpeg with MPP

---

**Last Updated:** January 21, 2026
