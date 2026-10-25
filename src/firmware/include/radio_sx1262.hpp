#pragma once

#include "radio.hpp"     // the Radio interface (also brings in <cstdint>/<cstddef>)
#include "xiao_pins.hpp" // XIAO ESP32-S3 <-> Wio-SX1262 pin map

// Sx1262Radio — the REAL radio driver, slotted behind the portability Radio
// interface, built on RadioLib (https://github.com/jgromes/RadioLib, v7.x).
//
// This is the driver that actually moves our 26-byte frames over the air on
// the Wio-SX1262-for-XIAO. It compiles only for the device build (it includes
// <RadioLib.h>), and main.cpp instantiates it only when XLORA_USE_SX1262 is
// defined (see platformio.ini). The host unit test and the default device
// build use the hardware-free StubRadio, so the tests never need a radio.
//
// PIN MAP (XIAO ESP32-S3, Wio-SX1262 stacked on the LoRa headers) — confirmed
// against assets/Wio-SX1262 for XIAO -113010003.pdf:
//   D8 -> CS (NSS)     D9 -> DIO1 (IRQ)     D10 -> BUSY
//   D11 -> SCK         D12 -> MOSI          D13 -> MISO        D4 -> RST
//
// Note: D9 is also XIAO I2C-SCL and D10 is also XIAO I2C-SDA, but the SX1262
// SPI owns those lines on this board. The magnetometer / temperature sensors
// are NOT used by the link PoC, so there is no pin conflict.
//
// Radio model (both nodes run this):
//   * continuous, interrupt-driven RX: startReceive() arms the SX1262 to
//     listen; when a full LoRa packet completes, DIO1 pulses and our ISR sets
//     a flag; poll() reads the completed packet (readData) and re-arms.
//   * blocking TX: send() calls transmit() (waits ~one frame airtime, ~213 ms
//     at SF9/125 kHz), then re-arms continuous RX.
//   * explicit LoRa header, hardware CRC on, private sync word 0x12, 125 kHz,
//     17 dBm, US915.
// RadioLib types (RadioLib 7.x puts them in the global namespace). We only
// hold pointers to them, so forward declarations keep <RadioLib.h> out of this
// header and out of the host/native build entirely.
class Module;
class SX1262;

namespace xlora { namespace radio {

class Sx1262Radio : public Radio {
public:
  Sx1262Radio();

  // Configures the SPI bus + SX1262 modem. Returns false (and last_error()
  // != 0) if the radio fails to initialise — a good first thing to check on
  // the bench, because it means the pin map or the board is wrong.
  bool begin() override;
  void set_spreading_factor(int sf) override;
  void set_frequency_hz(uint32_t hz) override;
  bool send(const uint8_t* data, size_t len) override;
  bool poll(uint8_t* out, size_t cap, size_t& out_len) override;

  // Last RadioLib status code (RADIOLIB_ERR_NONE == 0) from begin()/send().
  int16_t last_error() const { return last_error_; }

private:
  Module* module_ = nullptr;  // RadioLib: SPI + control pins (BUSY = "gpio")
  SX1262* radio_  = nullptr;  // RadioLib: the SX1262 LoRa radio
  int16_t last_error_ = 0;

  // LoRa modem configuration — US915 defaults (see hardware_strategy.md §3).
  static constexpr float kFreqMhz      = 915.9f;  // mid-band; both nodes agree
  static constexpr float kBwKhz        = 125.0f;
  static constexpr int   kSpreadingFct = 9;       // SF9 default
  static constexpr int   kCodingRate   = 5;       // 4/5
  static constexpr int   kTxPowerDbm   = 17;      // within the 8 dBm/3 kHz PSD cap
  static constexpr int   kPreamble     = 8;
};

}}  // namespace xlora::radio
