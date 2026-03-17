# J3 24-Pin Connector Pinout - RV1126B-P Module

## Overview

This document provides complete pinout information for the J3 24-pin expansion connector on the RV1126B-P module, including electrical specifications, GPIO mappings, and device tree references.

## Pin Assignment Table

| Pin | Signal Name | Direction | Electrical Type | Voltage Level | GPIO/Function | Description | Device Tree Notes |
|-----|-------------|-----------|-----------------|---------------|---------------|-------------|-------------------|
| 1 | GND | - | Power Ground | 0V | - | Ground reference | System ground |
| 2 | GND | - | Power Ground | 0V | - | Ground reference | System ground |
| 3 | VCC5V0 | Input | Power Supply | 5.0V | - | 5V power input | Main power rail |
| 4 | VCC5V0 | Input | Power Supply | 5.0V | - | 5V power input | Main power rail |
| 5 | USB_HOST_DP | Bidirectional | USB 2.0 Data | 3.3V | USB Host | USB 2.0 D+ signal | EHCI/OHCI compatible |
| 6 | USB_HOST_DM | Bidirectional | USB 2.0 Data | 3.3V | USB Host | USB 2.0 D- signal | EHCI/OHCI compatible |
| 7 | GND | - | Power Ground | 0V | - | Ground reference | System ground |
| 8 | ADKEY_IN0 | Input | ADC Input | 0-1.8V | ADC/GPIO | ADC key input channel 0 | Analog input for button detection |
| 9 | GND | - | Power Ground | 0V | - | Ground reference | System ground |
| 10 | WIFI_WAKE_HOST | Input | GPIO | 3.3V | GPIO0_B2 | WiFi wake-up host signal | Active high, SDIO WiFi control |
| 11 | BT_RST | Output | GPIO | 3.3V | GPIO0_B1 | Bluetooth reset (active low) | Active low reset signal |
| 12 | BT_WAKE | Output | GPIO | 3.3V | GPIO | Bluetooth wake signal | Host to BT wake |
| 13 | BT_WAKE_HOST | Input | GPIO | 3.3V | GPIO | BT wake host signal | BT to host wake |
| 14 | WIFI_REG_ON | Output | GPIO | 3.3V | GPIO0_B0 | WiFi power enable | Active high power control |
| 15 | SDIO_D0 | Bidirectional | SDIO Data | 3.3V | GPIO1_B4 | SDIO data bit 0 | Pull-up, drive level 2 |
| 16 | SDIO_D1 | Bidirectional | SDIO Data | 3.3V | GPIO1_B5 | SDIO data bit 1 | Pull-up, drive level 2 |
| 17 | SDIO_D2 | Bidirectional | SDIO Data | 3.3V | GPIO1_B6 | SDIO data bit 2 | Pull-up, drive level 2 |
| 18 | SDIO_D3 | Bidirectional | SDIO Data | 3.3V | GPIO1_B7 | SDIO data bit 3 | Pull-up, drive level 2 |
| 19 | SDIO_CMD | Bidirectional | SDIO Command | 3.3V | GPIO1_B3 | SDIO command line | Pull-up, drive level 2 |
| 20 | SDIO_CLK | Output | SDIO Clock | 3.3V | GPIO1_B2 | SDIO clock (up to 200MHz) | Pull-up, drive level 2 |
| 21 | UART0_RTSN | Output | UART Control | 3.3V | GPIO1_C0 | UART0 request to send (active low) | Hardware flow control |
| 22 | UART0_TX | Output | UART Data | 3.3V | GPIO1_C3 | UART0 transmit data | Pull-up, 24MHz base clock |
| 23 | UART0_RX | Input | UART Data | 3.3V | GPIO1_C2 | UART0 receive data | Pull-up, 24MHz base clock |
| 24 | UART0_CTSN | Input | UART Control | 3.3V | GPIO1_C1 | UART0 clear to send (active low) | Hardware flow control |

## GPIO Number Calculation

For pins that can be accessed as GPIOs:

```
GPIO_Number = (GPIO_Bank * 32) + Pin_Offset

Where:
- RK_PA0-PA7 = 0-7
- RK_PB0-PB7 = 8-15
- RK_PC0-PC7 = 16-23
- RK_PD0-PD7 = 24-31
```

### GPIO Mappings

