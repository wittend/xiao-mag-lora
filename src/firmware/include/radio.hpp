#pragma once
#include <cstddef>
#include <cstdint>

namespace xlora::radio {

// Thin point-to-point LoRa radio interface.
//   - Device build: an SX126x in LoRa direct (unmodulated/packet) mode.
//   - Native build: a stub (see radio_stub.hpp) for host testing.
//
// This stays hardware-agnostic so the frame/store/link logic can be unit-tested
// on the host without an MCU or radio.
class Radio {
public:
    virtual ~Radio() = default;

    virtual bool begin() = 0;
    virtual void set_spreading_factor(int sf) = 0;   // 7..12
    virtual void set_frequency_hz(std::uint32_t f) = 0;

    // Transmit one frame. Returns true on success.
    virtual bool send(const std::uint8_t* data, std::size_t len) = 0;

    // Non-blocking receive. Returns true and fills out (<= cap bytes) / out_len
    // when a full frame is available; false otherwise.
    virtual bool poll(std::uint8_t* out, std::size_t cap, std::size_t& out_len) = 0;
};

}  // namespace xlora::radio
