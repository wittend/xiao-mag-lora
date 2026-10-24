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

## C. FCC Part 15 duty cycle (902–928 MHz)

Limit: **1 % TX duty = 0.6 s airtime per minute** (47 CFR §15.247(d)(4)(iii) —
applies to *frequency-hopping and spread-spectrum* transmitters; LoRa CSS is
spread spectrum, so it binds. Also 1 W conducted / 30 dBm EIRP — well above
our ~20 dBm TX).

Sustainable cadence, 26 B frame:

| SF | airtime | max compliant | 1 pkt/s window per minute |
|----|---------|---------------|---------------------------|
| SF7 | 59 ms | 10.1 pkts/min | ~1 s |
| SF8 | 111 ms | 5.4 pkts/min | ~1 s |
| SF9 | 213 ms | 2.8 pkts/min | **~2.8 s** |
| SF10 | 401 ms | 1.5 pkts/min | ~1.5 s |

⇒ **Continuous/sustained 1 pkt/s is non-compliant at every SF** (minimum
airtime alone is 5.9 % duty — a hard floor). 1 pkt/s operates only inside
duty-limited acquisition windows (length = open question Q10).

**Sustained-1-pkt/s alternatives (same Wio-S3 board, per strategy §3a):**

| Option | basis | 26 B packet airtime | duty limit | range vs SF9 |
|--------|-------|--------------------|------------|--------------|
| GFSK 9.6 kbps, 902–928 MHz | §15.247(c), 50 mW TPO | ≈ 30 ms | **none** (not spread spectrum) | ~10–30× less |
| WiFi 2.4 GHz | §15.407 | negligible | none (PSD-limited) | needs base/infra |

GFSK sensitivity ≈ −115 dBm vs −147 dBm (SF9) → ~32 dB of lost link margin
≈ 10–30× range. If the client needs sustained 1 pkt/s *and* km-scale range,
no Wio-S3 path satisfies both.


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
terrain/obstruction/shadowing, not FSPL. Site survey (step 6) replaces this
table. SF9 default leaves ≥ 57 dB margin to 10 km in clear sight.

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
| Q9 (sustained vs windowed) | if sustained → physical-layer change (§3a: GFSK/WiFi), not LoRa SF |
| Q10 window length (if windowed) | updates E (autonomy) and C (compliance envelope) |
| `rt` needs negatives / > 65.535 | frame → 30 B; SF10 airtime → ~460 ms (still fits 1 s) |
| BW raised to 250 kHz | airtimes halve → more 1-pkt/s time per minute, less range |
| Measured Wio-S3/P1-Pro currents | replaces E's assumptions with data |