| Signal | GPIO Reference | Calculation | Linux GPIO# |
|--------|---------------|-------------|-------------|
| WIFI_WAKE_HOST | GPIO0_B2 | (0 × 32) + 10 | 10 |
| BT_RST | GPIO0_B1 | (0 × 32) + 9 | 9 |
| WIFI_REG_ON | GPIO0_B0 | (0 × 32) + 8 | 8 |
| SDIO_D0 | GPIO1_B4 | (1 × 32) + 12 | 44 |
| SDIO_D1 | GPIO1_B5 | (1 × 32) + 13 | 45 |
| SDIO_D2 | GPIO1_B6 | (1 × 32) + 14 | 46 |
| SDIO_D3 | GPIO1_B7 | (1 × 32) + 15 | 47 |
| SDIO_CMD | GPIO1_B3 | (1 × 32) + 11 | 43 |
| SDIO_CLK | GPIO1_B2 | (1 × 32) + 10 | 42 |
| UART0_RTSN | GPIO1_C0 | (1 × 32) + 16 | 48 |
| UART0_TX | GPIO1_C3 | (1 × 32) + 19 | 51 |
| UART0_RX | GPIO1_C2 | (1 × 32) + 18 | 50 |
| UART0_CTSN | GPIO1_C1 | (1 × 32) + 17 | 49 |

## Electrical Specifications

### Power Requirements
- **VCC5V0**: 5.0V ±5%
- **Maximum current**: Up to module power consumption (<5W)
- **All signal pins**: 3.3V LVCMOS logic levels
- **Absolute maximum voltage**: -0.3V to 3.6V on signal pins
- **Input high voltage (VIH)**: 2.0V minimum
- **Input low voltage (VIL)**: 0.8V maximum
- **Output high voltage (VOH)**: 2.4V minimum @ 4mA
- **Output low voltage (VOL)**: 0.4V maximum @ 4mA

### Interface Specifications

#### UART0 (Pins 21-24)
- **Compatible**: snps,dw-apb-uart (DesignWare APB UART)
- **Base clock**: 24MHz
- **Max baud rate**: 4Mbps (typical use: 115200-1500000 bps)
- **Features**: Full hardware flow control (RTS/CTS)
- **Data format**: Supports 5-8 bits, 1-2 stop bits, parity options
- **FIFO depth**: 256 bytes TX/RX

#### SDIO Interface (Pins 15-20)
- **Compatible**: rockchip,rv1126-dw-mshc
- **Max frequency**: 200MHz (typical: 50MHz for WiFi modules)
- **Bus width**: 4-bit data
- **Modes supported**: UHS-I SDR104
- **Internal pull-ups**: Enabled, ~47kΩ
- **Drive strength**: Level 2 (8mA)
- **Card detection**: Software-based
- **Power sequencing**: Controlled via GPIO0_B0

#### USB Host (Pins 5-6)
- **Standard**: USB 2.0 High-Speed (480 Mbps)
- **Controllers**: EHCI (High-Speed) and OHCI (Full/Low-Speed)
- **Differential impedance**: 90Ω ±15%
- **Common mode voltage**: 1.65V
- **Power supply**: Requires 5V for bus power
- **Max current per port**: 500mA

#### ADC Input (Pin 8)
- **Resolution**: 10-bit SAR ADC
- **Input voltage range**: 0V to 1.8V
- **Sampling rate**: Up to 1 MSPS
- **Typical use**: Button/key detection via resistor ladder
- **Input impedance**: >1MΩ

## Device Tree References

### UART0 Definition
```dts
uart0: serial@ff560000 {
    compatible = "rockchip,rv1126-uart", "snps,dw-apb-uart";
    reg = <0xff560000 0x100>;
    interrupts = <GIC_SPI 12 IRQ_TYPE_LEVEL_HIGH>;
    reg-shift = <2>;
    reg-io-width = <4>;
    dmas = <&dmac 5>, <&dmac 4>;
    clock-frequency = <24000000>;
    clocks = <&cru SCLK_UART0>, <&cru PCLK_UART0>;
    clock-names = "baudclk", "apb_pclk";
    pinctrl-names = "default";
    pinctrl-0 = <&uart0_xfer &uart0_ctsn &uart0_rtsn>;
    status = "disabled";
};
```

