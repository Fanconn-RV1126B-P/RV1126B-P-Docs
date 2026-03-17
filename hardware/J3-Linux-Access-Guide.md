# J3 Connector Linux Access Guide - RV1126B-P Module

## Overview

This guide provides detailed instructions for accessing and controlling the J3 connector pins from Linux userspace on the RV1126B-P module.

## Linux Interface Mapping

| Pin | Signal | Linux Interface | Device Path | sysfs Path | GPIO# |
|-----|--------|----------------|-------------|------------|-------|
| 1-2, 7, 9 | GND | - | - | - | - |
| 3-4 | VCC5V0 | Power monitoring | - | `/sys/class/power_supply/` | - |
| 5 | USB_HOST_DP | USB subsystem | `/dev/bus/usb/` | `/sys/bus/usb/devices/` | - |
| 6 | USB_HOST_DM | USB subsystem | `/dev/bus/usb/` | `/sys/bus/usb/devices/` | - |
| 8 | ADKEY_IN0 | IIO/ADC | `/dev/iio:device*` | `/sys/bus/iio/devices/iio:device*/` | - |
| 10 | WIFI_WAKE_HOST | GPIO | - | `/sys/class/gpio/gpio10/` | 10 |
| 11 | BT_RST | GPIO | - | `/sys/class/gpio/gpio9/` | 9 |
| 12 | BT_WAKE | Bluetooth/rfkill | - | `/sys/class/rfkill/` | - |
| 13 | BT_WAKE_HOST | Bluetooth/rfkill | - | `/sys/class/rfkill/` | - |
| 14 | WIFI_REG_ON | GPIO | - | `/sys/class/gpio/gpio8/` | 8 |
| 15 | SDIO_D0 | MMC/SDIO | `/dev/mmcblk2` | `/sys/class/mmc_host/mmc2/` | 44 |
| 16 | SDIO_D1 | MMC/SDIO | `/dev/mmcblk2` | `/sys/class/mmc_host/mmc2/` | 45 |
| 17 | SDIO_D2 | MMC/SDIO | `/dev/mmcblk2` | `/sys/class/mmc_host/mmc2/` | 46 |
| 18 | SDIO_D3 | MMC/SDIO | `/dev/mmcblk2` | `/sys/class/mmc_host/mmc2/` | 47 |
| 19 | SDIO_CMD | MMC/SDIO | `/dev/mmcblk2` | `/sys/class/mmc_host/mmc2/` | 43 |
| 20 | SDIO_CLK | MMC/SDIO | `/dev/mmcblk2` | `/sys/class/mmc_host/mmc2/` | 42 |
| 21 | UART0_RTSN | UART | `/dev/ttyS0` | `/sys/class/tty/ttyS0/` | 48 |
| 22 | UART0_TX | UART | `/dev/ttyS0` | `/sys/class/tty/ttyS0/` | 51 |
| 23 | UART0_RX | UART | `/dev/ttyS0` | `/sys/class/tty/ttyS0/` | 50 |
| 24 | UART0_CTSN | UART | `/dev/ttyS0` | `/sys/class/tty/ttyS0/` | 49 |

## 1. UART0 Access (Pins 21-24)

### Basic UART Operations

#### Check UART Availability
```bash
# List available serial ports
ls -l /dev/ttyS*

# Check UART0 specifically
ls -l /dev/ttyS0

# View UART device information
cat /sys/class/tty/ttyS0/dev
cat /sys/class/tty/ttyS0/port_type
```

#### Configure UART Settings
```bash
# View current settings
stty -F /dev/ttyS0 -a

# Configure 115200 8N1 (8 data bits, no parity, 1 stop bit)
stty -F /dev/ttyS0 115200 cs8 -cstopb -parenb

# Enable hardware flow control (RTS/CTS)
stty -F /dev/ttyS0 crtscts

# Disable hardware flow control
stty -F /dev/ttyS0 -crtscts

# Set raw mode (no processing)
stty -F /dev/ttyS0 raw -echo

# Common baud rates
stty -F /dev/ttyS0 9600      # 9600 bps
stty -F /dev/ttyS0 115200    # 115200 bps
stty -F /dev/ttyS0 921600    # 921600 bps
stty -F /dev/ttyS0 1500000   # 1.5 Mbps
```

