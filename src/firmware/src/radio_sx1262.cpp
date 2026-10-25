// Sx1262Radio implementation — RadioLib 7.x, SX1262 in LoRa packet mode.
// Device build only (pulls in <RadioLib.h> and the Arduino/ESP32 headers).
//
// Both PoC nodes run this driver. A node is simultaneously a sender (it pushes
// its local sample store as 26-byte frames in bursts) and a receiver (it
// reads inbound frames from the far node and forwards them to the host). The
// two directions share one radio via: continuous interrupt-driven RX + blocking
// TX, re-armed after every transmission (see radio_sx1262.hpp for the model).

#include "radio_sx1262.hpp"

#include <Arduino.h>   // SPI, ICACHE_RAM_ATTR
#include <SPI.h>
#include <RadioLib.h>  // SX1262, Module, ConfigLoRa_t (global namespace)

namespace {
// Set by the SX1262 DIO1 interrupt when a full LoRa packet is received; read
// by poll() in the main loop. File-static: the link PoC has a single radio.
volatile bool s_sx1262_rx_ready = false;
}  // namespace

// DIO1 packet-received handler. Runs in interrupt context, so it must be
// IRAM-placed and do the bare minimum: just flag that a packet is waiting.
static void ICACHE_RAM_ATTR onSx1262Rx() { s_sx1262_rx_ready = true; }

namespace xlora { namespace radio {

Sx1262Radio::Sx1262Radio() = default;

bool Sx1262Radio::begin() {
  // 1) Bind the shared SPI bus to the XIAO LoRa-header pins (SCK=D11,
  //    MISO=D13, MOSI=D12). CS is driven by the Module, so the bus SS is
  //    left unmanaged (-1).
  SPI.begin(pins::kSx1262Sck, pins::kSx1262Miso, pins::kSx1262Mosi, -1);

  // 2) Module = (CS, IRQ=DIO1, RST, BUSY) on the already-configured SPI. The
  //    SPIClass& constructor does NOT re-init SPI, so our pin mapping sticks.
  //    In RadioLib 7.x the Module's 4th "gpio" pin IS the BUSY line on SX126x.
  module_ = new Module(pins::kSx1262Cs, pins::kSx1262Dio1, pins::kSx1262Rst,
                       pins::kSx1262Busy, SPI);

  // 3) The SX1262 radio on top of the module.
  radio_ = new SX1262(module_);

  // 4) Configure the LoRa modem: US915, 125 kHz, SF9, 4/5, explicit header,
  //    hardware CRC on, private sync word.
  ConfigLoRa_t cfg;
  cfg.frequency       = kFreqMhz;
  cfg.bandwidth       = kBwKhz;
  cfg.spreadingFactor = kSpreadingFct;
  cfg.codingRate      = kCodingRate;
  cfg.syncWord        = RADIOLIB_LORA_SYNC_WORD_PRIVATE;  // 0x12 (private)
  cfg.power           = kTxPowerDbm;
  cfg.preambleLength  = kPreamble;

  // The Wio-SX1262 uses a TCXO (1.6 V on DIO3). This is the RadioLib default;
  // set it explicitly so the intent is visible (a plain-XTAL module would be 0).
  radio_->tcxoVoltage = 1.6f;

  // Explicit LoRa header is the RadioLib default: the 26-byte payload length
  // rides in the header so the receiver knows how much to read. Our own CCITT
  // CRC is kept as the authoritative frame-integrity check.
  last_error_ = radio_->begin(cfg);
  if (last_error_ != RADIOLIB_ERR_NONE) {
    return false;  // radio did not initialise -> wrong pins/board, check last_error()
  }

  // 5) Continuous, interrupt-driven RX: arm the radio to listen now.
  radio_->setPacketReceivedAction(&onSx1262Rx);
  radio_->startReceive();
  return true;
}

void Sx1262Radio::set_spreading_factor(int sf) {
  if (radio_) radio_->setSpreadingFactor(static_cast<uint8_t>(sf));
}

void Sx1262Radio::set_frequency_hz(uint32_t hz) {
  if (radio_) radio_->setFrequency(static_cast<float>(hz) / 1.0e6f);
}

bool Sx1262Radio::send(const uint8_t* data, size_t len) {
  if (!radio_) return false;
  // Blocking transmit: waits for the whole packet to leave the PA (~airtime).
  last_error_ = radio_->transmit(data, len);
  // Radio is in standby after TX -> re-arm continuous RX.
  radio_->startReceive();
  return last_error_ == RADIOLIB_ERR_NONE;
}

bool Sx1262Radio::poll(uint8_t* out, size_t cap, size_t& out_len) {
  out_len = 0;
  if (!radio_ || !out) return false;
  if (!s_sx1262_rx_ready) {
    return false;  // no completed packet yet
  }
  s_sx1262_rx_ready = false;
  size_t plen = radio_->getPacketLength(false);  // length of the received packet
  if (plen == 0) {
    radio_->startReceive();
    return false;
  }
  if (plen > cap) plen = cap;
  last_error_ = radio_->readData(out, plen);
  radio_->startReceive();  // re-arm for the next packet
  if (last_error_ == RADIOLIB_ERR_NONE) {
    out_len = plen;
    return true;
  }
  return false;
}

}}  // namespace xlora::radio
