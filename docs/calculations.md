# Calculations & Estimates (Appendix)

*Engineering math behind the design decisions in `hardware_strategy.md` and
`payload_format.md`. Formulas first, then numbers. Items marked *(ASSUMPTION)
need verification against the real hardware (P1-Pro spec sheet, Wio-S3
measurements, site survey).*

## A. LoRa airtime

Semtech SX126x symbol/packet model, 125 kHz bandwidth, 8-symbol preamble,
CRC on:

```
N_payload = 8 + ceil((8·PL − 4·SF) / SF)          (SF ≤ 11)
N_symbol  = N_payload + 16 (CRC) + 8 (preamble)
T_symbol  = 2^SF / BW
T_packet  = N_symbol · T_symbol
```

**26 B binary frame (decided wire format):**

| SF | symbols | airtime | fits 1 s slot |
|----|---------|---------|---------------|
| SF7 | 57 | 59 ms | yes |
| SF8 | 107 | 111 ms | yes |
| SF9 | 206 | 213 ms | yes — **default** |
| SF10 | 409 | 401 ms | yes — long range |
| SF11 | 816 | 801 ms | **no** |
| SF12 | 1642 | 1610 ms | **no** |

*(88 B JSON frame, rejected for the wire: SF9 = 438 ms, SF10 = 811 ms.)*

## B. Throughput at 1 packet/sec

| Frame | raw | useful (− 8 B header) |
|-------|-----|-----------------------|
| 26 B binary (chosen) | 208 bps | **144 bps** (18 B payload = 4+2+4+4+4 fields) |
| 88 B JSON (reference) | 704 bps | 640 bps |

The application is designed against **144 useful bits/sec**.

## C. FCC Part 15 constraints (902–928 MHz) — verified against eCFR

**There is no TX duty cycle.** I verified the actual text of 47 CFR Part 15
(§§ 15.205, 15.207, 15.209, 15.247, 15.249) via the eCFR API (2026). The word
"duty" does not appear in §15.247, and the provision I earlier cited as the
source of a "1 % TX duty" — "§15.247(d)(4)(iii)" — **does not exist**. That
figure was an error and is retracted. (The only "duty" in Part 15 is a buried-
cable-marker exemption that disallows duty compensation — the opposite of a
transmit cap.)

For a 902–928 MHz **digitally-modulated** transmitter (LoRa CSS is treated in
this class), the real §15.247 constraints are:

| Provision | Rule | Our design (Wio-S3, ~+20 dBm, 125 kHz CSS) | Verdict |
|-----------|------|--------------------------------------------|---------|
| (b)(3) peak conducted power | 1 Watt | 0.1 W | ✅ 10 dB headroom |
| (e) PSD, continuous TX | ≤ 8 dBm in any 3 kHz | ≈ +3.8 dBm/3 kHz @125 kHz; +0.8 @250 kHz | ✅ ~4 dB margin |
| (d) out-of-band | ≥ 20 dB below in-band in any 100 kHz out | clean chirp | ✅ routine |
| (b)(4)/(c) antenna gain | **no > 6 dBi point-to-point exemption** in 902–928 (exists only for 2.4/5.8 GHz) → back off (gain−6 dBi) dB | 2 dBi now; high-gain later = power tradeoff | ⚠️ note |

PSD check: 20 dBm spread over a 125 kHz chirp → 20 − 10·log10(125 000) =
−31 dBm/Hz; in a 3 kHz band that's −31 + 10·log10(3 000) = **+3.8 dBm**, under
the 8 dBm limit (even lower at 250 kHz). So continuous TX at our power is fine.

**Consequence for the data path.** With no duty cycle, on-air throughput is
bounded only by (1) 100 % on-air duty (physics) and (2) the PSD limit (met).
Sustained on-air ceiling = 1 / airtime_per_packet:

| SF/BW   | airtime | sustained-on-air ceiling |
|---------|---------|--------------------------|
| SF7/125k | 59 ms | ~16.9 pkts/s |
| SF8/125k | 111 ms | ~9.0 pkts/s |
| SF9/125k | 213 ms | ~4.7 pkts/s |
| SF10/125k | 401 ms | ~2.5 pkts/s |
| SF11/125k | 801 ms | ~1.25 pkts/s |

Sustained 1 packet/s is comfortably feasible at SF7–SF10 (only SF11 is near
the wire). The delay-tolerant "sample-locally, flush-in-bursts" model is even
more comfortable; batching N records/frame shortens total airtime further.

