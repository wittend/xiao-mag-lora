// XIAO / Wio-S3 P2P LoRa node firmware (device entry point).
//
// Operating model (see docs/hardware_strategy.md 3a, docs/calculations.md C):
//   - Sample a reading at ~1 Hz into a local backlog (SampleStore).
//   - Discipline the node clock from the L76K GNSS ($GPRMC -> UTC epoch); the
//     clock is what disciplines the 1/s cadence and the `ts` field.
//   - Flush the backlog in bursts over P2P LoRa (no line-rate retry); sequence
//     gaps at the receiver reveal loss.
//   - Decode inbound frames and forward them to the client.
//
// Only built for the ESP32-S3 (ARDUINO) environment; the host/native build of
// this project compiles the portable modules + unit test instead.
#if defined(ARDUINO)

#include <Arduino.h>

#include "gnss_nmea.hpp"
#include "link.hpp"
#include "lora_frame.hpp"
#include "radio.hpp"
#include "radio_stub.hpp"
#include "sample_store.hpp"

using namespace xlora;

namespace {
constexpr std::uint32_t kFlushIntervalMs = 5000;   // burst cadence (Q10)
constexpr int           kFlushBatchMax   = 10;      // max records per burst
constexpr int           kDefaultSf       = 9;       // SF9 default (213 ms @ 26 B)
constexpr std::uint32_t kDefaultFreqHz   = 915900000;  // TODO(Q3): a US915 uplink channel
constexpr std::size_t   kStoreCapacity   = 3600;    // ~1 h of 1 Hz backlog

radio::Radio* g_radio = nullptr;
SampleStore   g_store{kStoreCapacity};
link::TxSeq   g_txseq;
link::RxGaps  g_rxgaps;
link::Stats   g_stats;

std::uint32_t g_epoch = 0;    // GNSS-disciplined clock (seconds); 0 = undisciplined
char          g_nmea_buf[160];
int           g_nmea_len = 0;

// TODO(sensor): replace with the real sensor read. Returns a placeholder.
frame::Sample read_sample(std::uint32_t now_s) {
    frame::Sample s;
    s.ts = now_s;
    s.rt = 0;
    s.x = 0;
    s.y = 0;
    s.z = 0;
    return s;
}

void gnss_rx_byte(char c) {
    if (c == '\n') {
        if (g_nmea_len > 0) {
            g_nmea_buf[g_nmea_len] = '\0';
            const gnss::GpsFix fx = gnss::parse_rmc(g_nmea_buf);
            if (fx.valid) g_epoch = fx.epoch;   // clock discipline
            g_nmea_len = 0;
        }
    } else if (g_nmea_len < (int)sizeof(g_nmea_buf) - 1) {
        g_nmea_buf[g_nmea_len++] = c;
    }
}
}  // namespace

void setup() {
    Serial.begin(115200);
    // TODO(Q4): begin the GNSS (L76K) UART here at its baud (commonly 9600/115200).
    delay(1000);

    // TODO(Q3/Q4): replace StubRadio with the real SX1262 driver (RadioLib),
    // configured for US915 LoRa: frequency (US915 uplink channel), SF9,
    // 125 kHz BW, a LoRa sync word, RX-invert-on, CRC on.
    g_radio = new radio::StubRadio();
    g_radio->begin();
    g_radio->set_spreading_factor(kDefaultSf);
    g_radio->set_frequency_hz(kDefaultFreqHz);

    Serial.println("xiao-mag-lora node up");
}

void loop() {
    const std::uint32_t now_ms = millis();

    // 1) Sample at ~1 Hz into the backlog.
    const std::uint32_t now_s = (g_epoch != 0) ? g_epoch : now_ms / 1000u;
    static std::uint32_t last_sample_s = 0;
    if (now_s != last_sample_s) {
        last_sample_s = now_s;
        g_store.push(read_sample(now_s));
    }

    // 2) Flush the backlog in bursts (delay-tolerant delivery).
    static std::uint32_t last_flush_ms = 0;
    if (g_store.size() >= 1 && (now_ms - last_flush_ms) >= kFlushIntervalMs) {
        last_flush_ms = now_ms;
        for (int i = 0; i < kFlushBatchMax && g_store.size() > 0; ++i) {
            frame::Sample s;
            if (!g_store.pop(s)) break;
            frame::Frame f;
            f.type = frame::FrameType::Data;
            f.seq = g_txseq.alloc();
            f.s = s;
            std::uint8_t buf[frame::FRAME_LEN];
            const std::size_t len = frame::encode(buf, f);
            if (g_radio->send(buf, len)) ++g_stats.tx;
        }
    }

    // 3) Receive inbound frames, decode, forward to the client.
    std::uint8_t rbuf[256];
    std::size_t rlen = 0;
    if (g_radio->poll(rbuf, sizeof(rbuf), rlen)) {
        frame::Frame f;
        if (frame::decode(rbuf, rlen, f)) {
            g_rxgaps.observe(f.seq);
            ++g_stats.rx;
            // TODO(client): forward to the host (USB CDC / Wi-Fi / socket).
            Serial.printf("rx seq=%u type=%u ts=%lu\n", f.seq, (unsigned)f.type,
                          (unsigned long)f.s.ts);
        } else {
            ++g_stats.crc_err;
        }
    }

    // 4) Clock discipline: pull GNSS bytes from the L76K UART.
    //    TODO(Q4): once the GNSS UART is up, drain it here.
    //    while (gnss_uart.available()) gnss_rx_byte((char)gnss_uart.read());

    delay(5);
}

#endif  // ARDUINO
