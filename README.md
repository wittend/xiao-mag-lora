# xiao-mag-lora

Solar-powered, GNSS-enabled **point-to-point LoRa data link** (not LoRaWAN,
not mesh) built on Seeed Studio XIAO ESP32-S3 (Wio-S3 Wireless LoRa)
hardware, with a Linux host-side control stack and a single-page web app for
testing and monitoring.

> Note: the requirements file references a placeholder folder name
> `xiao-mag-halo`; the actual repository is `xiao-mag-lora`. This is the
> canonical project root.

## Project Workflow (from `requirements/`)

1. Write Requirements — `requirements/xaio-mag-ora_requirements.md`
2. Generate a Hardware Strategy — `docs/hardware_strategy.md`
3. Create an Overall Bill of Materials (BOM) — `docs/bom.md`
4. Set Up Project Repository & Environment — this folder + `software_requirements.md`
5. Develop Hardware & Software — `kicad/`, `src/`
6. Integration, Deployment, and Evaluation
7. Client-side (Linux host) control software + SPA web app

## Repository Layout

| Path       | Purpose                                                      |
| ---------- | ------------------------------------------------------------ |
| `assets/`  | Datasheets, reference PDFs, images, board renderings         |
| `docs/`    | Strategy, BOM, payload spec, calculations appendix, test plans |
| `src/`     | Firmware + client software source (per-language subfolders)  |
| `dist/`    | Build artifacts and packaged releases                        |
| `kicad/`   | KiCad hardware design (carrier/support board)                |
| `requirements/` | Original requirements source of truth                    |

## Status

- [x] Requirements written
- [x] Repository structure initialized
- [x] Hardware strategy draft
- [x] Bill of Materials draft
- [x] Toolkit selection — PlatformIO (confirmed)
- [x] Wire format — binary, 3-decimal fixed-point (26 B frame)
- [x] Regulatory basis — US915 LoRa verified under §15.247, **no TX duty cycle** (Q9 closed; evidence in `assets/fcc/`)
- [x] Firmware scaffold — PlatformIO project; 26 B frame codec + sample store + link + NMEA parser **host-tested** (byte-exact vs the worked example)
- [x] Firmware device build — **compiles green with the real SX1262 driver active** (`pio run -e wio_s3_lora`, RadioLib 7.x; stand-in board `seeed_xiao_esp32s3`)
- [x] Real SX1262 driver — slotted behind the `Radio` interface (`src/firmware/src/radio_sx1262.cpp`), US915 / SF9 / 125 kHz / 17 dBm, enabled by default
- [x] Host-side 26 B → JSON decoder — `src/host/decode.py` (byte-exact self-test vs the worked example)
- [x] PoC bench/flash walkthrough — `docs/poc_walkthrough.md` (2-node link test + control-pin gate)
- [~] Q3/Q4 resolved from `assets/` datasheets — Wio-S3 = ESP32-S3R8 + SX1262 (US915); L76K = UART @ 9600 (D6/D7). Q5 (P1-Pro power wiring) still open (teardown)
- [~] Flash + 2-node bench — blocked only on the 4 SX1262 control pins (CS/DIO1/BUSY/RST); SPI confirmed
- [ ] Client control software
- [ ] SPA web app
- [ ] Integration / deployment