**Open item (not a blocker) — confirm, don't assume:** §15.247(a)(2) requires
digital-modulation systems in 902–928 to have a **minimum 6 dB bandwidth of
500 kHz**. How CSS LoRa at 125 kHz satisfies that clause is established by the
Wio-S3's FCC grant / the LoRa Alliance US915-928 spec, not the raw text. If the
500 kHz floor binds, we run 500 kHz BW — a standard US915 setting that
shortens airtimes and widens PSD margin (a non-downside). → Q9.

**Alternatives if we ever outgrow LoRa's airtime** (not needed for 1/s): the
same SX1262 supports GFSK/FSK (narrowband, higher bps), and the Wio-S3 ESP32-
S3 supports WiFi (2.4 GHz, §15.407).


## D. Link budget estimate (FSPL, 915 MHz)


```
FSPL(dB) = 32.45 + 20·log10(f_MHz) + 20·log10(d_km)
RX (dBm) = TX(20) + TXant(2) + RXant(2) − FSPL − impl.loss(2)
```
*(ASSUMPTIONS: +20 dBm TX (SX1262 max 22), 2 dBi SMA whip both ends,
2 dB implementation losses.)*

Standard SX126x sensitivities @125 kHz: SF7 −137, SF8 −143, SF9 −147,
SF10 −151, SF11 −155, SF12 −158 dBm.

| Distance | FSPL | RX level | margin @SF9 | margin @SF10 |
|----------|------|----------|-------------|--------------|
| 1 km | 91.7 dB | −69.7 dBm | 77 dB | 81 dB |
| 5 km | 105.7 dB | −83.7 dBm | 63 dB | 67 dB |
| 10 km | 111.7 dB | −89.7 dBm | 57 dB | 61 dB |
| 50 km | 125.7 dB | −103.7 dBm | 43 dB | 47 dB |

Free-space math shows large margins at km scale; real-world limits will be
terrain/obstruction/shadowing, not FSPL. At the stated ≤1 km LoS target even
SF7 (least margin) keeps ~67 dB, so range is not the constraint — the fastest
SF that links is the right default (§3a).

## E. Power & autonomy estimate

*(ASSUMPTIONS — verify with P1-Pro spec + bench measurements:)*
- 4× 18650 LiFePO4 @ 3.0 Ah, 3.2 V → 4S pack 12.8 V, 38.4 Wh; 80 % DOD →
  **30.7 Wh usable**. *(P1-Pro cell count/capacity to confirm.)*
- GNSS clock-hold: 20 mA continuous ≈ 0.26 W → **6.2 Wh/day** — the dominant
  drain.
- Deep-sleep base (ESP32-S3 + RTC + P1-Pro controller): ~0.3 mW → 0.007
  Wh/day (negligible).
- TX @SF9 (213 ms/pkt, ~100 mA at 12.8 V): 0.021 Wh per packet; a 60 s
  1-pkt/s window ≈ 1.28 Wh.

**Autonomy (GNSS hold + light TX): ~4–5 days** → meets the 2–3 day target
with margin. If Q9 yields long/frequent windows, subtract ~1.3 Wh per
60 s of 1-pkt/s from the 30.7 Wh budget.

**Solar sizing check:** 16–20 W panel × 3–5 peak-sun-hours × 80 % charge
efficiency = **38–80 Wh/day** available vs ~6–8 Wh/day consumption.
Recharging a fully drained pack after 3 sunless days needs ~20 Wh — covered
by a single average day of sun. Panel choice is therefore driven more by
mechanical fit than by energy balance.

## F. Fixed-point encoding

Scale 1000 (3 decimals, per spec):

| field | JSON value | encoded | C type | range |
|-------|-----------|---------|--------|-------|
| rt | 23.125 | 23125 | uint16 ×1000 | 0 – 65.535 |
| x | 12345.678 | 12345678 | int32 ×1000 | ±21,474,836.47 |
| y | −234.500 | −234500 | int32 ×1000 | ±21,474,836.47 |
| z | 987.001 | 987001 | int32 ×1000 | ±21,474,836.47 |
| ts | 26 Oct 2025 14:20:00 UTC | 1761488400 | uint32 epoch s | valid to 2106-02-07 |

Residual assumptions (flagged in `payload_format.md`): `rt` unsigned
0–65.535; x/y/z within int32×1000 range; second-resolution `ts` sufficient.

## G. What would change these numbers

| Change | Effect |
|--------|--------|
| Q9 (CSS LoRa US915 class / 500 kHz BW) | confirms the PSD/bandwidth path; if 500 kHz binds, airtimes halve (favorable) |
| `rt` needs negatives / > 65.535 | frame → 30 B; SF10 airtime → ~460 ms (still fits 1 s) |
| BW raised to 250 kHz | airtimes halve → higher on-air ceiling, less range, wider PSD margin |
| Measured Wio-S3/P1-Pro currents | replaces E's assumptions with data |
