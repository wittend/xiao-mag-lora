# 00 — Requirements (Structured)

**Project:** xiao-mag-lora — solar-powered, GNSS clock-disciplined,
point-to-point LoRa telemetry link.
**Revision:** 1.0 — 2026-10-25 (structured back-fill of
`requirements/xaio-mag-ora_requirements.md` and `software_requirements.md`;
no requirement content changed).

Statuses used below: *In development*, *Implemented (unverified)*,
*Verified*, *Not started*, *Deferred*. Live status is tracked in the
progress reports and closed out in `02-completion-report.md`.

## 1. Purpose and Scope

A field-deployable, solar-powered **point-to-point (P2P) LoRa data link**
carrying one fixed telemetry record per second (timestamp, rate, x/y/z)
from a remote sensor node to a Linux host node. Not LoRaWAN, not mesh: no
gateways, network servers, or multi-hop routing. Out of scope for the
current phase: multi-node networks, positioning, encryption beyond simple
keys, and the host-side application stack (tracked as REQ-012…REQ-015).

## 2. System Requirements

### 2.1 Radio link

| ID | Requirement |
|----|-------------|
| REQ-001 | Two symmetric nodes (Wio-S3 Wireless LoRa, XIAO ESP32-S3 + SX1262) exchange data in P2P direct mode. A role flag selects transmitter (sends telemetry) or receiver (hosts the link). |
| REQ-002 | Radio operates in the US915 band (902–928 MHz) under 47 CFR §15.247. 2–4 pre-provisioned channels per site (e.g. 915.9/916.1/916.3/916.5 MHz); no dynamic frequency planning. |
| REQ-003 | Default data rate SF9 / 125 kHz / CR 4/5 / 17 dBm (213 ms on-air per frame); SF7 for backlog flush, SF8/SF10 selectable per site. SF chosen by link budget at the required on-air rate; no TX duty cycle applies. |
| REQ-004 | Link layer uses fixed 8 B header (magic, seq, type, CCITT-CRC field) + 18 B payload = **26 B frame**; payload is compact binary, 3-decimal fixed-point (spec: `docs/payload_format.md`). |
| REQ-005 | Reliable *ultimate* delivery: sequence numbers, receiver gap detection, and gap retransmit of accumulated records sent in bursts. No line-rate per-packet retry. |
| REQ-006 | Telemetry record per frame: `ts` (epoch seconds), `rt` (rate), `x`, `y`, `z`, each 3-decimal fixed-point. Sustained 1 record/s per direction. |
| REQ-007 | GNSS (L76K, UART @ 9600 baud, NMEA) is used for **clock maintenance only** — it disciplines the remote clock so `ts` and the 1 record/s cadence stay aligned. **No position data is transmitted.** |

### 2.2 Power, packaging, field operation

| ID | Requirement |
|----|-------------|
| REQ-008 | Solar-powered, field-deployable: solar panel → LiFePO4 charge controller → 4× 18650 pack (12.8 V nom) → 5 V to the node; deep-sleep duty cycle synced to solar charge state. |
| REQ-009 | Weatherproof 2" PVC Sch 40 enclosure with gasketed antenna penetrations; mountable on 1"–1.5" PVC pipe. |
| REQ-010 | Node provisioning over WiFi/BT at deploy: role, channel(s), keys, schedule. WiFi/BT **disabled in normal field operation**. |

### 2.3 Host-side (Linux) and web

| ID | Requirement |
|----|-------------|
| REQ-011 | Telemetry ingest: host-side software decodes 26 B binary frames to JSON for logging/display (spec: `docs/payload_format.md`). |
| REQ-012 | Client control software on Linux host (Go/C/C++/Rust; security- and portability-first across Debian/Ubuntu/Fedora/Arch/RHEL; single static binary preferred): talks to node(s) over LoRa and/or WiFi/BT in range; node provisioning (region/channel/keys), firmware update, diagnostics. |
| REQ-013 | History storage of the record stream plus node health (battery/charge state, link quality, temperature); SQLite is the working default. |
| REQ-014 | SPA web app (served by the host service) providing: node status board (online/offline, battery %, clock-sync state), live telemetry stream (WebSocket), node control (transmit test message, sleep, config change), link health view (RX count, SNR/SF, packet loss, last RX time). No mesh topology. |
| REQ-015 | CI builds firmware (PlatformIO) + client (static binary) + SPA and runs unit tests on every push; versioned artifacts in `dist/`; git tags mark releases. |

## 3. Quality / Constraint Requirements

| ID | Requirement |
|----|-------------|
| REQ-016 | Toolchain: PlatformIO (confirmed) with native custom firmware; ESP-IDF only if lower-level work becomes necessary. |
| REQ-017 | Link must be robust at ≤ 1 km line-of-sight with margin (4.7× on-air headroom at SF9 target per `docs/calculations.md`). |
| REQ-018 | No secrets in the repo; node keys generated at provisioning. No host PC at the field site (deploy-and-forget). |

## 4. Verification Approach (by requirement)

| REQ | Verification method |
|-----|---------------------|
| REQ-001, -004, -005 | 2-node bench link test (`docs/poc_walkthrough.md`): frame receipt, seq continuity, gap retransmit; JSON on host stdout |
| REQ-002, -003, -007 | Bench: provision channels over WiFi/BT; observe 1 record/s cadence drift with GNSS fix; RF measurements at range |
| REQ-006 | Byte-exact codec unit tests (native host test) + `src/host/decode.py --selftest` vs the worked example |
| REQ-008, -009 | Field evaluation: multi-day uptime, charge-state telemetry |
| REQ-010 | Provisioning walkthrough on bench; confirm radio disabled in normal op |
| REQ-011 | `decode.py --selftest`; ingest run against live bench node |
| REQ-012–REQ-015 | Host software build + functional tests; CI green; SPA exercised against a live node |

## 5. Open Questions (carried from prior phases)

- **Q1 (pin map):** SX1262 control pins CS/DIO1/BUSY/RST on the Wio-S3 are
  bench-verification pending (`docs/poc_walkthrough.md` pin gate). SPI pins
  are confirmed.
- **Q5 (power wiring):** P1-Pro teardown to confirm solar/charge wiring for
  the field node.

## 6. Revision History

| Rev | Date       | Change |
|-----|------------|--------|
| 1.0 | 2026-10-25 | Structured back-fill from original requirements + `software_requirements.md`; assigned REQ-001…REQ-018. No content changes. |
