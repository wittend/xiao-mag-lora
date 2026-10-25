# Payload Format (wire spec)

*Decided 2026-10-24: binary encoding, 3-decimal fixed-point fields.
Replaces the JSON example as the wire format; JSON remains the host-side
representation (decoder output, SPA, logs).*

## Frame layout — 26 bytes, little-endian

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0      | 2    | magic | `0xA5 0x5A` (fixed; receiver discards frames without it) |
| 2      | 2    | seq   | uint16 sequence, wraps at 65535; gaps = lost packets |
| 4      | 1    | type  | `0x01` data · `0x02` ack · `0x03` keepalive · `0x04` config |
| 5      | 1    | rsvd  | reserved, transmit `0x00` |
| 6      | 2    | crc   | CCITT-FALSE (0xFFFF init) over bytes 0–15 (header minus crc + payload) |
| 8      | 4    | ts    | uint32, Unix epoch seconds (UTC) |
| 12     | 2    | rt    | uint16, value × 1000 → range **0 – 65.535**, 3-decimal |
| 14     | 4    | x     | int32, value × 1000 → range **±21,474,836.47**, 3-decimal |
| 18     | 4    | y     | int32, value × 1000 → range **±21,474,836.47**, 3-decimal |
| 22     | 4    | z     | int32, value × 1000 → range **±21,474,836.47**, 3-decimal |

## Encoding rules

- Fixed-point scale **1000** on `rt`, `x`, `y`, `z` (3 decimals, as specified).
- `ts` is integer epoch seconds (UTC); no sub-second component.
- All multi-byte integers little-endian.
- CRC covers everything except the crc field itself; receiver drops on
  mismatch and counts a `crc_err` link-health statistic.
- `seq` advances once per second (per direction) during acquisition
  windows; receivers compute loss rate from gaps.
- ACK frame (`type 0x02`): same header, 18 B payload echoing the acked `seq`
  in the `ts` field (4 B), remainder zeroed. Used for config frames only.

## Worked example

JSON (host-side):
```json
{ "ts":"26 Oct 2025 14:20:00", "rt":23.125, "x":12345.678,
  "y":-234.500, "z":987.001 }
```
Wire frame (26 B, verified little-endian; `ts` = 1761488400 = 26 Oct 2025
14:20:00 UTC):
```
A5 5A 01 00 01 00 F6 4C | 10 2E FE 68 | 55 5A | 4E 61 BC 00 | FC 6B FC FF | 79 0F 0F 00
```


## Airtime (125 kHz, 8-symbol preamble, CRC on)

| SF  | frame | airtime | note        |
|-----|-------|---------|---------------|
| SF7 | 26 B  | 59 ms   | default     |
| SF8 | 26 B  | 111 ms  | yes |
| SF9 | 26 B  | 213 ms  | range-only  |
| SF10| 26 B  | 401 ms  | yes (long range) |

SF7 (or SF6/250 kHz) is the default — the fastest SF that links at the ≤1 km
LoS target; SF8–SF9 only if a site needs the range. Faster SF maximizes flush
rate under the FCC TX duty limit (strategy §3a).

## Assumptions to confirm (fold into acceptance test)

1. `rt` is **unsigned 0 – 65.535** — if `rt` can be negative or exceed
   65.535, it moves to int32 (frame grows to 30 B; SF10 airtime 460 ms —
   still fits).
2. `x`, `y`, `z` never exceed ±21,474,836.47 (int32 × 1000).
3. Second-resolution `ts` is sufficient (no sub-second needed).
