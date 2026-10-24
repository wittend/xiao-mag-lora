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
- **Link layer:** fixed **8-byte header** (magic, seq, type, CCITT-CRC field,
  reserved) + payload. **Payload target 16 B, design default 32 B, hard max 64 B
  (64 B only at ≤ SF9).** Best-effort delivery with sequence-gap detection;
  ACK only for provisioning/command frames (ACKs also consume TX duty).
- **Frequency/channel plan (US915):** 2–4 pre-configured channels per site
  (e.g., 915.9 / 916.1 / 916.3 / 916.5 MHz) so multiple P2P links in the same
  area don't collide; no dynamic frequency planning (that's LoRaWAN's job).
  Channels provisioned at deploy via WiFi/BT.
- **Data rates (constrained by the 1 pkt/s budget — see §3):** default SF9;
  SF10 for long links (442 ms at 32 B — fits a 1 s slot with ~558 ms to
  spare); SF7–SF8 for short/bench links. SF11+ is **excluded**: airtime
  exceeds one 1 s slot at any useful payload.
- **Duty cycle:** FCC Part 15 (902–928 MHz) caps TX at **1 %** — continuous
  1 pkt/s is *not* compliant. 1 pkt/s is the instantaneous protocol rate;
  sustained operation uses the duty-cycled profiles in §3.
- **Provisioning flow:** WiFi/BT + FPC antenna at deploy for role, channel,
  keys, and schedule; node then runs on LoRa P2P + solar + GNSS.

## 3. Data-Rate Budget (1 packet/sec, small packets)
The data path is capped at **1 small packet per second** per direction.
Treat this as a hard budget, not an average:

1. **Packet size.** LoRa payload cap is 255 B, but the budget calls for small
   packets. Design default **32 B payload**; 16 B minimum for GNSS-only
   telemetry. At 1 pkt/s that is a ~**256 bps raw / ~190 bps useful** link
   (after 8 B header + CRC) — plan every application around ~200 bps.
2. **Airtime must fit the 1 s slot.** Semtech airtime @125 kHz (8-byte
   preamble, CRC on):

   | SF  | 16 B  | 32 B  | 64 B  |
   |-----|-------|-------|-------|
   | SF7 | 48 ms | 67 ms | 104 ms |
   | SF8 | 90 ms | 123 ms| 188 ms |
   | SF9 | 176 ms| 233 ms| 348 ms |
   | SF10| 336 ms| 442 ms| 655 ms |
   | SF11| 725 ms| 922 ms| 1298 ms |  ← exceeds 1 s at any useful payload
   | SF12| 1.45 s| 1.78 s| 2.50 s |

   ⇒ **SF9 is the default**; SF10/32 B (442 ms) is the range-maximum that still
   fits a 1 s slot (leaving ~558 ms for RX/ACK). SF11+ excluded from the 1 pkt/s
   mode. 64 B payloads only at ≤ SF9.
3. **Sustained rate is duty-cycle-limited, not just budget-limited.** FCC
   Part 15 (902–928 MHz) allows ≤ 1 % TX duty = 0.6 s of airtime per minute:
   - SF9/32 B (233 ms) → max ~2.5 pkts/min ≈ **1 packet per ~23 s**
   - SF10/32 B (442 ms) → max ~1.4 pkts/min ≈ **1 packet per ~44 s**
   So the 1 pkt/s figure is the *instantaneous* protocol clock; a compliant
   node operates in **bursts or a slow sustained cadence**, e.g. Profile A:
   *sustained* — 1 telemetry packet every 25–45 s; Profile B: *burst* — 1 pkt/s
   for a few seconds (uplink of a queued event) then long sleep. Which profile
   is the nominal one is an open question (client to confirm).
4. **No retransmission storms.** Lost packets are not auto-retried at line
   rate (retries would blow both the 1 s slot and the duty budget); the link
   layer uses sequence numbers + best-effort, with optional single ACK for
   critical/config frames only. Application-level cadence absorbs loss.

## 4. Power Subsystem (decided 2026-10-24)

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

## 5. Mechanical

- 2" Sch 40 pipe body (≈ 60 mm ID) fits the pack + Wio-S3 stack; end caps
  cemented with PVC cement, drilled for SMA antenna and (optionally) GNSS
  feed-through or top-hat GNSS mount.
- 1"–1.5" pipe fittings for pole/mast mounting.
- Sealing: gaskets at caps, RTV at antenna glands.

## 6. Open Questions

1. ~~Solar panel W / autonomy~~ — decided: 16–20 W, ~18 V peak, 2–3 days.
2. ~~Charge controller~~ — decided: SenseCAP P1-Pro (4× 18650 LiFePO4).
3. Wio-S3 LoRa radio part + band (US915) confirmation.
4. L76K interface pins on the Wio-S3 (shared UART?) — check datasheet.
5. Exact P1-Pro ↔ Wio-S3 power/interface wiring (P1-Pro is itself an
   ESP32-S3 node — used for power only in this P2P design; confirm its
  radio stays disabled and how 5 V is tapped for the Wio-S3.)
6. Duty cycle / sleep schedule (drives the autonomy math).

7. Confirm the operating profile with the client (sustained cadence vs
   burst windows) — drives autonomy math and client ingest rate.

## 7. Procurement Note

Order from Seeed Studio US warehouse only; confirm stock at order time to
avoid delays (per requirements). A partial kit will be mailed by the client
for evaluation — reserve bench testing until it arrives.
