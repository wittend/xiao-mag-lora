# firmware — XIAO / Wio-S3 P2P LoRa node

PlatformIO firmware that drives the SX126x-class LoRa radio in **direct (P2P)
mode** — no LoRaWAN, no mesh. It samples a reading at ~1 Hz into a local
backlog, disciplines the node clock from the L76K GNSS, and flushes the backlog
in bursts. Point-to-point only; see `docs/hardware_strategy.md` and
`docs/calculations.md` for the operating model and the regulatory basis
(§15.247, Q9 closed).

## Layout

| Path | Purpose |
|------|---------|
| `include/lora_frame.hpp`, `src/lora_frame.cpp` | **26-byte wire-frame codec** (encode/decode) + CCITT-FALSE CRC. Hardware-free. |
| `include/sample_store.hpp`, `src/sample_store.cpp` | Fixed-capacity ring buffer of samples awaiting flush (drop-oldest on overflow). |
| `include/link.hpp`, `src/link.cpp` | Per-direction sequence allocator + sequence-gap (loss) estimator + link stats. |
| `include/gnss_nmea.hpp`, `src/gnss_nmea.cpp` | Parse a `$GPRMC` sentence (checksum-verified) into a UTC epoch — clock discipline only. |
| `include/radio.hpp` | Hardware-agnostic `Radio` interface (begin / SF / freq / send / poll). |
| `include/radio_stub.hpp`, `src/radio_stub.cpp` | In-memory `Radio` for host tests + placeholder until the real driver lands. |
| `include/radio_sx1262.hpp`, `src/radio_sx1262.cpp` | **Real SX1262 `Radio` driver** (RadioLib 7.x): US915 LoRa TX (blocking) + continuous interrupt-driven RX. Device-only. |
| `src/main.cpp` | Device entry (`setup()`/`loop()`): sample → store → flush → RX → client; GNSS clock. Built for ESP32-S3 only. |
| `test/host_test.cpp`, `test/run_host_test.sh` | Host-side unit tests (no PlatformIO required). |

The portable modules (`lora_frame`, `sample_store`, `link`, `gnss_nmea`,
`radio_stub`) are standard C++ with no Arduino/MCU dependency, so they unit-test
on the host and build for the device unchanged.

## Build & test

### Host unit test (no toolchain needed)
```sh
src/firmware/test/run_host_test.sh
```
Compiles the portable modules + `test/host_test.cpp` with `g++` and runs them.
It asserts, among other things, that the encoder reproduces the documented
worked example **byte-for-byte** (26 B, `A5 5A … 79 0F 0F 00`) and that
decode/round-trip, CRC + magic rejection, the ring buffer, sequence wrap/loss,
and the `$GPRMC`→epoch path all behave.

### PlatformIO
```sh
cd src/firmware
pio run   -e native         # host build of the portable modules (sanity/type check)
pio run   -e wio_s3_lora    # device build (ESP32-S3) — verified green (stand-in board, see below)
pio device monitor -e wio_s3_lora
```
> `board =` is a **stand-in** (`seeed_xiao_esp32s3`, the same ESP32-S3 MCU)
> because stock PlatformIO has no `Wio-S3-LoRa` board def — see open question
> **Q3**. The **real SX1262 driver (RadioLib 7.x) is enabled by default**
> (`-DXLORA_USE_SX1262`), so the built firmware is a working P2P PoC node. To
> build without the radio (pure toolchain sanity) comment that flag out in
> `platformio.ini`.

## Frame wire format
Source of truth: [`docs/payload_format.md`](../../docs/payload_format.md).
26 bytes, little-endian: `magic(2) seq(2) type(1) rsvd(1) crc(2) | ts(4) rt(2)
x(4) y(4) z(4)`. CRC is CCITT-FALSE (init `0xFFFF`) over the whole frame **except
the crc field** (bytes 0–5 and 8–25).

## TODO — hardware in hand (PoC on the Kit: XIAO ESP32-S3 + Wio-SX1262)
- **DONE:** real SX1262 driver slotted in (`radio_sx1262.cpp`, RadioLib 7.x),
  US915 / SF9 / 125 kHz / 17 dBm, wired behind the `Radio` interface in `main.cpp`
  (replaces `StubRadio`). Device build green with the radio active.
- **BENCH-VERIFY (PoC gate):** the 4 SX1262 **control pins** (CS/DIO1/BUSY/RST) in
  `include/xiao_pins.hpp` are a best-guess; SPI (D8/D9/D10) is confirmed. On first
  boot the firmware prints the pin map + `begin: OK/FAIL (RadioLib error N)`. If
  FAIL, read the 4 real control pins off the Wio-SX1262-for-XIAO board/schematic
  (PDF p.6) and fix them in `xiao_pins.hpp`, rebuild, reflash.
- **Q4 (GNSS):** L76K confirmed UART @ 9600, D6/D7 — already wired in `main.cpp`.
  Verify on the bench it locks and disciplines the clock.
- **Sensor:** `read_sample()` is a placeholder (dummy sample) — wire the real
  RM3100 + MCP9808 I2C reads once the radio link is proven.
- **Client forwarding (DONE):** the RX path forwards each CRC-valid frame's raw 26
  bytes to `Serial` (USB CDC). The host decoder [`src/host/decode.py`](../host/decode.py)
  resyncs on the `0xA5 0x5A` magic, CRC-checks, and emits one JSON object per frame.
  See [`docs/poc_walkthrough.md`](../../docs/poc_walkthrough.md) to flash + run the 2-node
  link test. Wi-Fi/BLE config mode is a later step (off in normal op for power).

## Host-side decoder (26 B → JSON)
The node forwards raw 26-byte frames over USB. `src/host/decode.py` turns them into
JSON (resyncs on `0xA5 0x5A`, re-validates the CCITT-FALSE CRC, one JSON object per
line; `-v` echoes the node's ASCII heartbeat lines to stderr):

```sh
python3 src/host/decode.py --selftest            # prove it (byte-level, no hardware)
python3 src/host/decode.py -p /dev/ttyACM1 -v    # live USB-serial (needs pyserial)
python3 src/host/decode.py -f capture.bin         # from a saved capture, or via stdin
```

## Radio configuration (US915, §15.247 — Q9 closed)
Set in `src/radio_sx1262.cpp` (`Sx1262Radio::begin()`): LoRa, **915.9 MHz**
(a US915 channel; adjust per site), **SF9 default** (213 ms @ 26 B), **125 kHz**
BW, coding rate 4/5, **17 dBm** out, preamble 8, explicit header (hardware CRC off
— our CCITT CRC is authoritative). SF7 is used for faster backlog flush. No TX
duty cycle applies (verified; see `assets/fcc/README.md`). Antenna ≤ 2 dBi as
certified — higher gain needs a proportional conducted-power backoff (> 6 dBi, no
902–928 P2P exemption).
