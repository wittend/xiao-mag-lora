#pragma once
#include <cstdint>

// XIAO ESP32-S3 + Wio-SX1262 pin map for the xiao-mag-lora PoC.
//
// GPIO numbers are the ESP32-S3 pins backing the XIAO "D" headers. Authoritative
// mapping (from "Pin Multiplexing with Seeed Studio XIAO ESP32-S3 (Sense).md"):
//   D0=1  D1=2  D2=3  D3=4  D4=5(SDA)  D5=6(SCL)  D6=43(TX)  D7=44(RX)
//   D8=7(SCK)  D9=8(MISO)  D10=9(MOSI)  D11=42  D12=41
//
// SX1262 (Wio-SX1262-for-XIAO, SKU 113010003) wiring to the XIAO:
//   * SPI  - the Wio-SX1262 uses the XIAO's built-in SPI header (datasheet:
//            "1x SPI (connect to XIAO)"), so SCK/MISO/MOSI = D8/D9/D10. CONFIRMED.
//   * CONTROL (CS / DIO1 / BUSY / RST) - these ride the remaining XIAO pins and
//            their exact mapping lives only in the schematic image (PDF p.6),
//            which is not extractable here. The values below are a SENSIBLE
//            best-guess on free GPIOs (D0/D1/D11/D12), chosen to avoid UART
//            (D6/D7), I2C (D4/D5) and the boot/JTAG strapping pins. RadioLib's
//            begin() returns a non-zero error if any of the four is wrong, so a
//            bad map is immediately visible on the bench (see the boot diagnostic
//            in src/main.cpp) and is corrected here, in one place.
namespace xlora::pins {

// --- SX1262 (Wio-SX1262 breakout) ----------------------------------------
constexpr int kSx1262Sck  = 7;    // XIAO D8   SPI clock         [CONFIRMED]
constexpr int kSx1262Mosi = 9;    // XIAO D10  SPI MOSI          [CONFIRMED]
constexpr int kSx1262Miso = 8;    // XIAO D9   SPI MISO          [CONFIRMED]
constexpr int kSx1262Cs   = 42;   // XIAO D11  chip select (NSS) [BENCH-VERIFY]
constexpr int kSx1262Dio1 = 41;   // XIAO D12  DIO1 / IRQ        [BENCH-VERIFY]
constexpr int kSx1262Busy = 2;    // XIAO D1   BUSY              [BENCH-VERIFY]
constexpr int kSx1262Rst  = 1;    // XIAO D0   NRESET (active-low) [BENCH-VERIFY]
// kSx1262Dio2 / kSx1262Dio3 (RF switch / TCXO): not driven in v1 (single IPEX).

// --- L76K GNSS (clock discipline only), UART -----------------------------
constexpr int         kGnssRxPin = 43;   // XIAO D6  node <- GPS NMEA
constexpr int         kGnssTxPin = 44;   // XIAO D7  node -> GPS config
constexpr unsigned long kGnssBaud = 9600;  // L76K default NMEA baud

// --- I2C (RM3100 magnetometer + MCP9808 temperature) ---------------------
constexpr int kI2cSdaPin = 5;    // XIAO D4
constexpr int kI2cSclPin = 6;    // XIAO D5

// --- user I/O ------------------------------------------------------------
constexpr int kUserLedPin = 21;  // XIAO USER_LED (GPIO21)

}  // namespace xlora::pins
