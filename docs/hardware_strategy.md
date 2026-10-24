# Hardware Strategy

*Draft v0.1 — items marked (VERIFY) should be confirmed against the Seeed
datasheets in `assets/` before ordering.*

## 1. System Overview

A field-deployable **point-to-point (P2P) LoRa data link**: two Wio-S3 nodes
exchanging data directly over LoRa. **Not LoRaWAN, not mesh** — no
gateways, no network server, no multi-hop routing. Each link is a dedicated
two-node (or star of direct links) radio path.

- **Radio/compute:** 2× Seeed Studio **Wio-S3 Wireless LoRa** (XIAO ESP32-S3
  + onboard LoRa, 868/915 MHz, SMA connector) — one for the deployed node,
  one for bench/inter-node testing. (VERIFY: confirm exact LoRa radio part
  number and band selection from the product PDF; US deployment ⇒ US915.)
- **GNSS:** 1× **L76K GNSS Module for XIAO** (VERIFY: chip — likely
  ATGM336H-class, connects via XIAO expansion port over UART/I²C) provides
  position for telemetry payload on the P2P link.
- **2.4 GHz RF:** 2.4 GHz FPC Antenna A-02 (1.6 dBi) ×2 for the ESP32-S3
  WiFi/BT radio — used for one-time external configuration, firmware
  flashing, and diagnostics.
- **Power:** solar panel → LiFePO4 charge controller → 4× 18650 LiFePO4
  pack (nominal 12.8 V), down-converted to 5 V for the Wio-S3.
- **Enclosure:** 2" PVC Sch 40 pipe with end caps, PVC cemented; 1"–1.5"
  pipe segments for mounting. Antenna penetrations gasketed for weather
  sealing.

## 2. Firmware Direction (P2P, not mesh/LoRaWAN)
The Wio-S3 LoRa radio (SX1262-class, (VERIFY)) is driven in **direct P2P
mode** — no LoRaWAN MAC, no network server. Firmware is a custom
PlatformIO application:
- **Roles:** one node = transmitter/sensor (GNSS fix + telemetry payload);
  one node = receiver/relay toward the Linux host. Symmetric firmware with a
  role flag is preferred (either Wio-S3 can be either endpoint).
- **Link layer:** custom lightweight frames over LoRa — 2-byte magic, seq,
  type (data/ack/keepalive), payload CRC (CCITT), fixed SF/bandwidth per link
  class. Optional simple ACK + N-retry; deep sleep between exchanges.
- **Frequency/channel plan (US915):** 2–4 pre-configured channels per site
  (e.g., 915.9 / 916.1 / 916.3 / 916.5 MHz) so multiple P2P links in the same
  area don't collide; no dynamic frequency planning (that's LoRaWAN's job).
  Channels provisioned at deploy via WiFi/BT.
- **Data rates:** default SF10–SF12 long range for fixed site pairs;
  higher-rate (SF7–SF8) mode selectable for short/bench links.
- **Duty cycle:** US915 0.1 % (or the current FCC Part 15 duty limits) is
  **not** a constraint for infrequent fixed P2P traffic, but TX duty and
  sleep schedule still drive the autonomy math.
- **Provisioning flow:** WiFi/BT + FPC antenna at deploy for role, channel,
  keys, and schedule; node then runs on LoRa P2P + solar + GNSS.
## 3. Power Subsystem (decided 2026-10-24)

- **Decision: adapt the SenseCAP Solar Node P1-Pro.** It already uses 4× 18650
  LiFePO4 cells (matches the requirement to manage four 18650 cells), provides
  solar charge management. Its own ESP32-S3 radio is **not** used for the
  data link (P2P only); the P1-Pro serves purely as the solar/LiFePO4 power
  subsystem.
- **Solar panel:** 16–20 W class, ~18 V peak (Vmp/Voc to be confirmed from the
  selected panel's datasheet). Sized for **≥ 2–3 days sunless autonomy** at
  expected duty cycle (LoRa TX every N min + GNSS fix cadence).
  - Autonomy math to be added once the duty cycle / sleep schedule is fixed in
    firmware (see open questions).

## 4. Mechanical

- 2" Sch 40 pipe body (≈ 60 mm ID) fits the pack + Wio-S3 stack; end caps
  cemented with PVC cement, drilled for SMA antenna and (optionally) GNSS
  feed-through or top-hat GNSS mount.
- 1"–1.5" pipe fittings for pole/mast mounting.
- Sealing: gaskets at caps, RTV at antenna glands.

## 5. Open Questions

1. ~~Solar panel W / autonomy~~ — decided: 16–20 W, ~18 V peak, 2–3 days.
2. ~~Charge controller~~ — decided: SenseCAP P1-Pro (4× 18650 LiFePO4).
3. Wio-S3 LoRa radio part + band (US915) confirmation.
4. L76K interface pins on the Wio-S3 (shared UART?) — check datasheet.
5. Exact P1-Pro ↔ Wio-S3 power/interface wiring (P1-Pro is itself an
   ESP32-S3 node — used for power only in this P2P design; confirm its
  radio stays disabled and how 5 V is tapped for the Wio-S3.)
6. Duty cycle / sleep schedule (drives the autonomy math).

## 6. Procurement Note

Order from Seeed Studio US warehouse only; confirm stock at order time to
avoid delays (per requirements). A partial kit will be mailed by the client
for evaluation — reserve bench testing until it arrives.
