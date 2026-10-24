# Software Requirements

## Firmware (on-node)

- **MCU:** ESP32-S3 (Wio-S3 Wireless LoRa, XIAO form factor)
- **Toolchain (decided):** PlatformIO — Meshtastic's official build
  path. ESP-IDF only if lower-level work becomes necessary.
- **Base firmware:** Meshtastic (provides LoRa mesh, telemetry, web client,
  position handling). Fork/customize for:
  - L76K GNSS integration (position fix → mesh position broadcasts)
  - Power management: deep-sleep schedule synced to solar charge state
  - (VERIFY) exact pin map for GNSS + battery sense on this board
- **Radio config:** US915, default 915.9 MHz (Meshtastic US default),
  LoRa SF7–SF12, long/medium/fast regional presets as needed.

## Client-side (Linux host)

- **Languages:** any combination of Go / C / C++ / Rust; prioritizing
  security and portability across popular distributions (Debian, Ubuntu,
  Fedora, Arch, RHEL).
- **Responsibilities:**
  - Talk to node(s) over LoRa (a host LoRa radio) and/or WiFi/BT when in range
  - Node provisioning (region/channel/keys), firmware update, diagnostics
  - Telemetry ingest: GNSS fixes, battery/charge state, link quality,
    temperature
  - Store history (SQLite is a reasonable default — decision to confirm)
- **Deliverable shape:** single static binary preferred (Go or Rust) to
  minimize distribution pain.

## SPA Web App (testing & monitoring)

- Served by the client-side service (or standalone build consumed by it).
- Features (v1):
  - Node status board: online/offline, battery %, last fix, position map
  - Live log/telemetry stream (WebSocket)
  - Node control: transmit test message, sleep, config change
  - Mesh topology / last-hop view
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
