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
> **Q3**. RadioLib is disabled in `lib_deps` (firmware uses StubRadio); enable it
> when the real SX1262 driver lands.

## Frame wire format
Source of truth: [`docs/payload_format.md`](../../docs/payload_format.md).
26 bytes, little-endian: `magic(2) seq(2) type(1) rsvd(1) crc(2) | ts(4) rt(2)
x(4) y(4) z(4)`. CRC is CCITT-FALSE (init `0xFFFF`) over the whole frame **except
the crc field** (bytes 0–5 and 8–25).

## TODO — blocked on the datasheets / hardware (see strategy §6)
- **Q3 (radio part + board):** confirm the exact Wio-S3 LoRa radio and the
  PlatformIO `board =` def; wire `RadioLib` `SX1262` for US915 in
  `main.cpp` (replace `StubRadio`).
- **Q4 (GNSS UART):** confirm the L76K UART pins + baud; begin the GNSS port in
  `setup()` and drain it in `loop()` into `gnss_rx_byte()` (the NMEA parser is
  already wired).
- **Q5 (power wiring):** P1-Pro ↔ Wio-S3 power/interface (power-only use of the
  P1-Pro).
- **Sensor:** `read_sample()` is a placeholder — wire the real sensor read.
- **Client forwarding:** the RX path currently logs to `Serial`; forward decoded
  frames to the host (USB CDC / Wi-Fi / socket) per `software_requirements.md`.

## Radio configuration (US915, §15.247 — Q9 closed)
When integrating the real SX1262: LoRa, **SF9 default** (213 ms @ 26 B),
**125 kHz** BW, RX-invert-on, CRC on, frequency on a US915 uplink channel
(923.3–927.5 MHz region). No TX duty cycle applies (verified; see
`assets/fcc/README.md`). Antenna ≤ 2 dBi as certified — higher gain needs a
proportional conducted-power backoff (> 6 dBi, no 902–928 P2P exemption).
