# src/ — software source (per-language subfolders)

The `xiao-mag-lora` software lives here, split by language/target.

| Subfolder | What it is | Language / target |
|-----------|-----------|-------------------|
| `firmware/` | P2P LoRa node firmware (drive the SX126x, sample, store, flush, discipline the clock from GNSS) | C++ / PlatformIO, XIAO/Wio-S3 (ESP32-S3) |
| `client/`   | Linux host-side control/ingest service (decode frames, log, serve the SPA) | TBD (C++ or Python) |
| `spa/`      | Read-only web app (map + latest readings) | web |

Only `firmware/` is scaffolded so far. The rest are placeholders for the
respective workstreams (see `software_requirements.md`).

## Frame wire format
The single source of truth for the radio payload is
`docs/payload_format.md` (26-byte, little-endian, CCITT-FALSE CRC). The
firmware codec is `firmware/src/lora_frame.cpp`, and it is unit-tested on the
host against the documented worked example (byte-for-byte) via
`firmware/test/run_host_test.sh`.