### SDIO Definition
```dts
sdio: dwmmc@ffc70000 {
    compatible = "rockchip,rv1126-dw-mshc", "rockchip,rk3288-dw-mshc";
    reg = <0xffc70000 0x4000>;
    interrupts = <GIC_SPI 77 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&cru HCLK_SDIO>, <&cru CLK_SDIO>,
             <&cru SCLK_SDIO_DRV>, <&cru SCLK_SDIO_SAMPLE>;
    clock-names = "biu", "ciu", "ciu-drive", "ciu-sample";
    fifo-depth = <0x100>;
    max-frequency = <200000000>;
    pinctrl-names = "default";
    pinctrl-0 = <&sdio_clk &sdio_cmd &sdio_bus4>;
    power-domains = <&power RV1126_PD_SDIO>;
    status = "disabled";
};
```

### USB Host Definition
```dts
usb_host0_ehci: usb@ffe00000 {
    compatible = "generic-ehci";
    reg = <0xffe00000 0x10000>;
    interrupts = <GIC_SPI 82 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&cru HCLK_USBHOST>, <&cru HCLK_USBHOST_ARB>,
             <&u2phy1>;
    clock-names = "usbhost", "arbiter", "utmi";
    phys = <&u2phy_host>;
    phy-names = "usb";
    power-domains = <&power RV1126_PD_USB>;
    status = "disabled";
};
```

### Pinctrl Configuration
```dts
uart0 {
    uart0_xfer: uart0-xfer {
        rockchip,pins =
            /* uart0_rx */
            <1 RK_PC2 1 &pcfg_pull_up>,
            /* uart0_tx */
            <1 RK_PC3 1 &pcfg_pull_up>;
    };
    uart0_ctsn: uart0-ctsn {
        rockchip,pins =
            /* uart0_ctsn */
            <1 RK_PC1 1 &pcfg_pull_none>;
    };
    uart0_rtsn: uart0-rtsn {
        rockchip,pins =
            /* uart0_rtsn */
            <1 RK_PC0 1 &pcfg_pull_none>;
    };
};

sdio {
    sdio_bus4: sdio-bus4 {
        rockchip,pins =
            /* sdio_d0 */
            <1 RK_PB4 1 &pcfg_pull_up_drv_level_2>,
            /* sdio_d1 */
            <1 RK_PB5 1 &pcfg_pull_up_drv_level_2>,
            /* sdio_d2 */
            <1 RK_PB6 1 &pcfg_pull_up_drv_level_2>,
            /* sdio_d3 */
            <1 RK_PB7 1 &pcfg_pull_up_drv_level_2>;
    };
    sdio_clk: sdio-clk {
        rockchip,pins =
            /* sdio_clk */
            <1 RK_PB2 1 &pcfg_pull_up_drv_level_2>;
    };
    sdio_cmd: sdio-cmd {
        rockchip,pins =
            /* sdio_cmd */
            <1 RK_PB3 1 &pcfg_pull_up_drv_level_2>;
    };
};
```

### GPIO and Control Signals
```dts
sdio_pwrseq: sdio-pwrseq {
    compatible = "mmc-pwrseq-simple";
    pinctrl-names = "default";
    pinctrl-0 = <&wifi_enable_h>;
    reset-gpios = <&gpio0 RK_PB0 GPIO_ACTIVE_LOW>;
};

wireless-wlan {
    compatible = "wlan-platdata";
    rockchip,grf = <&grf>;
    pinctrl-names = "default";
    pinctrl-0 = <&wifi_wake_host>;
    wifi_chip_type = "rk96x";
    WIFI,host_wake_irq = <&gpio0 RK_PB2 GPIO_ACTIVE_HIGH>;
    status = "okay";
};

wireless-bluetooth {
    compatible = "bluetooth-platdata";
    uart_rts_gpios = <&gpio3 RK_PA6 GPIO_ACTIVE_LOW>;
    pinctrl-names = "default", "rts_gpio";
    pinctrl-0 = <&uart2m0_rtsn_pins>;
    pinctrl-1 = <&uart2_gpios>;
    BT,power_gpio = <&gpio0 RK_PB1 GPIO_ACTIVE_HIGH>;
    status = "okay";
};
```

## PCB Design Guidelines

### General Layout Recommendations

