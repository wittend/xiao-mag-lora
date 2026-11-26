# P1 / P1-Pro Teardown & Integration Checklist

*Phase 1 prep — resolves strategy open question Q5 (P1 ↔ node power/
interface wiring) and fixes the scope of the xiao carrier design.
Do this **before** locking the carrier schematic.*

## 0. Context & purpose

The SenseCAP Solar Node P1(-Pro) is the **power subsystem** of the field
node: solar charge management + 4× 18650 LiFePO4 (strategy §4). Our data
link is a separate XIAO ESP32-S3 (Sense) + Wio-SX1262 + L76K stack. This
teardown answers, with measurements, exactly how the two connect.

**Reference docs in `assets/`:**
- `SenseCAP Solar Node P1 for Meshtastic - 114993643.pdf` (spec sheet)
- `PCA-Prime-Pro-top-hi-res-1.webp` (top-side reference photo)
- Seeed wiki: "SenseCAP Solar Node Consumption Test and Battery Life
  Calculation" + `SenseCAP_Solar_Node_instruction_diagram` (links in the
  spec's Resource Links section)

**Known from the P1 spec sheet (SKU 114993643):**
- Main controller: XIAO nRF52840 Plus · LoRa: Wio-SX1262 (SX1262,
  TXOP 22 dBm @ 862–930 MHz)
