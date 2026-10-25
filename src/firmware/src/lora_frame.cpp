#include "lora_frame.hpp"
#include <cstring>

namespace xlora::frame {

std::uint16_t crc16_ccitt_false_run(std::uint16_t crc, const std::uint8_t* data,
                                    std::size_t len) {
    for (std::size_t i = 0; i < len; ++i) {
        crc = static_cast<std::uint16_t>(crc ^ static_cast<std::uint16_t>(data[i]) << 8);
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x8000)
                crc = static_cast<std::uint16_t>((crc << 1) ^ 0x1021);
            else
                crc = static_cast<std::uint16_t>(crc << 1);
        }
    }
    return crc;
}

std::uint16_t crc16_ccitt_false(const std::uint8_t* data, std::size_t len) {
    return crc16_ccitt_false_run(0xFFFF, data, len);
}

namespace {
void put16(std::uint8_t* p, std::uint16_t v) {
    p[0] = static_cast<std::uint8_t>(v & 0xFF);
    p[1] = static_cast<std::uint8_t>((v >> 8) & 0xFF);
}
void put32(std::uint8_t* p, std::uint32_t v) {
    p[0] = static_cast<std::uint8_t>(v & 0xFF);
    p[1] = static_cast<std::uint8_t>((v >> 8) & 0xFF);
    p[2] = static_cast<std::uint8_t>((v >> 16) & 0xFF);
    p[3] = static_cast<std::uint8_t>((v >> 24) & 0xFF);
}
std::uint16_t get16(const std::uint8_t* p) {
    return static_cast<std::uint16_t>(static_cast<std::uint16_t>(p[0]) |
                                      (static_cast<std::uint16_t>(p[1]) << 8));
}
std::uint32_t get32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}
std::int32_t get32s(const std::uint8_t* p) { return static_cast<std::int32_t>(get32(p)); }
}  // namespace

std::size_t encode(std::uint8_t* out, const Frame& f) {
    std::uint8_t b[FRAME_LEN];
    b[0] = MAGIC_0;
    b[1] = MAGIC_1;
    put16(b + 2, f.seq);
    b[4] = static_cast<std::uint8_t>(f.type);
    b[5] = f.rsvd;
    // Payload (bytes 8..25).
    put32(b + 8, f.s.ts);
    put16(b + 12, f.s.rt);
    put32(b + 14, static_cast<std::uint32_t>(f.s.x));
    put32(b + 18, static_cast<std::uint32_t>(f.s.y));
    put32(b + 22, static_cast<std::uint32_t>(f.s.z));
    // CRC over bytes 0..5 then 8..25 (everything except the crc field).
    std::uint16_t crc = crc16_ccitt_false_run(0xFFFF, b, 6);
    crc = crc16_ccitt_false_run(crc, b + 8, FRAME_LEN - 8);
    put16(b + 6, crc);
    std::memcpy(out, b, FRAME_LEN);
    return FRAME_LEN;
}

bool decode(const std::uint8_t* in, std::size_t len, Frame& out) {
    if (len < FRAME_LEN) return false;
    if (in[0] != MAGIC_0 || in[1] != MAGIC_1) return false;
    std::uint16_t stored = get16(in + 6);
    std::uint16_t calc = crc16_ccitt_false_run(0xFFFF, in, 6);
    calc = crc16_ccitt_false_run(calc, in + 8, FRAME_LEN - 8);
    if (stored != calc) return false;
    out.seq = get16(in + 2);
    out.type = static_cast<FrameType>(in[4]);
    out.rsvd = in[5];
    out.s.ts = get32(in + 8);
    out.s.rt = get16(in + 12);
    out.s.x = get32s(in + 14);
    out.s.y = get32s(in + 18);
    out.s.z = get32s(in + 22);
    return true;
}

}  // namespace xlora::frame
