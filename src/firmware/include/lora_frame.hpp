#pragma once
#include <cstddef>
#include <cstdint>

namespace xlora::frame {

// 26-byte, little-endian wire frame. Source of truth: docs/payload_format.md.
constexpr std::size_t FRAME_LEN = 26;
constexpr std::uint8_t MAGIC_0 = 0xA5;
constexpr std::uint8_t MAGIC_1 = 0x5A;

enum class FrameType : std::uint8_t {
    Data      = 0x01,
    Ack       = 0x02,
    Keepalive = 0x03,
    Config    = 0x04,
};

// One logical sample (the 18-byte payload). Fixed-point x1000 on rt/x/y/z.
struct Sample {
    std::uint32_t ts;   // epoch seconds (UTC)
    std::uint16_t rt;   // x1000, 0..65.535
    std::int32_t  x;    // x1000
    std::int32_t  y;    // x1000
    std::int32_t  z;    // x1000
};

// Full decoded frame. For type==Ack, s.ts holds the acked seq (rest ignored).
struct Frame {
    std::uint16_t seq = 0;
    FrameType     type = FrameType::Data;
    std::uint8_t  rsvd = 0;
    Sample        s{};
};

// CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection, no xor-out.
std::uint16_t crc16_ccitt_false(const std::uint8_t* data, std::size_t len);
// Running form so the frame CRC can hash two contiguous ranges (0-5, then 8-25).
std::uint16_t crc16_ccitt_false_run(std::uint16_t crc, const std::uint8_t* data,
                                    std::size_t len);

// Encode f into out (must be >= FRAME_LEN). Returns FRAME_LEN.
std::size_t encode(std::uint8_t* out, const Frame& f);

// Decode in (len bytes) into out. Returns false if len < FRAME_LEN, magic
// mismatch, or CRC mismatch.
bool decode(const std::uint8_t* in, std::size_t len, Frame& out);

}  // namespace xlora::frame
