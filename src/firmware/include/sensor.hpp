#pragma once
#include <cstdint>

#include "lora_frame.hpp"

namespace xlora {

// Produces one logical sample (the 1 Hz measurement) to be queued and flushed
// over the air. v1 is a placeholder (DummySensor); the real implementation
// reads the RM3100 magnetometer (x/y/z) and MCP9808 temperature (rt) over I2C.
class Sensor {
public:
    virtual ~Sensor() = default;
    // now_s: best epoch-seconds available (GNSS-disciplined, or local fallback).
    virtual frame::Sample read(std::uint32_t now_s) = 0;
};

}  // namespace xlora
