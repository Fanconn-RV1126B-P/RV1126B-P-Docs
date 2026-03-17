# Complete Build and Flash Guide for RV1126B-P IPC

This comprehensive guide covers building, flashing, testing, and customizing the RV1126BP IPC firmware with RTSP streaming capabilities.

---

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [Configuration Overview](#configuration-overview)
3. [Building Firmware](#building-firmware)
4. [Flashing Firmware](#flashing-firmware)
5. [Testing IPC Features](#testing-ipc-features)
6. [Configuration & Customization](#configuration--customization)
7. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Hardware Requirements
- RV1126BP development board
- USB Type-C cable
- PC running Linux (tested on Ubuntu)
- Network connection for both board and PC

### Software Requirements
- Docker and Docker Compose installed
- upgrade_tool v2.44 (included in SDK)
- VLC or ffplay for RTSP testing
- Web browser for web interface testing

### Host OS compatibility

| Host OS | Support | Notes |
|---|---|---|
| **Ubuntu 22.04 / 24.04** | ✅ Recommended | Native Linux — best performance, no caveats |
| **Mac (Docker Desktop)** | ⚠️ Works with caveats | Volume mounts are significantly slower; root-owned `binary/` chroot files on a Mac-mounted volume may fail to clean up — use `docker exec` to `rm -rf` from inside the container |
| **Windows (Docker Desktop + WSL2)** | ⚠️ Works with caveats | Keep repos inside the WSL2 filesystem (`~/...`), **not** under `/mnt/c/...` — Windows-mounted volumes have severe performance and ownership issues with root-owned chroot files |

> **Recommendation:** Use a native **Ubuntu Linux** host. The build creates root-owned chroot directories, mounts `binfmt_misc`, and runs privileged containers.

### Initial Setup
```bash
# Clone or extract the SDK
cd ~/RV1126B-P/RV1126B-P-SDK/
# SDK should be at: rv1126b_linux6.1_sdk_v1.1.0/

# Docker environment should be at
cd ~/RV1126B-P/RV1126B-P-SDK-Docker/
```

---

## Configuration Overview

The SDK provides multiple configurations. For IPC (RTSP streaming) functionality, use the **IPC 64-bit configuration**.

### Available Configurations

| Configuration | Features | Recommended |
|--------------|----------|-------------|
| `rockchip_rv1126bp_evb1_v10_defconfig` | Basic build, no RKIPC | ❌ For IPC |
| `rockchip_rv1126bp_ipc_32_evb1_v10_defconfig` | IPC with RTSP, 32-bit | ⚠️ Not recommended per Fanconn |
| `rockchip_rv1126bp_ipc_64_evb1_v10_defconfig` | IPC with RTSP, 64-bit | ✅ **Recommended** |

### IPC Configuration Includes
- **RKIPC**: RTSP streaming server
- **Web Interface**: Nginx + Angular frontend
- **Multimedia Stack**: MPP (Media Process Platform), RGA, camera drivers
- **ISP**: Image Signal Processor support for IMX415 sensor
- **Network Services**: RTSP (port 554), HTTP (port 80)

---

## Building Firmware

### Step 1: Start Docker Build Environment

```bash
cd ~/RV1126B-P/RV1126B-P-SDK-Docker

# Start the container
docker compose up -d

# Enter the build environment
docker compose exec rv1126b-builder bash
```

### Step 2: Select IPC 64-bit Configuration

```bash
# Inside Docker container
cd rv1126b_linux6.1_sdk_v1.1.0

# Switch to IPC 64-bit configuration
./build.sh rockchip_rv1126bp_ipc_64_evb1_v10_defconfig

# Verify the configuration
ls -la device/rockchip/.chip/rockchip_defconfig
# Should show: rockchip_defconfig -> rockchip_rv1126bp_ipc_64_evb1_v10_defconfig
```

### Step 3: Build the Firmware

```bash
# Inside Docker container

# Full clean build (first time or major changes)
./build.sh cleanall
./build.sh all

# Or for incremental builds (only changed components)
./build.sh buildroot      # Just rebuild rootfs
./build.sh kernel         # Just rebuild kernel
./build.sh uboot          # Just rebuild U-Boot
./build.sh updateimg      # Regenerate update.img from built images
```

**Build Time**: Full build takes 30-60 minutes depending on your system.

### Step 4: Verify Build Output

```bash
# Still inside Docker
ls -lh rockdev/update.img

# Should see a file around 200-300MB
# Example: -rw-r--r-- 1 root root 287M Jan 23 10:45 update.img
```

### Step 5: Exit Docker

```bash
exit
```

The firmware is now built at `~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/rockdev/update.img`

---

## Flashing Firmware

### Step 1: Put Board in Loader Mode

1. **Power off** the board
2. **Hold** the recovery/maskrom button
3. **Connect** USB Type-C cable to PC
4. **Power on** the board (or press reset)
5. **Release** the button after 2-3 seconds

### Step 2: Verify Device Detected

```bash
lsusb | grep 2207
```

Expected output:
```
Bus 003 Device 002: ID 2207:110f Fuzhou Rockchip Electronics Company USB download gadget
```

### Step 3: Flash Firmware

Navigate to SDK directory on host (not in Docker):

```bash
cd ~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0

# Flash all partitions
sudo ./rkflash.sh all
```

Expected output:
```
Using ~/RV1126B-P/upgrade_tool_v2.17/config.ini
Loading firmware...
Support Type:110F       FW Ver:1.0.00   FW Time:2026-01-21 15:17:51
Loader ver:1.00 Loader Time:2026-01-21 14:34:51
Start to upgrade firmware...
Test Device Start
Test Device Success
Check Chip Start
Check Chip Success
Get FlashInfo Start
Get FlashInfo Success
Prepare IDB Start
Prepare IDB Success
Download IDB Start
Download IDB Success
Download Firmware Start
Download Image... (100%)
Download Firmware Success
Upgrade firmware ok.
```

### Step 4: Reboot

The board will automatically reboot after flashing. Wait 30-60 seconds for the system to fully boot.

---

## Testing IPC Features

### Accessing the Board

The firmware supports multiple access methods:

#### Option 1: ADB via USB

```bash
# Connect via USB Type-C
adb devices

# Access shell
adb shell
```

#### Option 2: ADB via Network

```bash
# First connect via USB to get the board's IP
adb shell ifconfig eth0

# Connect to the board over network
adb connect <BOARD_IP>:5555

# Access shell
adb shell
```

#### Option 3: SSH via Network

```bash
# SSH with default credentials
ssh root@<BOARD_IP>
# Password: rockchip
```

**Default SSH Credentials:**
- Username: `root`
- Password: `rockchip`

### Network Configuration

After booting, the board should get an IP address via DHCP. Find the IP:

```bash
# Option 1: Check your router's DHCP leases

# Option 2: Use ADB via USB
adb shell ifconfig eth0

# Option 3: Use serial console if available
```

Example: The board will get an IP like `192.168.1.x` from your DHCP server. Use `<BOARD_IP>` in the commands below.

### Testing RTSP Streaming

#### Test with VLC

```bash
# Main stream (high resolution)
vlc rtsp://<BOARD_IP>:554/live/0

# Sub stream (lower resolution)
vlc rtsp://<BOARD_IP>:554/live/1
```

#### Test with ffplay

```bash
# Main stream
ffplay -rtsp_transport tcp rtsp://<BOARD_IP>:554/live/0

# Sub stream
ffplay -rtsp_transport tcp rtsp://<BOARD_IP>:554/live/1
```

#### Verify RKIPC is Running

```bash
# Connect to the board (choose one method)
adb shell                    # via USB
adb -s <BOARD_IP>:5555 shell # via network ADB
ssh root@<BOARD_IP>          # via SSH (password: rockchip)

# Check RKIPC process
ps | grep rkipc

# Expected output:
# 1234 root     /usr/bin/rkipc

# Check listening ports
netstat -tulpn | grep :554

# Expected output:
# tcp        0      0 0.0.0.0:554             0.0.0.0:*               LISTEN      1234/rkipc
```

### Testing Web Interface

#### Access Web UI

Open in your browser:
```
http://<BOARD_IP>
```

#### Login Credentials

```
Username: admin
Password: admin
```

#### Web Interface Features

- **Live View**: Real-time video preview
- **Configuration**: Video settings, network, system
- **Recording**: Storage management
- **System**: Firmware info, reboot, factory reset

### Verify Camera Sensor

```bash
# Connect to the board
adb shell  # or: ssh root@<BOARD_IP>

# Check camera device
ls -l /dev/video*

# Expected output:
# crw-rw---- 1 root video 81, 0 Jan 23 10:30 /dev/video0
# crw-rw---- 1 root video 81, 1 Jan 23 10:30 /dev/video1
# ...

# Check ISP and sensor
dmesg | grep -i imx415

# Expected output showing sensor detection:
# [    2.345678] imx415 1-001a: driver version: 00.01.02
# [    2.456789] imx415 1-001a: mode: 3840x2160
```

### RKIPC Auto-Start Verification

RKIPC starts automatically via `/etc/init.d/S99rkipc`. To verify:

```bash
# Connect to the board
adb shell  # or: ssh root@<BOARD_IP>

# Check init script
cat /etc/init.d/S99rkipc

# Verify it runs at boot
ls -la /etc/rc5.d/ | grep rkipc

# Manual restart if needed
killall rkipc
/usr/bin/RkLunch.sh &
```

**Note**: Recent testing confirms RKIPC launches reliably without additional delays. The startup script works as-is.

---

## Configuration & Customization

### What Can Be Changed

1. **Video Settings**: Resolution, bitrate, framerate, codec
2. **Network Settings**: IP address, gateway, DNS
3. **User Credentials**: Admin username and password
4. **Storage**: Recording path, retention policy
5. **System**: Hostname, timezone

### Configuration File Locations

#### RKIPC Configuration

Primary config file:
```bash
/userdata/rkipc.ini
```

Key sections:
```ini
[video.0]
width=3840
height=2160
framerate=25
bitrate=8192

[video.1]
width=1920
height=1080
framerate=25
bitrate=4096

[system]
userName=admin
password=YWRtaW4=    # base64 encoded "admin"
```

#### Web Interface Configuration

```bash
/usr/www/              # Frontend files
/usr/www/cgi-bin/      # Backend CGI scripts
/etc/nginx/nginx.conf  # Nginx config
```

### How to Change Configuration

#### Method 1: Edit Configuration File Directly

```bash
# Connect via ADB (USB or network) or SSH
adb shell           # via USB
ssh root@<BOARD_IP> # via SSH (password: rockchip)

# Edit RKIPC config
vi /userdata/rkipc.ini

# After editing, restart RKIPC
killall rkipc
/usr/bin/RkLunch.sh &
```

#### Method 2: Use Web Interface

1. Login to `http://<BOARD_IP>`
2. Navigate to **Configuration** section
3. Change desired settings
4. Click **Save** (may require reboot)

#### Method 3: Modify During Build

To make permanent changes that survive factory resets:

1. **Edit overlay files**:
   ```bash
   cd ~/RV1126B-P/RV1126B-P-SDK-Docker
   docker compose exec rv1126b-builder bash
   
   cd rv1126b_linux6.1_sdk_v1.1.0/buildroot/board/rockchip/rv1126b/fs-overlay-ipc/
   
   # Edit configuration files here
   # Example: etc/init.d/S99rkipc
   #          etc/nginx/nginx.conf
   ```

2. **Rebuild firmware**:
   ```bash
   ./build.sh buildroot
   ./build.sh updateimg
   exit
   ```

3. **Flash updated firmware**:
   ```bash
   cd ~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0
   sudo ./rkflash.sh all
   ```

### Common Customizations

#### Change Video Resolution

Edit `/userdata/rkipc.ini`:
```ini
[video.0]
width=1920          # Change from 3840
height=1080         # Change from 2160
framerate=30        # Change from 25
bitrate=4096        # Adjust accordingly
```

Restart RKIPC after changes.

#### Change Admin Password

Option 1 - Via config file:
```bash
# Generate base64 encoded password
echo -n "newpassword" | base64
# Output: bmV3cGFzc3dvcmQ=

# Edit config
vi /userdata/rkipc.ini
# Change: password=bmV3cGFzc3dvcmQ=
```

Option 2 - Via web interface:
1. Login as admin
2. Go to **System** → **User Management**
3. Change password
4. Save and logout

#### Change Network Settings

Edit `/etc/network/interfaces`:
```bash
# For static IP
auto eth0
iface eth0 inet static
    address 192.168.1.100
    netmask 255.255.255.0
    gateway 192.168.1.1
    dns-nameservers 8.8.8.8
```

Or use web interface:
1. **Configuration** → **Network** → **Ethernet**
2. Set static IP or DHCP
3. Apply settings

#### Enable/Disable Services

RKIPC startup script location:
```bash
/etc/init.d/S99rkipc
```

To disable auto-start:
```bash
chmod -x /etc/init.d/S99rkipc
```

To enable:
```bash
chmod +x /etc/init.d/S99rkipc
```

---

## Troubleshooting

### RTSP Stream Not Accessible

**Symptoms**: Cannot connect to rtsp://<BOARD_IP>:554/live/0

**Solutions**:
1. Verify RKIPC is running:
   ```bash
   adb shell "ps | grep rkipc"
   ```

2. Check port is listening:
   ```bash
   adb shell "netstat -tulpn | grep :554"
   ```

3. Verify camera sensor detected:
   ```bash
   adb shell "dmesg | grep -i imx415"
   ```

4. Manual restart:
   ```bash
   adb shell "killall rkipc && sleep 1 && /usr/bin/RkLunch.sh &"
   ```

### Web Interface Not Loading

**Symptoms**: Cannot access http://<BOARD_IP>

**Solutions**:
1. Check nginx is running:
   ```bash
   adb shell "ps | grep nginx"
   ```

2. Verify port 80 is listening:
   ```bash
   adb shell "netstat -tulpn | grep :80"
   ```

3. Restart nginx:
   ```bash
   adb shell "/etc/init.d/S50nginx restart"
   ```

### Login Fails with Correct Credentials

**Symptoms**: admin/admin doesn't work

**Solutions**:
1. Check password encoding in config:
   ```bash
   adb shell "cat /userdata/rkipc.ini | grep password"
   ```

2. Verify base64 encoding:
   ```bash
   echo "YWRtaW4=" | base64 -d
   # Should output: admin
   ```

3. Reset to defaults:
   ```bash
   adb shell "rm /userdata/rkipc.ini && reboot"
   ```

### Flash Fails - Device Not Detected

**Symptoms**: upgrade_tool can't find device

**Solutions**:
1. Verify USB connection:
   ```bash
   lsusb | grep 2207
   ```

2. Try different USB port or cable

3. Retry entering loader mode:
   - Power off completely
   - Hold recovery button firmly
   - Connect USB
   - Power on, wait 3 seconds
   - Release button

4. Check USB permissions:
   ```bash
   sudo chmod 666 /dev/bus/usb/003/002  # Adjust bus/device numbers
   ```

### Build Fails in Docker

**Symptoms**: Compilation errors or dependency issues

**Solutions**:
1. Clean and rebuild:
   ```bash
   ./build.sh cleanall
   ./build.sh all
   ```

2. Verify configuration:
   ```bash
   ls -la device/rockchip/.chip/rockchip_defconfig
   ```

3. Check Docker resources:
   ```bash
   docker stats rv1126b-builder
   # Ensure sufficient RAM and CPU
   ```

4. Restart Docker container:
   ```bash
   exit
   docker compose down
   docker compose up -d
   docker compose exec rv1126b-builder bash
   ```

---

## Quick Reference

### One-Liner Commands

**Full rebuild in Docker**:
```bash
cd ~/RV1126B-P/RV1126B-P-SDK-Docker && \
docker compose exec rv1126b-builder bash -c \
"cd rv1126b_linux6.1_sdk_v1.1.0 && ./build.sh cleanall && ./build.sh all"
```

**Quick buildroot rebuild**:
```bash
cd ~/RV1126B-P/RV1126B-P-SDK-Docker && \
docker compose exec rv1126b-builder bash -c \
"cd rv1126b_linux6.1_sdk_v1.1.0 && ./build.sh buildroot && ./build.sh updateimg"
```

**Flash from host**:
```bash
cd ~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0 && \
sudo ./rkflash.sh all
```

**Test RTSP quickly**:
```bash
ffplay -rtsp_transport tcp rtsp://<BOARD_IP>:554/live/0
```

**Restart RKIPC remotely**:
```bash
adb shell "killall rkipc && sleep 1 && /usr/bin/RkLunch.sh &"
```

### Important Paths

| Component | Path |
|-----------|------|
| SDK Root | `~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/` |
| Docker Compose | `~/RV1126B-P/RV1126B-P-SDK-Docker/` |
| Firmware Output | `rockdev/update.img` |
| Buildroot Config | `buildroot/configs/rockchip_rv1126b_ipc_defconfig` |
| IPC Overlay | `buildroot/board/rockchip/rv1126b/fs-overlay-ipc/` |
| Startup Script | `/etc/init.d/S99rkipc` (on device) |
| RKIPC Config | `/userdata/rkipc.ini` (on device) |
| Web Files | `/usr/www/` (on device) |
| RKIPC Binary | `/usr/bin/rkipc` (on device) |

### Default Credentials & URLs

| Service | URL/Address | Credentials |
|---------|-------------|-------------|
| RTSP Main Stream | `rtsp://<BOARD_IP>:554/live/0` | None |
| RTSP Sub Stream | `rtsp://<BOARD_IP>:554/live/1` | None |
| Web Interface | `http://<BOARD_IP>` | admin / admin |
| SSH | `<BOARD_IP>:22` | root / rockchip |
| ADB (network) | `<BOARD_IP>:5555` | None |

---

## Additional Resources

- **SDK Documentation**: `~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/docs/`
- **RKIPC Source**: `~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/app/rkipc/`
- **Web Backend Source**: `~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/app/ipcweb-backend/`
- **Kernel Source**: `~/RV1126B-P/RV1126B-P-SDK/rv1126b_linux6.1_sdk_v1.1.0/kernel-6.1/`

---

**Last Updated**: January 23, 2026  
**Tested Configuration**: rockchip_rv1126bp_ipc_64_evb1_v10_defconfig  
**Tested Hardware**: RV1126BP EVB v1.0 with IMX415 camera sensor