#### Send and Receive Data
```bash
# Send data
echo "Hello World" > /dev/ttyS0

# Send binary data
printf "\x01\x02\x03" > /dev/ttyS0

# Receive data (blocking read)
cat /dev/ttyS0

# Receive with timeout (1 second)
timeout 1 cat /dev/ttyS0

# Use dd for binary transfers
dd if=/dev/ttyS0 of=output.bin bs=1 count=100

# Send file
cat file.txt > /dev/ttyS0
```

#### Interactive Terminal Access
```bash
# Using screen
screen /dev/ttyS0 115200

# Using minicom
minicom -D /dev/ttyS0 -b 115200

# Using picocom
picocom -b 115200 /dev/ttyS0

# Exit screen: Ctrl+A then K
# Exit minicom: Ctrl+A then X
# Exit picocom: Ctrl+A then Ctrl+X
```

#### UART Statistics and Debugging
```bash
# View serial port statistics
cat /proc/tty/driver/serial

# Check for errors
cat /proc/interrupts | grep uart

# Kernel messages related to UART
dmesg | grep -i uart
dmesg | grep ttyS0

# Monitor UART events
udevadm monitor --kernel --subsystem-match=tty
```

### UART Loopback Test
```bash
#!/bin/bash
# Connect TX (pin 22) to RX (pin 23) externally for this test

PORT="/dev/ttyS0"
TEST_STRING="UART_LOOPBACK_TEST_12345"

# Configure port
stty -F $PORT 115200 raw -echo -crtscts

# Send test string
echo "$TEST_STRING" > $PORT &

# Receive and compare
RECEIVED=$(timeout 2 cat $PORT | tr -d '\0')

if [ "$RECEIVED" = "$TEST_STRING" ]; then
    echo "✓ Loopback test passed!"
else
    echo "✗ Loopback test failed"
    echo "Expected: $TEST_STRING"
    echo "Received: $RECEIVED"
fi
```

### Python UART Example
```python
#!/usr/bin/env python3
import serial
import time

# Open UART0
ser = serial.Serial(
    port='/dev/ttyS0',
    baudrate=115200,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=1,
    rtscts=True  # Enable hardware flow control
)

# Send data
ser.write(b'Hello from Python\n')

# Receive data
data = ser.read(100)  # Read up to 100 bytes
print(f"Received: {data}")

# Close port
ser.close()
```

---

## 2. SDIO Interface (Pins 15-20)

### Check SDIO Status

```bash
# List MMC/SDIO hosts
ls -l /sys/class/mmc_host/

# Check SDIO device (typically mmc2 for sdio)
ls -l /sys/class/mmc_host/mmc2/

# View SDIO card information
cat /sys/class/mmc_host/mmc2/mmc2:0001/type
cat /sys/class/mmc_host/mmc2/mmc2:0001/vendor
cat /sys/class/mmc_host/mmc2/mmc2:0001/device
cat /sys/class/mmc_host/mmc2/mmc2:0001/name

# Check SDIO timing
cat /sys/class/mmc_host/mmc2/mmc2:0001/timing

# View SDIO clock frequency
cat /sys/kernel/debug/mmc2/ios

# Check SDIO power state
cat /sys/class/mmc_host/mmc2/mmc2:0001/power/runtime_status
```

### WiFi Interface (over SDIO)

```bash
# Check WiFi interface
ifconfig -a | grep wlan
ip link show wlan0

# Bring interface up
ip link set wlan0 up
ifconfig wlan0 up

# Scan for networks
iw dev wlan0 scan | grep -E "SSID|signal"
iwlist wlan0 scan | grep -E "ESSID|Quality"

# Connect to network
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf
dhclient wlan0

# Check connection status
iw dev wlan0 link
iw dev wlan0 info

# View WiFi statistics
iw dev wlan0 station dump
cat /proc/net/wireless

# Check WiFi driver
lsmod | grep -E "brcm|wifi|sdio"
dmesg | grep -i sdio | tail -20
```

### SDIO Debugging

```bash
# Enable SDIO debug messages
echo 8 > /proc/sys/kernel/printk
echo 'file drivers/mmc/* +p' > /sys/kernel/debug/dynamic_debug/control

# Monitor SDIO events
dmesg -w | grep -i sdio

# Check SDIO interrupts
cat /proc/interrupts | grep mmc

# Test SDIO rescan
echo 1 > /sys/class/mmc_host/mmc2/rescan

# Check SDIO power sequencing
cat /sys/kernel/debug/mmc2/pwrseq
```

