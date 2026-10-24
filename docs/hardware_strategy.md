# Hardware Strategy

*Draft v0.1 — items marked (VERIFY) should be confirmed against the Seeed
datasheets in `assets/` before ordering.*

## 1. System Overview

A field-deployable LoRa mesh radio node:

- **Radio/compute:** 2× Seeed Studio **Wio-S3 Wireless LoRa** (XIAO ESP32-S3
  + onboard LoRa, 868/915 MHz, SMA connector) — one for the deployed node,
  one for bench/inter-node testing. (VERIFY: confirm exact LoRa radio part
  number and band selection from the product PDF; US deployment ⇒ US915.)
- **GNSS:** 1× **L76K GNSS Module for XIAO** (VERIFY: chip — likely
  ATGM336H-class, connects via XIAO expansion port over UART/I²C) provides
  position for node telemetry and mesh positioning.
- **2.4 GHz RF:** 2.4 GHz FPC Antenna A-02 (1.6 dBi) ×2 for the ESP32-S3
  WiFi/BT radio — used for one-time external configuration, firmware
  flashing, and diagnostics.
- **Power:** solar panel → LiFePO4 charge controller → 4× 18650 LiFePO4
  pack (nominal 12.8 V), down-converted to 5 V for the Wio-S3.
- **Enclosure:** 2" PVC Sch 40 pipe with end caps, PVC cemented; 1"–1.5"
  pipe segments for mounting. Antenna penetrations gasketed for weather
  sealing.

## 2. Firmware Direction

- Target the **Meshtastic** firmware for the Wio-S3 (ESP32-S3 + SX126x is a
  supported board family). This gives mesh networking, LoRa telemetry, and a
  phone/web client for free, and matches the "adapt for Meshtastic" note in
  the requirements.
- Custom/forked firmware only where needed (e.g., GNSS feed wiring, power
  management hooks, deep-sleep schedules).
- Provisioning flow: WiFi/BT + FPC antenna for config at deploy time;
  node then runs on LoRa + solar + GNSS.

## 3. Power Subsystem (needs discussion)

- 4× 18650 **LiFePO4** (3.2 V/cell) → 4S pack, 12.8 V nominal, 14.6 V max
  charge.
- Solar panel Voc must stay under the charge controller's input limit;
  controller must support 4S LiFePO4 (or be an MPPT 12 V type). Options:
  - Adapt a **SenseCAP Solar Node P1-Pro** (has solar + LiFePO4 + charge
    mgmt, Meshtastic-friendly) — fastest path, but battery is fixed-size.
  - Small-footprint panel + dedicated 4S LiFePO4 BMS/charge controller from
    Seeed or equivalent — more custom, more cell capacity.
- (OPEN) Panel W rating vs. average sun hours on site; target runtime
  without sun (days) — these two numbers drive panel size and BMS choice.

## 4. Mechanical

- 2" Sch 40 pipe body (≈ 60 mm ID) fits the pack + Wio-S3 stack; end caps
  cemented with PVC cement, drilled for SMA antenna and (optionally) GNSS
  feed-through or top-hat GNSS mount.
- 1"–1.5" pipe fittings for pole/mast mounting.
- Sealing: gaskets at caps, RTV at antenna glands.

## 5. Open Questions

1. Solar panel peak voltage / wattage — site-specific; needs the two numbers
   above (panel W, sunless-autonomy days).
2. Charge controller: off-the-shelf 4S LiFePO4 BMS/MPPT vs. SenseCAP P1-Pro
   adaptation.
3. Wio-S3 LoRa radio part + band (US915) confirmation.
4. L76K interface pins on the Wio-S3 (shared UART?) — check datasheet.
5. Toolchain: Arduino, PlatformIO, or ESP-IDF (Meshtastic builds via
   PlatformIO by default — leans PlatformIO).

## 6. Procurement Note

Order from Seeed Studio US warehouse only; confirm stock at order time to
avoid delays (per requirements). A partial kit will be mailed by the client
for evaluation — reserve bench testing until it arrives.
