# Bill of Materials

*Draft v0.2 — prices/stock unverified. Check Seeed US warehouse stock before
ordering. (VERIFY) = spec to confirm against datasheet.*

## Radio & Compute

| # | Item | Qty | Link / Part | Notes |
|---|------|-----|-------------|-------|
| 1 | Seeed Studio Wio-S3 Wireless LoRa Module | 2 | seeedstudio.com p-6832 | XIAO ESP32-S3 + LoRa 868/915, SMA |
| 2 | 2.4 GHz FPC Antenna A-02 (1.6 dBi) for XIAO ESP32-S3 | 2 | seeedstudio.com p-6440 | WiFi/BT config antenna |
| 3 | L76K GNSS Module for Seeed XIAO | 1 | Seeed (datasheet 109100021.pdf) | (VERIFY) chip + interface |

## Power

| # | Item | Qty | Notes |
|---|------|-----|-------|
| 4 | Solar panel 16–20 W class, ~18 V peak | 1 | Sized for ≥2–3 days sunless; confirm Vmp/Voc from datasheet |
| 5 | SenseCAP Solar Node P1-Pro (4× 18650 LiFePO4, solar charge mgmt) | 1 | Power subsystem — decided; includes cells |
| 6 | 12.8 V → 5 V step-down (buck) converter | 1 | For Wio-S3 VBUS; sized for LoRa TX peaks |
| 7 | Wires, connectors, fuses, XT30/JST as needed | 1 set | |

## Enclosure & Mounting

| # | Item | Qty | Notes |
|---|------|-----|-------|
| 8 | 2" PVC Sch 40 pipe (cut length TBD) | 1 | Main housing |
| 9 | 2" PVC end caps | 2 | Cemented + gasketed |
| 10 | PVC cement | 1 | |
| 11 | 1"–1.5" PVC Sch 40 pipe + end caps (mounting) | 1 set | Pole/mast mount |
| 12 | Antenna feed-through glands + RTV | 2+ | SMA penetration sealing |

## Consumables / Bench

| # | Item | Qty | Notes |
|---|------|-----|-------|
| 13 | SMA LoRa antenna 915 MHz (or whip) | 2 | External LoRa antenna (added — not in original list) |
| 14 | USB-C cables (Wio-S3 uses USB-C) | 2 | Flashing/diagnostics |
| 15 | RTV silicone, gasket material | 1 set | Weather sealing |

**Total: 15 line items** (two-node pair + spares TBD).