---

## 3. GPIO Control (Pins 10, 11, 14)

### Legacy sysfs GPIO Interface

#### Export and Control GPIOs
```bash
# Export GPIO8 (WIFI_REG_ON)
echo 8 > /sys/class/gpio/export

# Set direction (in/out)
echo "out" > /sys/class/gpio/gpio8/direction
echo "in" > /sys/class/gpio/gpio10/direction

# Read GPIO value
cat /sys/class/gpio/gpio10/value

# Set GPIO value (0=low, 1=high)
echo 1 > /sys/class/gpio/gpio8/value   # Set high
echo 0 > /sys/class/gpio/gpio8/value   # Set low

# Configure as active-low
echo 1 > /sys/class/gpio/gpio8/active_low

# Configure interrupt edge
echo "rising" > /sys/class/gpio/gpio10/edge    # Rising edge
echo "falling" > /sys/class/gpio/gpio10/edge   # Falling edge
echo "both" > /sys/class/gpio/gpio10/edge      # Both edges
echo "none" > /sys/class/gpio/gpio10/edge      # Disable

# Unexport GPIO when done
echo 8 > /sys/class/gpio/unexport
```

#### Complete GPIO Control Script
```bash
#!/bin/bash
# Control WiFi power (GPIO8 - WIFI_REG_ON)

GPIO=8

# Export GPIO
if [ ! -d /sys/class/gpio/gpio$GPIO ]; then
    echo $GPIO > /sys/class/gpio/export
    sleep 0.1
fi

# Set as output
echo out > /sys/class/gpio/gpio$GPIO/direction

# Power on WiFi
echo 1 > /sys/class/gpio/gpio$GPIO/value
echo "WiFi powered ON"

sleep 2

# Power off WiFi
echo 0 > /sys/class/gpio/gpio$GPIO/value
echo "WiFi powered OFF"

# Cleanup
echo $GPIO > /sys/class/gpio/unexport
```

### Modern libgpiod Interface (Recommended)

#### Install libgpiod Tools
```bash
# On Debian/Ubuntu/Buildroot
apt-get install gpiod libgpiod-dev

# Or build from source
git clone https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
cd libgpiod
./autogen.sh --enable-tools=yes
make && make install
```

#### Using gpiodetect and gpioinfo
```bash
# Detect GPIO chips
gpiodetect
# Output: gpiochip0 [gpio0] (32 lines)
#         gpiochip1 [gpio1] (32 lines)

# Get information about specific chip
gpioinfo gpiochip0
gpioinfo gpiochip1

# Find specific GPIO line
gpioinfo gpiochip0 | grep -A2 "line.*8"
```

#### GPIO Operations with libgpiod
```bash
# Read GPIO value
gpioget gpiochip0 10              # Read GPIO0_B2 (WIFI_WAKE_HOST)
gpioget gpiochip1 18              # Read GPIO1_C2 (UART0_RX)

# Set GPIO value
gpioset gpiochip0 8=1             # Set GPIO0_B0 high (WiFi on)
gpioset gpiochip0 8=0             # Set GPIO0_B0 low (WiFi off)
gpioset gpiochip0 9=0             # Assert BT reset (active low)

# Set multiple GPIOs
gpioset gpiochip0 8=1 9=1         # Set GPIO8 and GPIO9 high

# Monitor GPIO for changes
gpiomon gpiochip0 10              # Monitor GPIO0_B2
gpiomon --rising-edge gpiochip0 10
gpiomon --falling-edge gpiochip0 10

# Toggle GPIO with timing
gpioset --mode=time --sec=2 gpiochip0 8=1  # High for 2 seconds

# Toggle continuously
while true; do
    gpioset gpiochip0 8=1
    sleep 0.5
    gpioset gpiochip0 8=0
    sleep 0.5
done
```

