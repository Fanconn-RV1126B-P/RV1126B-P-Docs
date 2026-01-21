# Buildroot Integration Guide

Guide for integrating custom software packages into RV1126B-P SDK's buildroot system.

## 📑 Table of Contents

- [Introduction](#introduction)
- [Buildroot Package System](#buildroot-package-system)
- [Creating a Package](#creating-a-package)
- [Package Types](#package-types)
- [FFmpeg-Rockchip Integration Example](#ffmpeg-rockchip-integration-example)
- [Testing and Debugging](#testing-and-debugging)

---

## Introduction

Buildroot integration allows your software to be:
- ✅ Automatically cross-compiled
- ✅ Included in the rootfs image
- ✅ Managed with dependencies
- ✅ Configured through menuconfig
- ✅ Reproducibly built

---

## Buildroot Package System

### Directory Structure

```
buildroot/
├── package/
│   ├── Config.in              # Top-level menu
│   ├── Config.in.legacy       # Deprecated options
│   └── <category>/            # Package categories
│       └── <package-name>/
│           ├── Config.in      # Package configuration menu
│           ├── <package>.mk   # Build recipe
│           └── <package>.hash # Checksums (optional)
```

### Package Infrastructure

Buildroot supports several package types:

| Type | Use Case | Example |
|------|----------|---------|
| `generic-package` | Custom build system | Simple Makefile projects |
| `cmake-package` | CMake projects | Most modern C/C++ projects |
| `autotools-package` | Autotools (./configure) | GNU software |
| `meson-package` | Meson projects | Modern projects |
| `python-package` | Python packages | PyPI packages |

---

## Creating a Package

### Step 1: Create Package Directory

```bash
cd $SDK_ROOT/buildroot/package
mkdir -p mypackage
cd mypackage
```

### Step 2: Create Config.in

**mypackage/Config.in:**

```kconfig
config BR2_PACKAGE_MYPACKAGE
    bool "mypackage"
    depends on BR2_PACKAGE_ROCKCHIP_MPP  # Optional: dependencies
    select BR2_PACKAGE_LIBDRM             # Optional: auto-select
    help
      Description of your package.
      
      Multiple lines are supported.
      
      https://github.com/yourorg/mypackage
```

**Configuration Options:**

```kconfig
config BR2_PACKAGE_MYPACKAGE
    bool "mypackage"

if BR2_PACKAGE_MYPACKAGE

config BR2_PACKAGE_MYPACKAGE_FEATURE_X
    bool "Enable feature X"
    default y
    help
      Enable advanced feature X.

config BR2_PACKAGE_MYPACKAGE_DEBUG
    bool "Build with debug symbols"
    default n
    help
      Build with debug information.

endif  # BR2_PACKAGE_MYPACKAGE
```

### Step 3: Create Build Recipe (.mk file)

**mypackage/mypackage.mk:**

```makefile
################################################################################
#
# mypackage
#
################################################################################

# Version and source
MYPACKAGE_VERSION = 1.0.0
MYPACKAGE_SITE = https://github.com/yourorg/mypackage/archive
MYPACKAGE_SOURCE = v$(MYPACKAGE_VERSION).tar.gz
MYPACKAGE_SITE_METHOD = wget

# Or for git:
# MYPACKAGE_SITE = https://github.com/yourorg/mypackage.git
# MYPACKAGE_SITE_METHOD = git
# MYPACKAGE_GIT_SUBMODULES = YES  # If project has submodules

# License
MYPACKAGE_LICENSE = GPL-2.0+
MYPACKAGE_LICENSE_FILES = LICENSE

# Dependencies
MYPACKAGE_DEPENDENCIES = rockchip-mpp libdrm

# Install to staging (for development headers/libraries)
MYPACKAGE_INSTALL_STAGING = YES

# Install to target (for runtime binaries/libraries)
MYPACKAGE_INSTALL_TARGET = YES

# Configuration options
MYPACKAGE_CONF_OPTS = --enable-something

ifeq ($(BR2_PACKAGE_MYPACKAGE_FEATURE_X),y)
MYPACKAGE_CONF_OPTS += --enable-feature-x
endif

# Use the appropriate package infrastructure
$(eval $(autotools-package))
# Or: $(eval $(cmake-package))
# Or: $(eval $(generic-package))
```

### Step 4: Add to Category Menu

Edit `buildroot/package/Config.in` or category-specific file:

```kconfig
menu "Audio and video applications"
    source "package/ffmpeg/Config.in"
    source "package/mypackage/Config.in"  # Add this line
endmenu
```

### Step 5: Enable in Buildroot Config

```bash
cd $SDK_ROOT/buildroot
make menuconfig

# Navigate to your package category
# Select [*] mypackage
# Save and exit
```

### Step 6: Build

```bash
# Build only your package
make mypackage

# Or rebuild if already built
make mypackage-rebuild

# Clean and rebuild
make mypackage-dirclean
make mypackage
```

---

## Package Types

### Generic Package

For projects with custom build system or simple Makefile.

```makefile
################################################################################
#
# myapp
#
################################################################################

MYAPP_VERSION = 1.0.0
MYAPP_SITE = $(TOPDIR)/../external/myapp
MYAPP_SITE_METHOD = local
MYAPP_LICENSE = GPL-2.0
MYAPP_DEPENDENCIES = rockchip-mpp

# Define build commands
define MYAPP_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

# Define install commands
define MYAPP_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/myapp $(TARGET_DIR)/usr/bin/myapp
    $(INSTALL) -D -m 0644 $(@D)/config.conf $(TARGET_DIR)/etc/myapp.conf
endef

$(eval $(generic-package))
```

### CMake Package

For CMake-based projects.

```makefile
################################################################################
#
# mycmakeapp
#
################################################################################

MYCMAKEAPP_VERSION = 1.0.0
MYCMAKEAPP_SITE = https://github.com/yourorg/mycmakeapp.git
MYCMAKEAPP_SITE_METHOD = git
MYCMAKEAPP_LICENSE = MIT
MYCMAKEAPP_LICENSE_FILES = LICENSE

MYCMAKEAPP_DEPENDENCIES = rockchip-mpp rockchip-rga

MYCMAKEAPP_CONF_OPTS = \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_HARDWARE_ACCEL=ON

$(eval $(cmake-package))
```

### Autotools Package

For projects using ./configure (autotools).

```makefile
################################################################################
#
# myautotoolsapp
#
################################################################################

MYAUTOTOOLSAPP_VERSION = 1.0.0
MYAUTOTOOLSAPP_SITE = https://example.com/releases
MYAUTOTOOLSAPP_SOURCE = myautotoolsapp-$(MYAUTOTOOLSAPP_VERSION).tar.gz
MYAUTOTOOLSAPP_LICENSE = GPL-3.0+
MYAUTOTOOLSAPP_LICENSE_FILES = COPYING

MYAUTOTOOLSAPP_DEPENDENCIES = host-pkgconf libdrm

MYAUTOTOOLSAPP_CONF_OPTS = \
    --enable-shared \
    --disable-static \
    --with-mpp=$(STAGING_DIR)/usr

MYAUTOTOOLSAPP_INSTALL_STAGING = YES

$(eval $(autotools-package))
```

---

## FFmpeg-Rockchip Integration Example

Complete example of integrating FFmpeg-Rockchip into buildroot.

### Step 1: Create Package Directory

```bash
cd $SDK_ROOT/buildroot/package
mkdir -p ffmpeg-rockchip
```

### Step 2: Config.in

**package/ffmpeg-rockchip/Config.in:**

```kconfig
config BR2_PACKAGE_FFMPEG_ROCKCHIP
    bool "ffmpeg-rockchip"
    depends on BR2_PACKAGE_FFMPEG_ARCH_SUPPORTS
    select BR2_PACKAGE_LIBDRM
    select BR2_PACKAGE_ROCKCHIP_MPP
    select BR2_PACKAGE_ROCKCHIP_RGA
    help
      FFmpeg with Rockchip MPP and RGA hardware acceleration support.
      
      This is a patched version of FFmpeg that provides:
      - Hardware video decoders (H.264, HEVC, VP9, AV1)
      - Hardware video encoders (H.264, HEVC)
      - Hardware filters (scale, overlay, VPP)
      - Zero-copy DMA pipeline
      
      https://github.com/Fanconn-RV1126B-P/ffmpeg-rockchip

if BR2_PACKAGE_FFMPEG_ROCKCHIP

config BR2_PACKAGE_FFMPEG_ROCKCHIP_GPL
    bool "Enable GPL code"
    help
      Allow use of GPL code, the resulting binary will be under GPL.

config BR2_PACKAGE_FFMPEG_ROCKCHIP_FFMPEG
    bool "Build ffmpeg (CLI tool)"
    default y
    help
      Build the ffmpeg command-line tool.

config BR2_PACKAGE_FFMPEG_ROCKCHIP_FFPROBE
    bool "Build ffprobe (stream analyzer)"
    default y
    help
      Build the ffprobe stream analyzer tool.

config BR2_PACKAGE_FFMPEG_ROCKCHIP_STATIC
    bool "Build static binary"
    default n
    help
      Build statically linked binary (larger but self-contained).

endif  # BR2_PACKAGE_FFMPEG_ROCKCHIP
```

### Step 3: Build Recipe

**package/ffmpeg-rockchip/ffmpeg-rockchip.mk:**

```makefile
################################################################################
#
# ffmpeg-rockchip
#
################################################################################

FFMPEG_ROCKCHIP_VERSION = master
FFMPEG_ROCKCHIP_SITE = https://github.com/Fanconn-RV1126B-P/ffmpeg-rockchip.git
FFMPEG_ROCKCHIP_SITE_METHOD = git
FFMPEG_ROCKCHIP_LICENSE = GPL-2.0+ and LGPL-2.1+
FFMPEG_ROCKCHIP_LICENSE_FILES = LICENSE.md COPYING.GPLv2 COPYING.LGPLv2.1

FFMPEG_ROCKCHIP_DEPENDENCIES = host-pkgconf libdrm rockchip-mpp rockchip-rga

FFMPEG_ROCKCHIP_INSTALL_STAGING = YES

# Base configuration
FFMPEG_ROCKCHIP_CONF_OPTS = \
    --prefix=/usr \
    --enable-cross-compile \
    --cross-prefix=$(TARGET_CROSS) \
    --sysroot=$(STAGING_DIR) \
    --host-cc="$(HOSTCC)" \
    --arch=$(BR2_ARCH) \
    --target-os=linux \
    --pkg-config="$(PKG_CONFIG_HOST_BINARY)" \
    --disable-stripping \
    --enable-libdrm \
    --enable-rkmpp \
    --enable-rkrga \
    --disable-doc \
    --disable-htmlpages \
    --disable-manpages \
    --disable-podpages \
    --disable-txtpages

# GPL option
ifeq ($(BR2_PACKAGE_FFMPEG_ROCKCHIP_GPL),y)
FFMPEG_ROCKCHIP_CONF_OPTS += --enable-gpl --enable-version3
else
FFMPEG_ROCKCHIP_CONF_OPTS += --disable-gpl
endif

# FFmpeg CLI tool
ifeq ($(BR2_PACKAGE_FFMPEG_ROCKCHIP_FFMPEG),y)
FFMPEG_ROCKCHIP_CONF_OPTS += --enable-ffmpeg
else
FFMPEG_ROCKCHIP_CONF_OPTS += --disable-ffmpeg
endif

# FFprobe tool
ifeq ($(BR2_PACKAGE_FFMPEG_ROCKCHIP_FFPROBE),y)
FFMPEG_ROCKCHIP_CONF_OPTS += --enable-ffprobe
else
FFMPEG_ROCKCHIP_CONF_OPTS += --disable-ffprobe
endif

# Static vs shared
ifeq ($(BR2_PACKAGE_FFMPEG_ROCKCHIP_STATIC),y)
FFMPEG_ROCKCHIP_CONF_OPTS += --enable-static --disable-shared
else
FFMPEG_ROCKCHIP_CONF_OPTS += --disable-static --enable-shared
endif

# Configure command
define FFMPEG_ROCKCHIP_CONFIGURE_CMDS
    (cd $(@D) && rm -rf config.cache && \
    $(TARGET_CONFIGURE_OPTS) \
    $(TARGET_CONFIGURE_ARGS) \
    $(FFMPEG_CONF_ENV) \
    ./configure \
        $(FFMPEG_ROCKCHIP_CONF_OPTS) \
    )
endef

# Optional: Strip binaries in post-install
define FFMPEG_ROCKCHIP_STRIP_BINARIES
    $(TARGET_STRIP) $(TARGET_DIR)/usr/bin/ffmpeg || true
    $(TARGET_STRIP) $(TARGET_DIR)/usr/bin/ffprobe || true
endef
FFMPEG_ROCKCHIP_POST_INSTALL_TARGET_HOOKS += FFMPEG_ROCKCHIP_STRIP_BINARIES

$(eval $(autotools-package))
```

### Step 4: Add Hash File (Optional but recommended)

**package/ffmpeg-rockchip/ffmpeg-rockchip.hash:**

```
# Locally computed
sha256  <hash>  ffmpeg-rockchip-<version>.tar.gz
```

### Step 5: Register Package

Edit `package/Config.in`:

```kconfig
menu "Audio and video applications"
    source "package/ffmpeg/Config.in"
    source "package/ffmpeg-rockchip/Config.in"  # Add this
    source "package/gstreamer1/Config.in"
    # ...
endmenu
```

### Step 6: Enable and Build

```bash
cd $SDK_ROOT/buildroot

# Method 1: menuconfig
make menuconfig
# Navigate to: Target packages -> Audio and video applications
# Select: [*] ffmpeg-rockchip
#   [*] Enable GPL code
#   [*] Build ffmpeg (CLI tool)
#   [*] Build ffprobe (stream analyzer)

# Method 2: Add directly to defconfig
echo "BR2_PACKAGE_FFMPEG_ROCKCHIP=y" >> configs/rockchip_rv1126b_defconfig
echo "BR2_PACKAGE_FFMPEG_ROCKCHIP_GPL=y" >> configs/rockchip_rv1126b_defconfig
echo "BR2_PACKAGE_FFMPEG_ROCKCHIP_FFMPEG=y" >> configs/rockchip_rv1126b_defconfig

# Build
make ffmpeg-rockchip

# Rebuild entire system
make
```

---

## Testing and Debugging

### Build Single Package

```bash
# Clean build
make mypackage-dirclean
make mypackage

# Rebuild (keep build directory)
make mypackage-rebuild

# Reconfigure
make mypackage-reconfigure

# Extract source only
make mypackage-extract
```

### Check Package Configuration

```bash
# Show package info
make mypackage-show-info

# Show package dependencies
make mypackage-show-depends

# Show reverse dependencies
make mypackage-show-rdepends
```

### Debugging Build Issues

```bash
# Verbose build
make V=1 mypackage-rebuild

# Check package build directory
cd $SDK_ROOT/buildroot/output/rockchip_rv1126b/build/mypackage-1.0.0/

# Check config.log for autotools packages
cat config.log

# Check CMakeCache for cmake packages
cat CMakeCache.txt
```

### Test on Device

```bash
# Check if binary is in rootfs
ls -l $SDK_ROOT/buildroot/output/rockchip_rv1126b/target/usr/bin/myapp

# Build firmware image
cd $SDK_ROOT
./build.sh

# Flash to device and test
```

---

## Advanced Topics

### Patches

Add patches to `package/mypackage/`:

```bash
package/mypackage/
├── Config.in
├── mypackage.mk
├── 0001-fix-something.patch
├── 0002-add-feature.patch
└── 0003-another-fix.patch
```

Patches are automatically applied during build.

### Post-Install Scripts

```makefile
define MYPACKAGE_INSTALL_INIT_SYSV
    $(INSTALL) -D -m 0755 package/mypackage/S99myapp \
        $(TARGET_DIR)/etc/init.d/S99myapp
endef

define MYPACKAGE_INSTALL_INIT_SYSTEMD
    $(INSTALL) -D -m 0644 package/mypackage/myapp.service \
        $(TARGET_DIR)/usr/lib/systemd/system/myapp.service
endef
```

### Conditional Compilation

```makefile
ifeq ($(BR2_aarch64),y)
MYPACKAGE_CONF_OPTS += --enable-arm64-optimizations
endif

ifeq ($(BR2_PACKAGE_ROCKCHIP_MPP),y)
MYPACKAGE_DEPENDENCIES += rockchip-mpp
MYPACKAGE_CONF_OPTS += --enable-mpp
endif
```

---

## Best Practices

1. **Always add LICENSE and LICENSE_FILES**
2. **Use INSTALL_STAGING for libraries**
3. **Use INSTALL_TARGET for binaries**
4. **Add dependencies explicitly**
5. **Test clean builds**: `make clean && make`
6. **Follow naming conventions**: lowercase with hyphens
7. **Document configuration options** in Config.in help text
8. **Use post-install hooks** sparingly

---

## Common Issues

### Issue 1: Package Not Found in Menu

**Cause:** Not added to Config.in

**Fix:**
```bash
# Check if package is sourced
grep "mypackage" package/Config.in
```

### Issue 2: Dependencies Not Built

**Cause:** Missing DEPENDENCIES

**Fix:**
```makefile
MYPACKAGE_DEPENDENCIES = host-pkgconf rockchip-mpp libdrm
```

### Issue 3: Configure Fails

**Check:**
```bash
# Look at configure command
cat $BUILDROOT_OUTPUT/build/mypackage-1.0.0/config.log

# Try manually
cd $BUILDROOT_OUTPUT/build/mypackage-1.0.0/
# Run configure with same options
```

---

## Next Steps

- **[Building Software with SDK](./building-software-with-sdk.md)** - Overview of build approaches
- **[Cross-Compilation Guide](./cross-compilation-guide.md)** - Detailed cross-compilation

---

## References

- [Buildroot Manual](https://buildroot.org/downloads/manual/manual.html)
- [Buildroot Package Tutorial](https://buildroot.org/downloads/manual/manual.html#adding-packages)

---

**Last Updated:** January 21, 2026
