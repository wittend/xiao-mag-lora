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
  ATGM336H-class, connects via XIAO expansion port over UART/I²C). **Clock
  role only** — maintains the remote node's clock so the `ts` timestamp and
  the 1 pkt/s cadence stay aligned. **No position data is transmitted.**
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
- **Roles:** one node = transmitter/sensor (sends the telemetry record);
  one node = receiver/relay toward the Linux host (GNSS maintains its
  clock). Symmetric firmware with a role flag is preferred (either Wio-S3
  can be either endpoint).
- **Link layer:** fixed **8-byte header** (magic, seq, type, CCITT-CRC field,
  reserved) + payload. **The payload is the telemetry record itself, compact
  binary, 3-decimal fixed-point — 26 B frame (spec: `docs/payload_format.md`).**
  Best-effort delivery with sequence-gap detection; ACK only for
  provisioning/command frames (ACKs also consume TX duty).
- **Frequency/channel plan (US915):** 2–4 pre-configured channels per site
  (e.g., 915.9 / 916.1 / 916.3 / 916.5 MHz) so multiple P2P links in the same
  area don't collide; no dynamic frequency planning (that's LoRaWAN's job).
  Channels provisioned at deploy via WiFi/BT.
- **Data rates (constrained by the 1 pkt/s budget — see §3):** SF9 default
  (213 ms for the 26 B binary frame — fits a 1 s slot with ~787 ms spare);
  SF10 (401 ms) for long links; SF7–SF8 for short/bench. SF11+ is **excluded**:
  airtime exceeds one 1 s slot.
- **Duty cycle:** FCC Part 15 (902–928 MHz) caps TX at **1 %** — continuous
  1 pkt/s is *not* compliant at any SF. 1 pkt/s runs only inside
  duty-limited acquisition windows (§3 item 3).
- **Provisioning flow:** WiFi/BT + FPC antenna at deploy for role, channel,
  keys, and schedule; node then runs on LoRa P2P + solar + GNSS.

## 3. Data-Rate Budget (1 packet/sec, small packets)
The data path carries **one fixed telemetry record per second** per
direction — nothing else (no GNSS position; GNSS is clock-only, see §1).
Typical packet (client example):

    { "ts":"26 Oct 2025 14:20:00", "rt":23.125, "x":12345.678,
      "y":-234.500, "z":987.001 }

1. **Throughput.** At 1 pkt/s the 26 B frame is 208 bps raw / **144 bps
   useful** (18 B payload). The application is designed against that budget.
   *(88 B JSON reference: 704 / 640 bps — see `docs/calculations.md` §B.)*
2. **Wire format — DECIDED: compact binary, 3-decimal fixed-point** (spec in
   `docs/payload_format.md`): 8 B header + 4 B epoch-seconds `ts` + 2 B `rt`
   ×1000 + 4 B each of `x`/`y`/`z` ×1000 = **26 B frame**. JSON remains the
   host-side representation. Airtime @125 kHz:

   | Frame       | SF7      | SF8      | SF9      | SF10      |
   |-------------|----------|----------|----------|-----------|
   | 88 B JSON   | 132 ms   | 238 ms   | 438 ms   | 811 ms ✗  |
   | 26 B binary | 59 ms    | 111 ms   | 213 ms   | 401 ms    |
3. **Regulatory ceiling — the hard constraint.** FCC Part 15 (902–928 MHz)
   allows ≤ 1 % TX duty = 0.6 s airtime per minute. Minimum possible
   airtime per packet is ~59 ms (26 B @ SF7) → **continuous 1 pkt/s is
   non-compliant at every spreading factor** (5.9–44 % duty). With the 26 B
   frame: SF9 (213 ms) allows **≤ ~2 s of 1 pkt/s per minute**; SF10 (401 ms)
   ~1.5 s per minute. ⇒ The 1 pkt/s rate is achievable only in duty-limited
   **acquisition windows**; nominal window length is open question Q9 and
   drives autonomy, ingest buffering, and compliance.
