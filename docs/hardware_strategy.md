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
- **Data rates (constrained by the flush-rate budget — see §3):** use the
  **fastest SF that still links at ≤1 km LoS** — SF7 (59 ms), or SF6/250 kHz
  (16 ms), as default; SF8–SF9 only if a specific site needs the range. Faster
  SF = more flush throughput per unit of TX duty. SF11+ **excluded** (slowest,
  worst flush rate, never needed at this range).
- **Duty cycle:** FCC Part 15 (902–928 MHz) caps **average** TX duty (exact
  limit = open Q9, unverified). This bounds the radio's **flush rate**, not the
  1/s sampling rate. Delivery is bursty (windowed); sustained on-air 1/s is
  neither required nor achievable (§3a).
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
3. **Regulatory ceiling.** FCC Part 15 (902–928 MHz) caps **average TX duty**;
   the exact limit for a fixed-to-fixed spread-spectrum link is the single
   decisive unknown (§3a; my earlier “1 %” was unverified). It bounds the
   **flush rate** (duty ÷ airtime-per-sample), not the 1/s sampling rate.
   Sustained on-air 1/s is unachievable at any SF; **bursty (windowed)
   delivery is the operating mode** and is compliant by construction (average
   duty ≤ limit). See `docs/calculations.md` §C.
4. **No retransmission storms.** Lost packets are not auto-retried at line
   rate (retries would blow the TX duty budget); the link
   layer uses sequence numbers + best-effort, with optional single ACK for
   critical/config frames only. Application-level cadence absorbs loss.

   *All derivations and estimates: `docs/calculations.md` (appendix).*

### 3a. Operating model — sampling cadence vs delivery (corrected)

The 1/s figure is a **sampling** cadence, not a required **on-air** rate. The
node samples 1 record/s into local non-volatile storage (GNSS-clocked `ts`);
the radio delivers the accumulated records to the single peer **in bursts**,
delay-tolerant, with **reliable ultimate delivery** of every sample. Distance
target is ≤ ~1 km unobstructed LoS (less is fine).

This decouples the radio from the 1/s rate. The only real constraint is the
**average on-air duty** (FCC) vs the **flush rate** it permits:

    flush_rate (samples/s) = duty_limit / airtime_per_sample

Steady state (lag stays bounded) needs flush_rate ≥ 1.0. If not, the node can
still deliver everything eventually (bursty, delay-tolerant) — the backlog just
drains slower than it fills, so lag grows; acceptable only if the client
tolerates that.

| SF/BW    | airtime/sample | flush @1%  | flush @10% |
|----------|----------------|------------|------------|
| SF6/250k | 16 ms          | 0.62/s     | 6.2/s      |
| SF7/250k | 30 ms          | 0.34/s     | 3.4/s      |
| SF7/125k | 59 ms          | 0.17/s     | 1.7/s      |
| SF8/125k | 111 ms         | 0.09/s     | 0.9/s      |
| SF9/125k | 213 ms         | 0.05/s     | 0.5/s      |

Batching (N records/frame) amortizes the 8 B header: at SF7/125k/1 %, 8-per-
frame raises flush to 0.39/s (~2.3×).

**Range is not the constraint** (≤1 km LoS; SF7 has ~67 dB margin there), so we
use the *fastest* SF that still links — SF7/SF6 — not SF9.

⇒ **Decisive unknown: the actual FCC TX duty limit** for a fixed-to-fixed
902–928 MHz spread-spectrum link. *(My earlier “1 %” was from memory and I
could not verify the CFR text in this session.)* At ~10 % the node reaches
steady state with large margin (SF7/250k → 3.4× accumulation). At a strict 1 %,
24/7 sampling cannot be flushed in real time by any LoRa SF — delivery must be
bursty with growing lag, or sampling is decimated. If 1 % is real **and** lag
must be bounded, the same board falls back to 902–928 MHz GFSK (no duty limit,
~10–30× less range) or 2.4 GHz WiFi.


## 4. Power Subsystem (decided 2026-10-24)

- **Decision: adapt the SenseCAP Solar Node P1-Pro.** It already uses 4× 18650
  LiFePO4 cells (matches the requirement to manage four 18650 cells), provides
  solar charge management. Its own ESP32-S3 radio is **not** used for the
  data link (P2P only); the P1-Pro serves purely as the solar/LiFePO4 power
  subsystem.
- **Solar panel:** 16–20 W class, ~18 V peak (Vmp/Voc to be confirmed from the
  selected panel's datasheet). Sized for **≥ 2–3 days sunless autonomy**. The
  node samples 1/s continuously (cheap, on-chip); the radio flushes accumulated
  records in bursts bounded by the TX duty limit (open Q9). Dominant draws:
  flush TX + GNSS clock-hold + deep sleep between flushes.

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
9. **DECISIVE — actual FCC TX duty limit** for a fixed-to-fixed 902–928 MHz
    spread-spectrum link (verify CFR §15.247(d)(4); my “1 %” was unverified).
    Bounds flush rate: steady state needs duty/airtime ≥ 1/s (§3a).
10. **Client — delivery-lag tolerance:** must the delivered backlog be bounded
    (near-real-time, i.e. flush ≥ accumulation — needs ~10 %+ duty at SF7), or
    is delay-tolerant “eventually, reliably” acceptable (works at any duty, lag
    grows when duty is low)?

## 7. Procurement Note

Order from Seeed Studio US warehouse only; confirm stock at order time to
avoid delays (per requirements). A partial kit will be mailed by the client
for evaluation — reserve bench testing until it arrives.
