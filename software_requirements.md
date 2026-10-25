# Software Requirements

## Firmware (on-node)

- **MCU:** ESP32-S3 (Wio-S3 Wireless LoRa, XIAO form factor)
- **Toolchain (decided):** PlatformIO, native custom firmware (P2P — no
  Meshtastic/LoRaWAN base). ESP-IDF only if lower-level work becomes
  necessary.
- **Firmware: custom point-to-point LoRa data link.** Drives the onboard
  SX1262-class radio in direct mode (no LoRaWAN MAC). Features:
  - Role flag: transmitter (sends the telemetry record) vs receiver/host-side
  - Custom framed datagrams: fixed 8 B header (magic, seq, type, CCITT-CRC
    field) + the telemetry record (`ts`, `rt`, `x`, `y`, `z`) as **compact
    binary, 3-decimal fixed-point — 26 B frame** (decided; spec in
    `docs/payload_format.md`). The node **samples 1/s into local storage**; the
    radio delivers accumulated records **in bursts** (no FCC TX duty cycle
    applies — bounded only by on-air airtime, §C) — reliable *ultimate*
    delivery via sequence numbers + receiver ACK / gap retransmit (no line-rate
    retry)
  - L76K GNSS: **clock maintenance only** (disciplines the remote clock so
    `ts` and the 1 pkt/s cadence stay aligned); no position is transmitted
  - Deep-sleep duty cycle synced to solar charge state
  - (VERIFY) exact pin map for LoRa SPI, GNSS UART, battery sense
- **Radio config:** US915, 2–4 provisioned channels per site (915.9–916.5
  MHz). **SF9 (213 ms) default** — robust at the ≤1 km LoS target with 4.7×
  on-air headroom; SF7 (59 ms) for faster backlog flush, SF8/SF10 per site. SF
  is chosen by link budget at the required on-air rate (no FCC TX duty cycle,
  §C). No LoRaWAN, no mesh.

## Client-side (Linux host)

- **Languages:** any combination of Go / C / C++ / Rust; prioritizing
  security and portability across popular distributions (Debian, Ubuntu,
  Fedora, Arch, RHEL).
- **Responsibilities:**
  - Talk to node(s) over LoRa (a host LoRa radio) and/or WiFi/BT when in range
  - Node provisioning (region/channel/keys), firmware update, diagnostics
  - Telemetry ingest: the record stream at 1 pkt/s during acquisition
    windows, plus node health (battery/charge state, link quality,
    temperature); decode 26 B binary frames to JSON for logging/SPA (spec in
    `docs/payload_format.md`)
  - Store history (SQLite is a reasonable default — decision to confirm)
- **Deliverable shape:** single static binary preferred (Go or Rust) to
  minimize distribution pain.

## SPA Web App (testing & monitoring)

- Served by the client-side service (or standalone build consumed by it).
- Features (v1):
  - Node status board: online/offline, battery %, clock-sync state
  - Live log/telemetry stream (WebSocket)
  - Node control: transmit test message, sleep, config change
  - Link health view: RX count, SNR/SF, packet loss, last RX time (P2P)
    — no mesh topology
- Stack: to be selected with the toolkit decision (plain TS + Vite or
  equivalent; no server-side framework required).

## Testing & Build

- CI: build firmware (PlatformIO) + client (static binary) + SPA; run unit
  tests on every push.
- `dist/` receives versioned artifacts; tags mark releases.
- Bench test plan to be written under `docs/` before integration (step 6).

## Repository & Environments

- Repo root: `xiao-mag-lora` (this directory). Git initialized.
  - **Local dev remote (Gitea):** `gitea@10.0.0.199:dave/xiao-mag-lora.git` (push-to-create done)
  - **Source of truth (GitHub):** `https://github.com/wittend/xiao-mag-lora` (public) — `git@github.com:wittend/xiao-mag-lora.git`
- Environments:
  - Dev: host machine, USB-connected node(s)
  - Field: deployed node(s), power via solar, no host PC at the site
- No secrets in the repo; node keys generated at provisioning.