#### C Example with libgpiod
```c
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;

    // Open GPIO chip
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    // Get GPIO line 8 (WIFI_REG_ON)
    line = gpiod_chip_get_line(chip, 8);
    if (!line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return 1;
    }

    // Request line as output
    ret = gpiod_line_request_output(line, "wifi-power", 0);
    if (ret < 0) {
        perror("gpiod_line_request_output");
        gpiod_chip_close(chip);
        return 1;
    }

    // Toggle GPIO
    gpiod_line_set_value(line, 1);
    printf("WiFi power ON\n");
    sleep(2);
    
    gpiod_line_set_value(line, 0);
    printf("WiFi power OFF\n");

    // Cleanup
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    
    return 0;
}

// Compile: gcc -o gpio_test gpio_test.c -lgpiod
```

### Python GPIO Example
```python
#!/usr/bin/env python3
import gpiod
import time

# Open GPIO chip
chip = gpiod.Chip('gpiochip0')

# Get GPIO line 8 (WIFI_REG_ON)
line = chip.get_line(8)

# Request as output
line.request(consumer='wifi-power', type=gpiod.LINE_REQ_DIR_OUT)

try:
    # Toggle GPIO
    line.set_value(1)
    print("WiFi power ON")
    time.sleep(2)
    
    line.set_value(0)
    print("WiFi power OFF")
    
finally:
    line.release()
    chip.close()
```

---

## 4. USB Host (Pins 5-6)

### USB Device Detection

```bash
# List USB devices
lsusb
lsusb -v
lsusb -t  # Tree view

# Detailed USB information
cat /sys/kernel/debug/usb/devices

# List USB devices by path
ls -l /sys/bus/usb/devices/

# Check specific USB device
ls -l /sys/bus/usb/devices/1-1/

# View USB device details
cat /sys/bus/usb/devices/1-1/manufacturer
cat /sys/bus/usb/devices/1-1/product
cat /sys/bus/usb/devices/1-1/serial
cat /sys/bus/usb/devices/1-1/speed
```

### USB Host Controller

```bash
# Check USB host controllers
ls -l /sys/bus/platform/drivers/ehci-platform/
ls -l /sys/bus/platform/drivers/ohci-platform/

# View controller status
cat /sys/bus/platform/devices/ffe00000.usb/driver_override

# USB power management
cat /sys/bus/usb/devices/usb*/power/runtime_status
cat /sys/bus/usb/devices/usb*/power/control

# Disable USB autosuspend
echo on > /sys/bus/usb/devices/usb1/power/control
```

### Monitor USB Events

```bash
# Monitor USB hotplug events
udevadm monitor --kernel --subsystem-match=usb

# Watch USB connections
watch -n 1 lsusb

# USB kernel messages
dmesg | grep -i usb
dmesg -w | grep -i usb

# USB interrupts
cat /proc/interrupts | grep usb
```

### USB Device Testing

```bash
# Test USB mass storage
# Insert USB flash drive, then:
lsblk
mount /dev/sda1 /mnt
ls /mnt
umount /mnt

# Test USB serial device
ls /dev/ttyUSB*
screen /dev/ttyUSB0 115200

# USB speed test
hdparm -tT /dev/sda

# Benchmark USB performance
dd if=/dev/zero of=/mnt/test.bin bs=1M count=100 conv=fdatasync
```

---

## 5. ADC Input (Pin 8 - ADKEY_IN0)

### IIO (Industrial I/O) Interface

```bash
# Find IIO devices
ls /sys/bus/iio/devices/

# Check ADC device
ls -l /sys/bus/iio/devices/iio:device0/

# List available channels
ls /sys/bus/iio/devices/iio:device0/in_voltage*

# Read raw ADC value
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw

# Read ADC scale (conversion factor)
cat /sys/bus/iio/devices/iio:device0/in_voltage0_scale

# Read sampling frequency
cat /sys/bus/iio/devices/iio:device0/sampling_frequency
```

### Calculate Voltage

```bash
#!/bin/bash
# Read ADC and convert to voltage

DEVICE="/sys/bus/iio/devices/iio:device0"

# Read raw value
RAW=$(cat $DEVICE/in_voltage0_raw)

# Read scale
SCALE=$(cat $DEVICE/in_voltage0_scale)

# Calculate voltage in volts
VOLTAGE=$(echo "scale=3; $RAW * $SCALE / 1000" | bc)

echo "Raw ADC value: $RAW"
echo "Scale: $SCALE"
echo "Voltage: ${VOLTAGE}V"
```

