#include "sensor_dummy.hpp"
#include <cmath>

namespace xlora {

frame::Sample DummySensor::read(std::uint32_t now_s) {
    static std::uint32_t n = 0;
    const double t = static_cast<double>(++n) / 10.0;   // 0.1 s phase
    frame::Sample s;
    s.ts = now_s;
    // temperature ~23.0–23.5 C (x1000), mag field drifting (x1000 "Gauss-ish")
    s.rt = static_cast<std::uint16_t>(23000 + 500 * (0.5 + 0.5 * std::sin(t / 3.0)));
    s.x  = static_cast<std::int32_t>(20000 + 10000 * std::sin(t));
    s.y  = static_cast<std::int32_t>(45000 + 8000 * std::sin(t * 0.7 + 1.3));
    s.z  = static_cast<std::int32_t>(-4500 + 5000 * std::cos(t * 0.4));
    return s;
}

}  // namespace xlora
