#pragma once
#include "sensor.hpp"

namespace xlora {

// Placeholder sensor for the PoC: emits a smooth, deterministic fake sample so
// the LoRa link (encode -> TX -> RX -> decode) can be exercised without the
// RM3100/MCP9808. Values are sensor-like and fixed-point x1000.
class DummySensor final : public Sensor {
public:
    frame::Sample read(std::uint32_t now_s) override;
};

}  // namespace xlora
