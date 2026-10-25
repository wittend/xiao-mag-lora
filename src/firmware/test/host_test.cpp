// Host-side unit test for the portable firmware modules (no PlatformIO needed).
// Run with:  test/run_host_test.sh   (or see the g++ command inside it).
//
// Verifies, among other things, that the frame codec reproduces the documented
// worked example byte-for-byte (docs/payload_format.md).
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "gnss_nmea.hpp"
#include "link.hpp"
#include "lora_frame.hpp"
#include "radio_stub.hpp"
#include "sample_store.hpp"

namespace {
int g_fail = 0;

void check(bool cond, const char* msg) {
    if (cond) {
        std::printf("ok:   %s\n", msg);
    } else {
        std::printf("FAIL: %s\n", msg);
        ++g_fail;
    }
}

bool hex_to_bytes(const char* hex, std::vector<std::uint8_t>& out) {
    out.clear();
    while (*hex) {
        if (*hex == ' ') {
            ++hex;
            continue;
        }
        if (!std::isxdigit((unsigned char)*hex) || !std::isxdigit((unsigned char)hex[1]))
            return false;
        char b[3] = {hex[0], hex[1], 0};
        out.push_back(static_cast<std::uint8_t>(std::strtol(b, nullptr, 16)));
        hex += 2;
    }
    return true;
}

void test_frame_codec() {
    using namespace xlora::frame;
    Frame f;
    f.type = FrameType::Data;
    f.seq = 1;
    f.rsvd = 0;
    f.s.ts = 1761488400;  // 2025-10-26T14:20:00Z
    f.s.rt = 23125;       // 23.125
    f.s.x = 12345678;     // 12345.678
    f.s.y = -234500;      // -234.500
    f.s.z = 987001;       // 987.001

    std::uint8_t buf[FRAME_LEN];
    const std::size_t n = encode(buf, f);
    check(n == 26u, "encode returns 26 bytes");

    std::vector<std::uint8_t> expect;
    check(hex_to_bytes(
              "A5 5A 01 00 01 00 F6 4C 10 2E FE 68 55 5A 4E 61 BC 00 "
              "FC 6B FC FF 79 0F 0F 00",
              expect),
          "documented worked-example bytes parsed");
    check(n == expect.size() && std::memcmp(buf, expect.data(), 26) == 0,
          "encoded frame matches the documented wire bytes exactly");

    Frame d;
    check(decode(buf, n, d), "decode accepts a valid frame");
    check(d.seq == 1 && d.type == FrameType::Data && d.rsvd == 0, "header round-trips");
    check(d.s.ts == 1761488400u && d.s.rt == 23125u && d.s.x == 12345678 &&
              d.s.y == -234500 && d.s.z == 987001,
          "payload round-trips (incl. negative y)");

    std::uint8_t bad[26];
    std::memcpy(bad, buf, 26);
    bad[14] ^= 0x01;
    check(!decode(bad, 26, d), "decode rejects CRC mismatch");
    std::memcpy(bad, buf, 26);
    bad[0] = 0x00;
    check(!decode(bad, 26, d), "decode rejects bad magic");
    check(!decode(buf, 10, d), "decode rejects short frame");
}

void test_sample_store() {
    using xlora::SampleStore;
    using xlora::frame::Sample;
    SampleStore st(3);
    auto mk = [](std::uint32_t ts) {
        Sample s;
        s.ts = ts;
        s.rt = 0;
        s.x = 0;
        s.y = 0;
        s.z = 0;
        return s;
    };
    st.push(mk(1));
    st.push(mk(2));
    st.push(mk(3));
    check(st.size() == 3 && st.dropped() == 0, "store holds up to capacity");
    st.push(mk(4));  // overflow -> drop oldest (ts=1)
    check(st.size() == 3 && st.dropped() == 1, "drop-oldest on overflow");

    Sample out;
    check(st.pop(out) && out.ts == 2, "pop returns surviving oldest (ts=2)");
    check(st.pop(out) && out.ts == 3, "next pop (ts=3)");
    check(st.pop(out) && out.ts == 4, "next pop (ts=4)");
    check(!st.pop(out), "pop on empty returns false");
}

void test_link() {
    using xlora::link::RxGaps;
    using xlora::link::TxSeq;
    TxSeq tx(65534);
    check(tx.alloc() == 65534, "seq starts at 65534");
    check(tx.alloc() == 65535, "seq at 65535");
    check(tx.alloc() == 0, "seq wraps to 0");

    RxGaps rx;
    rx.observe(0);
    rx.observe(1);
    rx.observe(4);  // gap of two (missed 2,3)
    check(rx.lost() == 2, "gap of 2 => 2 lost");
    rx.observe(3);  // backward -> ignored
    check(rx.lost() == 2, "backward seq ignored");
    rx.observe(5);  // in-order continuation
    check(rx.lost() == 2, "in-order continuation adds no loss");
}

void test_nmea() {
    using xlora::gnss::parse_rmc;
    const auto fx = parse_rmc("$GPRMC,142303.00,A,3948.37,N,11623.03,E,0.16,38.38,260915,,*0C");
    check(fx.valid, "GPRMC parses (checksum ok, mode A)");
    check(fx.epoch == 1443277383u, "GPRMC epoch == 2015-09-26T14:23:03Z");

    check(!parse_rmc("$GPRMC,142303.00,A,3948.37,N,11623.03,E,0.16,38.38,260915,,*00").valid,
          "GPRMC rejects a bad checksum");
    check(!parse_rmc("not a nmea sentence").valid, "rejects non-NMEA input");
}

void test_radio_stub_end_to_end() {
    using xlora::frame::Frame;
    using xlora::frame::FrameType;
    using xlora::frame::FRAME_LEN;
    using namespace xlora::frame;

    Frame f;
    f.type = FrameType::Data;
    f.seq = 7;
    f.s.ts = 1000;
    f.s.rt = 1;
    f.s.x = 2;
    f.s.y = -3;
    f.s.z = 4;

    xlora::radio::StubRadio r;
    r.begin();
    r.set_spreading_factor(9);
    r.set_frequency_hz(915900000);

    std::uint8_t fb[FRAME_LEN];
    const std::size_t ln = encode(fb, f);
    check(r.send(fb, ln), "stub accepts a TX");
    check(r.tx_count() == 1 && r.last_tx().size() == 26, "stub recorded one 26-byte TX");
    check(r.sf() == 9, "SF stored on the radio");

    r.inject_rx(r.last_tx());  // loop the sent frame back around
    std::uint8_t in[256];
    std::size_t inlen = 0;
    Frame rd;
    check(r.poll(in, sizeof(in), inlen) && inlen == 26u && decode(in, inlen, rd) &&
              rd.seq == 7 && rd.s.y == -3,
          "frame round-trips encode -> send -> poll -> decode");
}
}  // namespace

int main() {
    test_frame_codec();
    test_sample_store();
    test_link();
    test_nmea();
    test_radio_stub_end_to_end();
    if (g_fail == 0) {
        std::printf("\nALL TESTS PASSED\n");
        return 0;
    }
    std::printf("\n%d TEST(S) FAILED\n", g_fail);
    return 1;
}