### Continuous ADC Monitoring

```bash
#!/bin/bash
# Monitor ADC value continuously

DEVICE="/sys/bus/iio/devices/iio:device0"

echo "Monitoring ADKEY_IN0 (Ctrl+C to stop)"
echo "Time,Raw,Voltage(V)"

while true; do
    RAW=$(cat $DEVICE/in_voltage0_raw)
    SCALE=$(cat $DEVICE/in_voltage0_scale)
    VOLTAGE=$(echo "scale=3; $RAW * $SCALE / 1000" | bc)
    TIMESTAMP=$(date +%H:%M:%S)
    
    echo "$TIMESTAMP,$RAW,$VOLTAGE"
    sleep 0.1
done
```

### Button Detection Example

```bash
#!/bin/bash
# Detect button presses via resistor ladder

DEVICE="/sys/bus/iio/devices/iio:device0"

# Button thresholds (adjust based on your resistor ladder)
BTN1_MIN=0
BTN1_MAX=200
BTN2_MIN=300
BTN2_MAX=500
BTN3_MIN=600
BTN3_MAX=800
BTN4_MIN=900
BTN4_MAX=1100

while true; do
    RAW=$(cat $DEVICE/in_voltage0_raw)
    
    if [ $RAW -ge $BTN1_MIN ] && [ $RAW -le $BTN1_MAX ]; then
        echo "Button 1 pressed"
    elif [ $RAW -ge $BTN2_MIN ] && [ $RAW -le $BTN2_MAX ]; then
        echo "Button 2 pressed"
    elif [ $RAW -ge $BTN3_MIN ] && [ $RAW -le $BTN3_MAX ]; then
        echo "Button 3 pressed"
    elif [ $RAW -ge $BTN4_MIN ] && [ $RAW -le $BTN4_MAX ]; then
        echo "Button 4 pressed"
    fi
    
    sleep 0.05
done
```

---

## 6. Bluetooth (Pins 11-13)

### Bluetooth Control

```bash
# Check Bluetooth devices
hciconfig
hciconfig -a

# List rfkill devices
rfkill list
rfkill list bluetooth

# Enable Bluetooth
rfkill unblock bluetooth
hciconfig hci0 up

# Disable Bluetooth
rfkill block bluetooth
hciconfig hci0 down

# Reset Bluetooth
hciconfig hci0 reset

# Bluetooth device info
cat /sys/class/rfkill/rfkill*/name
cat /sys/class/rfkill/rfkill*/state
cat /sys/class/rfkill/rfkill*/type
```

### Bluetooth Scanning and Pairing

```bash
# Scan for devices (classic Bluetooth)
hcitool scan

# Scan for BLE devices
hcitool lescan

# Using bluetoothctl
bluetoothctl
> power on
> agent on
> default-agent
> scan on
> pair XX:XX:XX:XX:XX:XX
> connect XX:XX:XX:XX:XX:XX
> quit
```

### Bluetooth Serial (RFCOMM)

```bash
# Bind Bluetooth serial port
rfcomm bind /dev/rfcomm0 XX:XX:XX:XX:XX:XX 1

# Use Bluetooth serial
screen /dev/rfcomm0 115200

# Release binding
rfcomm release /dev/rfcomm0
```

---

## 7. System-Wide Debugging

### Kernel Messages

```bash
# View all kernel messages
dmesg

# Follow kernel messages
dmesg -w

# Filter by subsystem
dmesg | grep -i gpio
dmesg | grep -i uart
dmesg | grep -i sdio
dmesg | grep -i usb

# Show timestamps
dmesg -T
```

### Device Tree Information

```bash
# View active device tree
ls /sys/firmware/devicetree/base/

# Check specific node
cat /sys/firmware/devicetree/base/model

# Find GPIO configuration
find /sys/firmware/devicetree/base/ -name "*gpio*"

# Check UART configuration
find /sys/firmware/devicetree/base/ -name "*uart*"
```

### Pinctrl Configuration

```bash
# View pinctrl status (requires debugfs)
mount -t debugfs none /sys/kernel/debug
cat /sys/kernel/debug/pinctrl/pinctrl-rockchip-pinctrl/pinmux-pins
cat /sys/kernel/debug/pinctrl/pinctrl-rockchip-pinctrl/pins
```