- GPS: user-added XIAO L76K · Battery: user-added 4× 18650 (button-top)
- Solar: 5 W panel integrated · Interfaces: Grove ×1 (I²C/GPIO), USB-C
  debug (spec'd **5 V 1 A**)
- Buttons: power on/off, reset, user-defined · LEDs: charge red/green,
  solar yellow, mesh blue, user white
- Size: **191.2 × 201.2 × 42.1 mm** · hoop bracket 70–100 mm

## 1. Pre-flight

- [ ] ⚠️ **Identify the actual unit: P1 (114993643) or P1-Pro (p-6412)?**
  The strategy/BOM says **P1-Pro**, but only the **P1** spec sheet is in
  `assets/`. Record the exact SKU/label: `____________`
- [ ] Multimeter (voltage + current ranges), small screwdriver set,
  compressed air, isopropyl, ESD strap/mat, camera.
- [ ] LiFePO4 safety: cells removed or disconnected for board-level work;
  never short pack terminals; note pack orientation/labels.
- [ ] **Photograph and label every connector before unplugging anything**
  (save to `assets/teardown/` with a name convention, e.g. `01-top.jpg`).
- [ ] Note firmware version / Meshtastic settings as shipped (screenshots).

## 2. THE XIAO question — determines carrier scope (do first)

This is the highest-value part of the teardown. If the Wio-SX1262 wiring
runs *through the XIAO header*, a XIAO ESP32-S3 (Sense) swap inherits all
RF wiring and the carrier shrinks to power + sensors + GNSS + antenna.

- [ ] Is the XIAO nRF52840 Plus **socketed** or soldered? `____________`
- [ ] How is the **Wio-SX1262** connected — to the XIAO expansion header,
      or to dedicated pads on the P1 board? `____________`
- [ ] Trace/label **every signal** the P1 PCB presents at the XIAO header:
      VBUS, GND, UART (L76K), SPI (SX1262), Grove, LEDs, buttons.
      (Photo of header pins + net names if visible: `__`)
- [ ] Does a **XIAO ESP32-S3 (Sense)** physically fit the same header
      (keying, orientation, standoffs)?
- [ ] If the L76K mounts via the XIAO header too: which pins (confirm
      D6/D7 UART @ 9600 assumption on the *P1's* header)?
- [ ] **Decision input D1:** swap-in (ESP32-S3 takes over the P1 XIAO
      slot) vs. separate carrier stack (P1 supplies power only).

## 3. Power rails

- [ ] **5 V rail:** where is it available (USB-C port? XIAO VBUS? test
      point?) with the unit on battery power? Voltage: `____ V` at
      `____ mA` loaded with a dummy load (e.g. 10 Ω for ~500 mA).
- [ ] Is USB-C **input only**, or does it pass battery 5 V out?
- [ ] **Battery pack rail:** pack voltage accessible where? (test point /
      balance header / cells). 4S LiFePO4 nominal 12.8 V — measured:
      `____ V`. Protection board present (OVP/UVP/OCF FETs)? `____`
- [ ] **Current headroom for our add-on stack** (ESP32-S3 + SX1262 TX
      bursts @ 17–22 dBm + L76K + RM3100 ≈ `____` mA est.): measure max
      sustainable current on the chosen rail without brownout.
- [ ] Any power sequencing/enable signals (GPIO-controlled LDOs)?

## 4. Battery state-of-charge access (node health, REQ-013)

- [ ] Fuel-gauge IC present? Part + I²C address: `____________`
- [ ] If none: voltage-divider or ADC access to pack voltage? `____`
- [ ] Does the stock firmware report battery %/SoC over USB (Meshtastic
      telemetry)? `____`
- [ ] **Decision input D2:** SoC source for the carrier (fuel-gauge I²C
      vs. carrier-side divider on pack rail vs. charge-LED proxy).

## 5. Solar & charge management

- [ ] Charge controller IC (from schematic photo): `____________`
- [ ] Charge current setting: `____ A` · input voltage tolerance
      (min/max): `____`
- [ ] **Conflict flag:** BOM specifies a **16–20 W external panel**, but
      the P1 integrates a **5 W** panel. Can the P1's solar input accept
      a larger panel (voltage/current within the charge IC's limits), or
      would an external panel need to bypass it and charge the pack
      directly? `____________`
- [ ] Charge LED behavior confirmed (red = charging, green = charged).
- [ ] **Decision input D3:** solar configuration (5 W built-in vs. 16–20 W
      external vs. both) — revisit autonomy sizing in
      `docs/calculations.md` accordingly.

## 6. Radio / nRF52840 isolation

- [ ] Power switch scope: whole unit or per-rail? `____________`
- [ ] Idle power of the nRF52840 XIAO **with Meshtastic off/asleep**:
      `____ mA`. Is a firmware sleep mode available, or must it be
      powered down / removed?
- [ ] Confirm the P1's SX1262 stays **off** while our stack transmits
      (or is absent after swap) — no co-location per its FCC 15.21 note.
- [ ] **Decision input D4:** nRF52840 fate — kept-asleep / powered-down /
      physically removed.

## 7. Mechanical & enclosure — **conflict flag**

- [ ] **The P1 (191 × 201 × 42 mm) does NOT fit the 2" Sch 40 PVC**
      (≈52.5 mm ID) assumed in strategy §5. Enclosure options:
      - [ ] A: P1 housing **is** the deployment enclosure; carrier board
            mounts inside the P1 case (near the battery bay); solar panel
            on top; antenna through the existing RP-SMA bracket.
      - [ ] B: Larger custom box housing P1 + carrier; 2" pipe only for
            pole/mast.
      - [ ] C: Other: `____________`
- [ ] Measure internal free space where the carrier would sit:
      `____ × ____ × ____ mm`.
- [ ] Hoop bracket (70–100 mm) vs. BOM's 1"–1.5" pipe mounting — confirm
      pole diameter and adapter.
- [ ] **Decision input D5:** enclosure concept + carrier max board size.

## 8. Power consumption log (update `docs/calculations.md`)

| State | Current (mA) | Voltage (V) | Notes |
|---|---|---|---|
| P1 idle, Meshtastic off | | | |
| P1 idle, Meshtastic asleep | | | |
| P1 charging (solar/USB) | | | |
| Our stack: idle (no TX) | | | |
| Our stack: TX burst @ 17 dBm | | | |
| Our stack: TX burst @ max | | | |

## 9. Decision log (fill at the end — then update the docs)

| # | Question | Decision | Date |
|---|----------|----------|------|
| D0 | P1 vs P1-Pro identity | | |
| D1 | Carrier scope (D2 above) | | |
| D2 | SoC source | | |
| D3 | Solar configuration | | |
| D4 | nRF52840 fate | | |
| D5 | Enclosure concept + board size | | |

**Follow-ups when decisions land:** update `docs/hardware_strategy.md`
(close Q5), `docs/bom.md` (panel item, enclosure items), umbrella
`docs/pcb_design_plan.md` (carrier scope §2), and `CURRENT_STATUS.md`.
