// XIAO / Wio-S3 P2P LoRa node firmware (device entry point).
//
// Operating model (see docs/hardware_strategy.md 3a, docs/calculations.md C):
//   - Sample a reading at ~1 Hz into a local backlog (SampleStore).
//   - Discipline the node clock from the L76K GNSS ($GPRMC -> UTC epoch); the
//     clock disciplines the 1/s cadence and the `ts` field.
//   - Flush the backlog in bursts over P2P LoRa (no line-rate retry); sequence
//     gaps at the receiver reveal loss.
//   - On receive: CRC-check, note gaps, and forward the RAW 26-byte air frame
//     to the host over USB. The host owns the 26-byte -> JSON decode
//     (src/host/decode.py), resynchronizing on the 0xA5 0x5A magic.
//
// Radio backend:
//   - XLORA_USE_SX1262 defined -> real SX1262 (RadioLib), LoRa direct mode.
//   - otherwise                -> in-memory StubRadio (host/native + pre-pin-check).
//
// Only built for the ESP32-S3 (ARDUINO) environment; the native build compiles
// the portable modules + unit test instead.
#if defined(ARDUINO)

#include <Arduino.h>
#include <HardwareSerial.h>

#include "gnss_nmea.hpp"
#include "link.hpp"
#include "lora_frame.hpp"
#include "radio.hpp"
#if defined(XLORA_USE_SX1262)
#include "radio_sx1262.hpp"
#else
#include "radio_stub.hpp"
#endif
#include "sample_store.hpp"
#include "sensor.hpp"
#include "sensor_dummy.hpp"
#include "xiao_pins.hpp"

using namespace xlora;

namespace {
constexpr std::uint32_t kFlushIntervalMs = 5000;   // burst cadence (Q10)
constexpr int           kFlushBatchMax   = 10;     // max records per burst
constexpr int           kDefaultSf       = 9;      // SF9 default (213 ms @ 26 B)
constexpr std::uint32_t kDefaultFreqHz   = 915900000;  // US915 uplink channel (Q3)
constexpr std::size_t   kStoreCapacity   = 3600;   // ~1 h of 1 Hz backlog
constexpr std::uint32_t kHeartbeatMs     = 5000;

HardwareSerial g_gnss(2);     // L76K on UART2 (optional; link runs without it)

radio::Radio* g_radio = nullptr;
SampleStore   g_store{kStoreCapacity};
Sensor*       g_sensor = new DummySensor();
link::TxSeq   g_txseq;
link::RxGaps  g_rxgaps;
link::Stats   g_stats;

std::uint32_t g_epoch = 0;    // GNSS-disciplined clock (seconds); 0 = undisciplined
char          g_nmea_buf[160];
int           g_nmea_len = 0;

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

// Forward a CRC-valid air frame to the host as its raw 26 bytes. The host
// resynchronizes on the 0xA5 0x5A magic (see src/host/decode.py).
void forward_to_host(const std::uint8_t* buf, std::size_t len) {
    Serial.write(buf, len);
}
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n[xiao-mag-lora] boot");

    // L76K GNSS (clock discipline). Optional; non-fatal if absent.
    g_gnss.begin(pins::kGnssBaud, SERIAL_8N1, pins::kGnssRxPin, pins::kGnssTxPin);

#if defined(XLORA_USE_SX1262)
    g_radio = new radio::Sx1262Radio();
    Serial.printf("[radio] SX1262 pins: SCK=%d MOSI=%d MISO=%d CS=%d DIO1=%d BUSY=%d RST=%d\n",
                  pins::kSx1262Sck, pins::kSx1262Mosi, pins::kSx1262Miso,
                  pins::kSx1262Cs, pins::kSx1262Dio1, pins::kSx1262Busy, pins::kSx1262Rst);
    const bool ok = g_radio->begin();
    if (ok) {
        Serial.println("[radio] SX1262 begin: OK");
    } else {
        const int err = static_cast<radio::Sx1262Radio*>(g_radio)->last_error();
        Serial.printf("[radio] SX1262 begin: FAIL (RadioLib error %d)\n", err);
        Serial.println("[radio]   -> SX1262 did not answer. Most likely cause on first boot: the 4");
        Serial.println("[radio]      [BENCH-VERIFY] control pins (CS/DIO1/BUSY/RST) in include/xiao_pins.hpp.");
        Serial.println("[radio]      SPI (D8/D9/D10) is fixed. Check the Wio-SX1262-for-XIAO schematic (PDF p.6),");
        Serial.println("[radio]      correct the pins, rebuild, and reflash.");
    }
#else
    g_radio = new radio::StubRadio();
    g_radio->begin();
    Serial.println("[radio] StubRadio (no radio; XLORA_USE_SX1262 not set)");
#endif
    g_radio->set_spreading_factor(kDefaultSf);
    g_radio->set_frequency_hz(kDefaultFreqHz);

    Serial.printf("[node] sf=%d freq=%.1f MHz store=%zu\n", kDefaultSf,
                  kDefaultFreqHz / 1e6, g_store.capacity());
}

void loop() {
    const std::uint32_t now_ms = millis();

    // 1) Sample at ~1 Hz into the backlog.
    const std::uint32_t now_s = (g_epoch != 0) ? g_epoch : now_ms / 1000u;
    static std::uint32_t last_sample_s = 0;
    if (now_s != last_sample_s) {
        last_sample_s = now_s;
        g_store.push(g_sensor->read(now_s));
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

    // 3) Receive inbound frames, CRC-check, forward raw to the host.
    std::uint8_t rbuf[frame::FRAME_LEN + 8];
    std::size_t rlen = 0;
    if (g_radio->poll(rbuf, sizeof(rbuf), rlen)) {
        frame::Frame f;
        if (frame::decode(rbuf, rlen, f)) {
            g_rxgaps.observe(f.seq);
            ++g_stats.rx;
            forward_to_host(rbuf, frame::FRAME_LEN);
        } else {
            ++g_stats.crc_err;
        }
    }

    // 4) Clock discipline: drain GNSS bytes.
    while (g_gnss.available()) gnss_rx_byte((char)g_gnss.read());

    // 5) Heartbeat (ASCII; deliberately never contains the 0xA5 frame byte, so
    //    it can't corrupt the host's frame resynchronization).
    static std::uint32_t last_hb_ms = 0;
    if (now_ms - last_hb_ms >= kHeartbeatMs) {
        last_hb_ms = now_ms;
        Serial.printf("[hb] tx=%lu rx=%lu lost=%lu crc_err=%lu backlog=%zu clock=%s\n",
                      (unsigned long)g_stats.tx, (unsigned long)g_stats.rx,
                      (unsigned long)g_rxgaps.lost(), (unsigned long)g_stats.crc_err,
                      g_store.size(), g_epoch ? "gnss" : "local");
    }

    delay(2);
}

#endif  // ARDUINO
