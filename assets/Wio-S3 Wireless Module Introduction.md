# Wio-S3 Wireless Module Introduction

URL: https://wiki.seeedstudio.com/wio-s3_wireless_module_introduction/

![pir](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/Wio-S3-Wireless-Module-with-IPEX.jpg)

[**Get One Now 🖱️**](https://www.seeedstudio.com/Wio-S3-Wireless-Module-p-6832.html)
SenseCAP Module Comparison
![appearance](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/Module_comparision_1.jpg)
![appearance](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/Module_comparision_2.jpg)

## Product Introduction

[Wio-S3 wireless module](https://www.seeedstudio.com/Wio-S3-Wireless-Module-p-6832.html) is embedded with the [ESP32-S3R8](https://www.espressif.com/en/products/socs/esp32-s3/) dual core Xtensa® LX7 MCU and the long range [SX1262 LoRa chip](https://www.semtech.com/products/wireless-rf/lora-connect/sx1262) . It supports LoRaWAN frequency plan on EU868, US915 and more, as well as **2.4GHz Wi-Fi and BLE 5.0 connectivity** . It integrates 16MB Flash and 8MB PSRAM, making it ideal for edge AI processing.

We offer two versions (with or without an onboard IPEX connector), offering developers versatile antenna integration options. Equipped with a TCXO, it ensures stable and accurate RF performance, which is suitable for IoT applications that require long range, high performance, and reliable wireless communication.

info

| SKU | Name | Specification |
| --- | --- | --- | 
| 100020327 | Wio-S3 Wireless Module | With IPEX connector | 
| 100079384 | Wio-S3-N Wireless Module | Without IPEX connector | 

![Wio-S3](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/Wio-S3-Wireless-Module.jpg)

tip
If you are not quite familiar with LoRa® and LoRaWAN®, check out this article [LoRapedia](https://www.seeedstudio.com/blog/2020/08/03/lorapedia-an-introduction-of-lora-and-lorawan-technology/) for detail.

## Features

- **Powerful MCU** : Based on ESP32-S3R8 dual-core Xtensa® LX7 MCU with clock speeds up to 240 MHz.
- **Large Memory Capacity** : Equipped with 16 MB external Flash and 8 MB PSRAM for complex IoT and edge AI applications.
- **Wi-Fi & Bluetooth® Connectivity** : Supports 2.4 GHz Wi-Fi (IEEE 802.11 b/g/n) and Bluetooth® 5.0 Low Energy.
- **Advanced LoRa® Transceiver & Global LoRaWAN® Support** : Based on the Semtech SX1262 chip for long-range, low-power wireless communication. Compatible with multiple LoRaWAN® frequency plans including EU868, US915, AU915, AS923, KR920, and IN865 for worldwide deployment.
- **Rich Peripheral Interfaces** : Supports UART, I2C, SPI, USB, ADC, PWM, I2S, SD/MMC, CAN (TWAI®), and GPIOs.
- **High RF Performance** :
- Sensitivity: Up to -137 dBm @ SF12, BW125 kHz
- RF Output Power: Up to 20.9 dBm @ 868/915 MHz
- **High Stability** : Integrated TCXO ensures stable frequency reference and reliable RF communication.
- **Ultra-low Power Consumption** : As low as 9.3 μA sleep current.
- **RF Shielding Design** : Integrated shielding cover for enhanced signal integrity and EMI protection.
- **Flexible Antenna Options** : Available with or without an onboard IPEX connector, providing versatile antenna integration choices.
- **Compact SMT Form Factor** : 21.6 × 16.5 × 3.3 mm, 38-pin SMT package for easy integration into space-constrained designs.
- **Easy to Use** : Comes with built-in command firmware and supports custom product development using an SDK.
- **Cost-effective Solution** : Optimized hardware design delivers strong performance at a competitive cost.
- **Designed for IoT Applications** : Ideal for smart agriculture, industrial IoT, environmental monitoring, asset tracking, and edge AI deployments.
- **FCC, CE, Telec Certified**
note
If you have a large quantity request or need any further information on Seeed IIoT products and services, please don't hesitate to contact us directly at [order@seeed.cc](mailto:order@seeed.cc) or click [here](https://solution.seeedstudio.com/contact-us/) . We'll be happy to offer you our best discount price. We are excited to see you can scale up and expand your business with our products and services.

## Applications

Ideal for LoRaWAN® sensor nodes and a wide range of wireless communication applications, including environmental monitoring, smart agriculture, industrial IoT, asset tracking, smart cities, and remote sensing systems.

![applications](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/application.jpg)

- **Smart Agriculture** : Enables remote monitoring of environmental conditions, supporting precision farming and smart irrigation.
- **Industrial Automation** : Supports industrial monitoring and mesh networking, improving system efficiency and reliability in complex environments.
- **IoT Data Logging & Remote Monitoring** : Ideal for low-power data loggers and distributed sensing applications, ensuring stable long-range communication.
- **Healthcare & Wearables** : Powers health monitoring and wearable devices with reliable data transmission and low-power operation.
- **AIoT & Human-Machine Interface** : Supports edge AI applications such as voice and image recognition, as well as touch and interactive control systems.

## Hardware Pinout

![appearance](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/pinout.jpg)
![appearance](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/pinout-2.jpg)

| Pin No. | Name | Type | Description |
| --- | --- | --- | --- | 
| 1 | ESP_GPIO47 | I/O/T | MCU GPIO | 
| 2 | ESP_GPIO38 | I/O/T | MCU GPIO | 
| 3 | ESP_GPIO39 | I/O/T | MCU GPIO | 
| 4 | ESP_GPIO40 | I/O/T | MCU GPIO | 
| 5 | ESP_GPIO41 | I/O/T | MCU GPIO | 
| 6 | ESP_GPIO42 | I/O/T | MCU GPIO | 
| 7 | ESP_GPIO43 / UART0_TX | I/O/T | MCU GPIO; UART0_TX from MCU | 
| 8 | ESP_GPIO44 / UART0_RX | I/O/T | MCU GPIO; UART0_RX from MCU | 
| 9 | ESP_GPIO45 | I/O/T | MCU GPIO | 
| 10 | ESP_GPIO46 | I/O/T | MCU GPIO | 
| 11 | ESP_GPIO3 | I/O/T | MCU GPIO | 
| 12 | ESP_GPIO2 | I/O/T | MCU GPIO | 
| 13 | ESP_GPIO1 | I/O/T | MCU GPIO | 
| 14 | ESP_BOOT | I/O/T | MCU GPIO0,
Boot Mode: Pull low to enter Download Boot Mode. | 
| 15 | ESP_RST | I | CHIP_PU:
High Level: Chip Enabled;
Low Level: Chip Disabled; Ensure that the CHIP_PU pin is not left floating. | 
| 16 | GND | - | Ground | 
| 17 | GND | - | Ground | 
| 18 | WIFI / BT_ANT / NC | I/O | WiFi and BT RF Input and Output from MCU | 
| 19 | GND | - | Ground | 
| 20 | GND | - | Ground | 
| 21 | VCC 3V3 | - | Supply voltage for the module | 
| 22 | GND | - | Ground | 
| 23 | GND | - | Ground | 
| 24 | ESP_GPIO11 | I/O/T | MCU GPIO | 
| 25 | ESP_GPIO10 | I/O/T | MCU GPIO | 
| 26 | ESP_GPIO12 | I/O/T | MCU GPIO | 
| 27 | ESP_GPIO13 | I/O/T | MCU GPIO | 
| 28 | ESP_GPIO14 | I/O/T | MCU GPIO | 
| 29 | ESP_GPIO15 | I/O/T | MCU GPIO | 
| 30 | ESP_GPIO16 | I/O/T | MCU GPIO | 
| 31 | ESP_GPIO17 | I/O/T | MCU GPIO | 
| 32 | ESP_GPIO18 | I/O/T | MCU GPIO | 
| 33 | ESP_GPIO19 / USB_DM | I/O/T | MCU GPIO; USB_D- from MCU | 
| 34 | ESP_GPIO20 / USB_DP | I/O/T | MCU GPIO; USB_D+ from MCU | 
| 35 | ESP_GPIO48 | I/O/T | MCU GPIO | 
| 36 | GND | - | Ground | 
| 37 | LORA_ANT / NC | - | LoRa RF Input and Output | 
| 38 | GND | - | Ground | 
| 39 | GND | - | Ground | 

## Specifications

| Items | Parameter | Specifications | Specifications | Specifications | Specifications | Unit | | |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | 
| Core | MCU | Xtensa® 32-bit LX7 (dual core, up to 240MHz) | Xtensa® 32-bit LX7 (dual core, up to 240MHz) | Xtensa® 32-bit LX7 (dual core, up to 240MHz) | Xtensa® 32-bit LX7 (dual core, up to 240MHz) | | | | 
| Core | Storage | 16MB Flash, 512KB SRAM | 16MB Flash, 512KB SRAM | 16MB Flash, 512KB SRAM | 16MB Flash, 512KB SRAM | | | | 
| Structure | Size | 21.6*(L) * 16.5(W) * 3.3(H) | 21.6*(L) * 16.5(W) * 3.3(H) | 21.6*(L) * 16.5(W) * 3.3(H) | 21.6*(L) * 16.5(W) * 3.3(H) | mm | | | 
| Structure | Package | 38 pins, SMT | 38 pins, SMT | 38 pins, SMT | 38 pins, SMT | | | | 
| Electrical Characteristics | Power Supply | 3.0 - 3.6V (3.3V typical) | 3.0 - 3.6V (3.3V typical) | 3.0 - 3.6V (3.3V typical) | 3.0 - 3.6V (3.3V typical) | V | | | 
| Electrical Characteristics | Minimum Sleep Current | 9.3uA | 9.3uA | 9.3uA | 9.3uA | uA | | | 
| Electrical Characteristics | Minimum Standby Current | 1.43 | 1.43 | 1.43 | 1.43 | mA | | | 
| Electrical Characteristics | Operation Current (Transmitter+MCU) | 113mA @16dBm in 868MHz type | 113mA @16dBm in 868MHz type | 113mA @16dBm in 868MHz type | 113mA @16dBm in 868MHz type | mA | | | 
| Electrical Characteristics | Operation Current (Transmitter+MCU) | 125mA @22dBm in 868MHz type | 125mA @22dBm in 868MHz type | 125mA @22dBm in 868MHz type | 125mA @22dBm in 868MHz type | mA | | | 
| Electrical Characteristics | Operation Current (Transmitter+MCU) | 127mA @22dBm in 915MHz type | 127mA @22dBm in 915MHz type | 127mA @22dBm in 915MHz type | 127mA @22dBm in 915MHz type | mA | | | 
| Electrical Characteristics | Operation Current (Receiver+MCU) | 5.5mA @BW125kHz, 868MHz type | 5.5mA @BW125kHz, 868MHz type | 5.5mA @BW125kHz, 868MHz type | 5.5mA @BW125kHz, 868MHz type | mA | | | 
| Electrical Characteristics | Operation Current (Receiver+MCU) | 5.7mA @BW125kHz, 915MHz type | 5.7mA @BW125kHz, 915MHz type | 5.7mA @BW125kHz, 915MHz type | 5.7mA @BW125kHz, 915MHz type | mA | | | 
| Electrical Characteristics | Output power | 20.9dBm max @868MHz LoRa | 20.9dBm max @868MHz LoRa | 20.9dBm max @868MHz LoRa | 20.9dBm max @868MHz LoRa | dBm | | | 
| Electrical Characteristics | Output power | 20.74dBm max @915MHz LoRa | 20.74dBm max @915MHz LoRa | 20.74dBm max @915MHz LoRa | 20.74dBm max @915MHz LoRa | dBm | | | 
| Electrical Characteristics | Sensitivity | @SF12, BW125kHz | @SF12, BW125kHz | @SF12, BW125kHz | @SF12, BW125kHz | dBm | | | 
| Electrical Characteristics | Sensitivity | Fr(MHz) | type | type | | dBm | max | max | 
| Electrical Characteristics | Sensitivity | 868 | -137 | -137 | | dBm | -137 | -137 | 
| Electrical Characteristics | Sensitivity | 915 | -136.5 | -136.5 | | dBm | -136.9 | -136.9 | 
| Electrical Characteristics | Harmonics (LoRa) | < -41dBm (2nd Harmonic) | < -41dBm (2nd Harmonic) | < -41dBm (2nd Harmonic) | < -41dBm (2nd Harmonic) | dBm | | | 
| Electrical Characteristics | Harmonics (LoRa) | < -49dBm (3rd Harmonic) | < -49dBm (3rd Harmonic) | < -49dBm (3rd Harmonic) | < -49dBm (3rd Harmonic) | dBm | | | 
| Interface | RFIO | 2 RF ports (Wi-Fi/BT & LoRa) | 2 RF ports (Wi-Fi/BT & LoRa) | 2 RF ports (Wi-Fi/BT & LoRa) | 2 RF ports (Wi-Fi/BT & LoRa) | | | | 
| Interface | UART | 3 groups of UART | 3 groups of UART | 3 groups of UART | 3 groups of UART | | | | 
| Interface | I2C | 2 groups of I2C | 2 groups of I2C | 2 groups of I2C | 2 groups of I2C | | | | 
| Interface | ADC | 2 groups of ADC, 12-bit | 2 groups of ADC, 12-bit | 2 groups of ADC, 12-bit | 2 groups of ADC, 12-bit | | | | 
| Interface | SPI | 1 group of SPI | 1 group of SPI | 1 group of SPI | 1 group of SPI | | | | 
| Interface | USB | 1 group of USB | 1 group of USB | 1 group of USB | 1 group of USB | | | | 
| Interface | NRST | 1 Manual reset pin input | 1 Manual reset pin input | 1 Manual reset pin input | 1 Manual reset pin input | | | | 
| Interface | BOOT | 1 Manual Boot pin input | 1 Manual Boot pin input | 1 Manual Boot pin input | 1 Manual Boot pin input | | | | 
| Temperature | Working Temperature | -40 ~ 85 | -40 ~ 85 | -40 ~ 85 | -40 ~ 85 | ℃ | | | 

## Power Consumption

| Mode | Mode | Power Type | Items | Data (avg) | Note |
| --- | --- | --- | --- | --- | --- | 
| WiFi | WiFi Tx | 3.3V | 802.11B | 349mA | WiFi antenna connected, LoRa sleeping | 
| WiFi | WiFi Tx | 3.3V | 802.11G | 333mA | WiFi antenna connected, LoRa sleeping | 
| WiFi | WiFi Tx | 3.3V | 802.11N | 310mA | WiFi antenna connected, LoRa sleeping | 
| WiFi | WiFi Rx | 3.3V | 802.11B | 104mA | WiFi antenna connected, LoRa sleeping | 
| WiFi | WiFi Rx | 3.3V | 802.11G | 103mA | WiFi antenna connected, LoRa sleeping | 
| WiFi | WiFi Rx | 3.3V | 802.11N | 103mA | WiFi antenna connected, LoRa sleeping | 
| LoRa | LoRa Tx | 3.3V | 868MHz, 16dBm | 113mA | LoRa antenna connected, WiFi & BLE off | 
| LoRa | LoRa Tx | 3.3V | 868MHz, 22dBm | 125mA | LoRa antenna connected, WiFi & BLE off | 
| LoRa | LoRa Tx | 3.3V | 915MHz, 22dBm | 127mA | LoRa antenna connected, WiFi & BLE off | 
| LoRa | LoRa Tx | 3.0V | 868MHz, 16dBm | 109mA | LoRa antenna connected, WiFi & BLE off | 
| LoRa | LoRa Tx | 3.0V | 868MHz, 22dBm | 124mA | LoRa antenna connected, WiFi & BLE off | 
| LoRa | LoRa Tx | 3.0V | 915MHz, 22dBm | 123mA | LoRa antenna connected, WiFi & BLE off | 
| LoRa | LoRa Rx | 3.3V | 868MHz | 5.5mA | LoRa antenna connected, WiFi & BLE off | 
| LoRa | LoRa Rx | 3.3V | 915MHz | 5.7mA | LoRa antenna connected, WiFi & BLE off | 
| WiFi + LoRa | WiFi + LoRa | 3.3V | WiFi 11B AP + LoRa 915MHz, 22dBm | 201mA | | 
| WiFi + LoRa | WiFi + LoRa | 3.3V | WiFi 11B AP + LoRa 868MHz, 16dBm | 189mA | | 
| BLE + LoRa | BLE + LoRa | 3.3V | BLE advertising + LoRa 915MHz, 22dBm | 158mA | | 
| BLE + LoRa | BLE + LoRa | 3.3V | BLE advertising + LoRa 868MHz, 16dBm | 146mA | | 
| Deep Sleep | Deep Sleep | 3.3V | ESP32-S3 Deep Sleep, LoRa Sleep | 9.3uA | ESP32-S3: all peripherals off, deep sleep | 
| Standby / Light Sleep | Standby / Light Sleep | 3.3V | ESP32-S3 Light Sleep, LoRa Standby | 1.43mA | ESP32-S3: light sleep, RTC/ULP active, wake on any event; LoRa standby (STDB_RC), internal RC wake; all RF off | 

note
The above power consumption data was measured under laboratory test conditions and is provided for reference only. Actual power consumption may vary depending on firmware, peripherals, operating mode, environmental conditions, and hardware configuration.

## Typical RF performance test

![appearance](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/RF1.jpg)
![appearance](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/RF2.jpg)
![appearance](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/RF3.jpg)

## Package information

![appearance](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/appearance.jpg)
![layout](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/layout.jpg)

## Reference design based on Wio-S3 module

This reference design facilitates the quick development of dual-mode LoRaWAN® and Wi-Fi/Bluetooth nodes, allowing users to achieve rapid configuration simply by sending commands over the dedicated UART interface. The system integrates dual 5V USB-C inputs with 1A fuse protection, a PMOS-controlled 3.3V BUCK regulator, and dedicated U.FL RF ports, while routing essential RST, BOOT (GPIO0), and peripheral GPIO pins for easy programming and sensor expansion.

![layout](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/reference-design-of-s3.png)

## Sources

### Datasheet

- [Wio-S3 Datasheet & Specifications](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/Wio-S3_Module_Datasheet_V1.0.pdf)
- [Espressif ESP32-S3R8 Datasheet](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/Espressif_ESP32-S3R8_Datasheet.pdf)
- [Semtech SX1262 Datasheet](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/SX1261_2%20V2-2.pdf)

### Library

- [Wio-S3 kicad library](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/Wio-S3_Wireless_Module_Kicad_File.zip)
- [Wio-S3 STEP 3D File](https://files.seeedstudio.com/wiki/SenseCAP/Wio-S3/Wio-S3%20Wireless%20Module_3D.step)

### Relevant SDK

- [ESP-IDF User Guide for ESP32-S3, SDK](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/index.html)
- [ESP32-S3 Test Tools and Guidelines](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32s3/index.html)

## Tech Support & Product Discussion

Please submit any technical issue into our [forum](http://forum.seeedstudio.com/) .

Thank you for choosing our products! We are here to provide you with different support to ensure that your experience with our products is as smooth as possible. We offer several communication channels to cater to different preferences and needs.