1. **Ground Plane**: Use solid ground plane under all signal traces
2. **Power Decoupling**: 
   - Place 100nF ceramic capacitor near each VCC5V0 pin
   - Add 10µF bulk capacitor near connector
3. **Signal Integrity**:
   - Keep high-speed signal traces short and direct
   - Route differential pairs (USB D+/D-) together with matched lengths
   - Maintain 90Ω differential impedance for USB signals

### USB Interface Design

- **Differential Pair Routing**:
  - Keep D+ and D- traces parallel and matched (±5mil)
  - Differential impedance: 90Ω ±15%
  - Trace spacing: ~6-8mil for typical 2-layer PCB
  - Trace width: ~10-12mil (depends on stackup)
  
- **Series Termination**:
  - Add 27Ω series resistors on D+ and D- close to connector
  - Place resistors within 0.5" of connector pins
  
- **ESD Protection**:
  - Use TVS diode array for ESD protection on D+/D- lines
  - Recommended: TPD2E001 or similar

### SDIO Interface Design

- **Controlled Impedance**:
  - Single-ended impedance: 50Ω ±10%
  - Route CLK, CMD, and DATA traces with 50Ω impedance
  
- **Length Matching**:
  - Match all data lines within 50mil of each other
  - Match CMD to data lines within 50mil
  - CLK can be slightly longer (phase relationship adjustable in software)
  
- **Termination**:
  - Internal pull-ups are enabled in SoC
  - External pull-ups: 10kΩ-47kΩ on CMD and DATA lines (optional)
  
- **Drive Strength**:
  - Configure for level 2 (8mA) in device tree
  - Can adjust based on trace length and loading

### UART Interface Design

- **Signal Routing**:
  - Keep UART traces short and direct
  - No special impedance control required
  - Can use standard PCB routing
  
- **Level Shifting** (if needed):
  - For 5V compatibility, use level shifter like TXS0102
  - Or use voltage divider on RX (3.3V to 5V direction not needed)
  
- **Protection**:
  - Add series resistors (100Ω-220Ω) for basic protection
  - TVS diodes if connector is externally accessible

### GPIO and Control Signals

- **Pull-up/Pull-down**:
  - Internal weak pull resistors are configured in device tree
  - Add external 10kΩ pull resistors if stronger pull is needed
  
- **Signal Integrity**:
  - Standard digital I/O routing guidelines apply
  - Keep traces under 6 inches for reliable operation
  
- **Drive Capability**:
  - GPIO output current: 8mA typical, 12mA maximum
  - Do not directly drive LEDs without current-limiting resistors

### Power Supply Design

- **Input Filtering**:
  - Add ferrite bead (600Ω @ 100MHz) on VCC5V0
  - Follow with 10µF + 100nF ceramic capacitors
  
- **Current Capacity**:
  - Ensure power supply can provide >1A peak current
  - Typical module consumption: <5W (<1A @ 5V)
  
- **Protection**:
  - Add reverse polarity protection (Schottky diode or P-FET)
  - Overvoltage protection recommended (TVS or Zener)

### ADC Input Design

- **Input Protection**:
  - Clamp to 0V-1.8V range with Schottky diodes
  - Series resistor: 1kΩ-10kΩ for current limiting
  
- **Filtering**:
  - Add RC filter (1kΩ + 100nF) for noise immunity
  - Place capacitor close to ADC pin
  
- **Button/Key Detection**:
  - Use resistor ladder with different values
  - Typical values: 0Ω, 1kΩ, 2.2kΩ, 4.7kΩ, 10kΩ
  - Pull-up to 1.8V reference

### Connector Selection

- **Recommended Connectors**:
  - 2×12 pin 2.54mm (0.1") pitch header
  - Samtec SSW-112-02-G-D or equivalent
  - Molex 90142 series
  - TE Connectivity Ampmodu II

- **Mating Connectors**:
  - IDC box header: 2×12 position
  - PCB socket: 2.54mm pitch
  - Wire-to-board: Use strain relief

## Revision History

| Date | Version | Description |
|------|---------|-------------|
| 2026-01-24 | 1.0 | Initial release with complete pinout and specifications |

## References

- RV1126B-P Module Specification v1.0
- RV1126 SoC Device Tree (rv1126.dtsi)
- RV1126 Pinctrl Configuration (rv1126-pinctrl.dtsi)
- Rockchip GPIO/Pinctrl Driver Documentation