### Interrupt Statistics

```bash
# View all interrupts
cat /proc/interrupts

# Filter specific interrupts
cat /proc/interrupts | grep -E "uart|gpio|usb|mmc"

# Watch interrupt counts
watch -n 1 'cat /proc/interrupts | grep -E "uart|gpio|usb|mmc"'
```

### Module Management

```bash
# List loaded modules
lsmod

# Load module
modprobe module_name

# Unload module
modprobe -r module_name

# Module information
modinfo brcmfmac        # WiFi driver
modinfo hci_uart        # Bluetooth
modinfo dwmmc_rockchip  # SDIO controller
```

---

## Quick Reference Scripts

### All-in-One Test Script

```bash
#!/bin/bash
# Comprehensive J3 connector test script

echo "=== J3 Connector Test ==="
echo

echo "1. UART0 Status:"
ls -l /dev/ttyS0 && echo "✓ UART0 available" || echo "✗ UART0 not found"
echo

echo "2. SDIO Status:"
ls -l /sys/class/mmc_host/mmc2/ && echo "✓ SDIO controller present" || echo "✗ SDIO not found"
echo

echo "3. USB Host Status:"
lsusb > /dev/null && echo "✓ USB host working" || echo "✗ USB host not working"
echo

echo "4. GPIO Status:"
for gpio in 8 9 10; do
    if [ -d /sys/class/gpio/gpio$gpio ] || [ -c /dev/gpiochip0 ]; then
        echo "✓ GPIO$gpio accessible"
    else
        echo "✗ GPIO$gpio not accessible"
    fi
done
echo

echo "5. ADC Status:"
ls -l /sys/bus/iio/devices/iio:device0/ && echo "✓ ADC available" || echo "✗ ADC not found"
echo

echo "Test complete!"
```

### WiFi Power Control

```bash
#!/bin/bash
# WiFi power control via GPIO8

case "$1" in
    on)
        echo 8 > /sys/class/gpio/export 2>/dev/null
        echo out > /sys/class/gpio/gpio8/direction
        echo 1 > /sys/class/gpio/gpio8/value
        echo "WiFi powered ON"
        ;;
    off)
        echo 8 > /sys/class/gpio/export 2>/dev/null
        echo out > /sys/class/gpio/gpio8/direction
        echo 0 > /sys/class/gpio/gpio8/value
        echo "WiFi powered OFF"
        ;;
    status)
        if [ -d /sys/class/gpio/gpio8 ]; then
            VALUE=$(cat /sys/class/gpio/gpio8/value)
            [ "$VALUE" = "1" ] && echo "WiFi is ON" || echo "WiFi is OFF"
        else
            echo "GPIO not exported"
        fi
        ;;
    *)
        echo "Usage: $0 {on|off|status}"
        exit 1
        ;;
esac
```

Save this script as `/usr/local/bin/wifi-power` and make it executable:
```bash
chmod +x /usr/local/bin/wifi-power
wifi-power on
wifi-power off
wifi-power status
```

---

## Troubleshooting

### Common Issues

1. **Permission Denied on Device Access**
   ```bash
   # Add user to required groups
   usermod -a -G dialout,gpio,plugdev $USER
   
   # Or use sudo
   sudo cat /dev/ttyS0
   ```

2. **GPIO Already Exported**
   ```bash
   # Unexport first
   echo 8 > /sys/class/gpio/unexport
   # Then export again
   echo 8 > /sys/class/gpio/export
   ```

3. **UART Not Responding**
   ```bash
   # Check if device is in use
   lsof /dev/ttyS0
   
   # Kill process using it
   fuser -k /dev/ttyS0
   ```

4. **USB Device Not Detected**
   ```bash
   # Check power
   cat /sys/bus/usb/devices/usb1/power/control
   
   # Reset USB bus
   echo 0 > /sys/bus/usb/devices/usb1/authorized
   echo 1 > /sys/bus/usb/devices/usb1/authorized
   ```

---

## References

- Linux GPIO Subsystem Documentation: `/usr/share/doc/linux-doc/gpio/`
- IIO Documentation: https://www.kernel.org/doc/html/latest/driver-api/iio/
- libgpiod: https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/
- Linux Serial HOWTO: https://tldp.org/HOWTO/Serial-HOWTO.html