4. **No retransmission storms.** Lost packets are not auto-retried at line
   rate (retries would blow both the 1 s slot and the duty budget); the link
   layer uses sequence numbers + best-effort, with optional single ACK for
   critical/config frames only. Application-level cadence absorbs loss.

   *All derivations and estimates: `docs/calculations.md` (appendix).*

### 3a. Pivotal decision — sustained vs windowed 1 pkt/s

Sustained 1 pkt/s is **unachievable on US915 LoRa at any SF** (minimum
possible duty 5.9 % vs 1 % limit — a hard floor, not tunable). The protocol
survives **only if** 1 pkt/s is required *during acquisition windows* (SF9:
28 pkts/min, ~2.8 s of 1-pkt-s per minute). If the client requires
**sustained 24/7** 1 pkt/s, the physical layer must change — the 1 % duty
binds *spread-spectrum* transmitters only, and the same Wio-S3 board covers
both alternatives:

- **GFSK/FSK narrowband, 902–928 MHz** (SX1262 does GFSK natively): falls
  under §15.247(c), 50 mW TPO, **no duty-cycle limit**. 26 B @ 9.6 kbps ≈
  30 ms airtime → continuous 1 pkt/s compliant. Cost: ~10–30× less range
  than SF9 (no processing gain; ≈ −115 dBm vs −147 dBm).
- **2.4 GHz WiFi** (on-board): no duty-cycle limit, but an infrastructure
  link (needs a base/coverage), not a direct P2P radio path.

⇒ **Client question (decides the physical layer):** sustained or windowed?
If sustained **and** km-scale range is required, no single Wio-S3 path
satisfies both and the project should stop. See `docs/calculations.md` §C.


## 4. Power Subsystem (decided 2026-10-24)

- **Decision: adapt the SenseCAP Solar Node P1-Pro.** It already uses 4× 18650
  LiFePO4 cells (matches the requirement to manage four 18650 cells), provides
  solar charge management. Its own ESP32-S3 radio is **not** used for the
  data link (P2P only); the P1-Pro serves purely as the solar/LiFePO4 power
  subsystem.
- **Solar panel:** 16–20 W class, ~18 V peak (Vmp/Voc to be confirmed from the
  selected panel's datasheet). Sized for **≥ 2–3 days sunless autonomy** at
  expected duty cycle: 1 pkt/s inside duty-limited acquisition windows (open
  Q9) + GNSS clock-hold + deep sleep between windows.
  - Autonomy math to be added once Q9 (acquisition-window length) is fixed.

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
6. ~~Duty cycle / sleep profile~~ — resolved by the 1 pkt/s + 1 % duty
   analysis in §3 item 3; remaining gap is Q9 below.
7. ~~Wire format~~ — **decided: binary, 3-decimal fixed-point.** 26 B frame,
   full spec in `docs/payload_format.md` (SF10 now available for long links).
8. ~~Binary field scales~~ — **decided: 3-decimal fixed-point** (×1000).
   Residual to verify at acceptance: `rt` unsigned 0–65.535, x/y/z within
   int32×1000 range, second-resolution `ts`.
9. **PIVOTAL (client) — sustained or windowed 1 pkt/s?** Sustained 24/7
    is unachievable on US915 LoRa (§3a) and forces a physical-layer change
    (GFSK narrowband or WiFi — same board). Windowed keeps the LoRa design.
    This single answer decides the protocol; confirm before any bench work.
10. **Acquisition-window length (if windowed, client):** how long must the
    link sustain 1 pkt/s (seconds per window, windows per hour)? Drives
    compliance, TX energy/autonomy math, and host ingest buffering.

## 7. Procurement Note

Order from Seeed Studio US warehouse only; confirm stock at order time to
avoid delays (per requirements). A partial kit will be mailed by the client
for evaluation — reserve bench testing until it arrives.
