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
  provisioning/command frames (ACKs also consume on-air airtime).
- **Frequency/channel plan (US915):** 2–4 pre-configured channels per site
  (e.g., 915.9 / 916.1 / 916.3 / 916.5 MHz) so multiple P2P links in the same
  area don't collide; no dynamic frequency planning (that's LoRaWAN's job).
  Channels provisioned at deploy via WiFi/BT.
- **Data rates (see §3):** choose the SF by **link budget at the required on-air
  rate**, not by a duty budget — 47 CFR §15.247 imposes **no TX duty cycle** on
  902–928 MHz. Sustained 1 packet/s needs airtime ≤ 1 s (true for SF ≤ 10).
  Default **SF9 (213 ms)** for margin; SF7/250 kHz (30 ms) for flush headroom;
  SF8/SF10 per site. SF11+ only if airtime is not a concern.
- **Regulatory (902–928 MHz, verified):** the real §15.247 caps are 1 W peak
  conducted power, ≤ 8 dBm PSD in any 3 kHz during continuous TX, and OOB ≥ 20 dB
  below in-band. The Wio-S3 at ~+20 dBm / 125 kHz CSS meets all three with
  margin. One open item: confirm the CSS/500-kHz 6-dB-bandwidth mapping (§C, Q9).
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
3. **Regulatory.** No FCC TX duty cycle applies to 902–928 MHz (verified
   §15.247; my earlier “1 %” was an error and is retracted). The binding caps are
   1 W conducted power and ≤ 8 dBm PSD per 3 kHz during continuous TX — both met
   with margin. So the on-air rate is bounded only by 100 % duty (physics):
   sustained 1/s is feasible at SF7–SF10 (SF11 marginal). See
   `docs/calculations.md` §C.
4. **No retransmission storms.** Lost packets are not auto-retried at line
   rate (retries would consume on-air airtime); the link
   layer uses sequence numbers + best-effort, with optional single ACK for
   critical/config frames only. Application-level cadence absorbs loss.

   *All derivations and estimates: `docs/calculations.md` (appendix).*

### 3a. Operating model — sampling vs delivery

**The 1/s figure is a sampling cadence, not a required on-air rate** (client:
delay-tolerant, reliable *ultimate* delivery, post-processed much later). The node
samples 1 record/s into local non-volatile storage (GNSS-clocked `ts`); the radio
delivers the accumulated records in bursts to the single peer. Distance target is
≤ ~1 km unobstructed LoS (less is fine).

With **no TX duty cycle** (§C), this decoupling is a *design choice for
robustness* — a lost packet drops only a batch of already-stored samples, nothing
is lost at the source — not a regulatory necessity. The on-air ceiling is simply
100 % duty plus the PSD limit: sustained 1 packet/s fits at SF7 (16.9× headroom)
down to SF10 (2.5×); only SF11 (1.25×) is near the wire.

Because we sample locally and flush in bursts, **range and data rate trade per
site**: use the fastest SF that links (SF7, ~67 dB margin at 1 km LoS) for flush
headroom, or step down SF for a marginal link. Batching N records/frame further
cuts total airtime (amortizes the 8 B header).

⇒ **No compliance wall.** The single open regulatory item is confirming how CSS
LoRa at 125 kHz maps to §15.247(a)(2)’s 500 kHz 6-dB-bandwidth minimum (the Wio-S3
FCC grant / LoRa Alliance US915-928 spec resolves it; if 500 kHz binds we simply
run 500 kHz BW — shorter airtimes, more PSD margin). → Q9. GFSK/WiFi are same-board
fallbacks only if we ever outgrow LoRa’s airtime — not a duty escape.


## 4. Power Subsystem (decided 2026-10-24)

- **Decision: adapt the SenseCAP Solar Node P1-Pro.** It already uses 4× 18650
  LiFePO4 cells (matches the requirement to manage four 18650 cells), provides
  solar charge management. Its own ESP32-S3 radio is **not** used for the
  data link (P2P only); the P1-Pro serves purely as the solar/LiFePO4 power
  subsystem.
- **Solar panel:** 16–20 W class, ~18 V peak (Vmp/Voc to be confirmed from the
  selected panel's datasheet). Sized for **≥ 2–3 days sunless autonomy**. The
  node samples 1/s continuously (cheap, on-chip); the radio flushes accumulated
  records in bursts (bounded only by 100 % on-air airtime; no TX duty cycle, §C).
  Dominant draws: flush TX + GNSS clock-hold + deep sleep between flushes.

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
6. ~~TX duty cycle~~ — **none exists** in 47 CFR §15.247 (verified; my “1 %” was
   an error). Sleep profile is set by solar charge state, not by any duty cap.
7. ~~Wire format~~ — **decided: binary, 3-decimal fixed-point.** 26 B frame,
   full spec in `docs/payload_format.md` (SF10 now available for long links).
8. ~~Binary field scales~~ — **decided: 3-decimal fixed-point** (×1000).
   Residual to verify at acceptance: `rt` unsigned 0–65.535, x/y/z within
   int32×1000 range, second-resolution `ts`.
9. **Compliance — CSS LoRa US915 class / bandwidth:** confirm how Chirp Spread
    Spectrum at 125 kHz maps to §15.247(a)(2)’s 500 kHz 6-dB-bandwidth minimum
    (resolve via the Wio-S3 FCC grant / LoRa Alliance US915-928 spec; §C).
10. **Client — delivery-lag tolerance (now low-risk):** with no duty cycle,
    bounded/near-real-time lag is achievable at any SF ≤ 10, so either reading
    (real-time or delay-tolerant) is satisfiable. Confirm which, to set the
    flush scheduler.

## 7. Procurement Note

Order from Seeed Studio US warehouse only; confirm stock at order time to
avoid delays (per requirements). A partial kit will be mailed by the client
for evaluation — reserve bench testing until it arrives.
