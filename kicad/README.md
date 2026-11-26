# xiao-carrier — KiCad project

Phase 1 board of the PCB plan (umbrella `docs/pcb_design_plan.md`): the
carrier for the **XIAO ESP32-S3 (Sense) + Wio-SX1262 + L76K** field node,
fed by the SenseCAP Solar Node P1(-Pro) power subsystem.

**Prerequisite:** run the P1 teardown first
(`../docs/p1pro_teardown_checklist.md`). Its decisions **D1** (carrier
scope — swap-in vs separate stack) and **D5** (board size / enclosure)
can shrink or reshape this design before schematic capture locks.

## Files

| File | Purpose |
|---|---|
| `xiao-carrier.kicad_pro` | Project settings (seeded from Dave's `XiaoHalowMag` KiCad 10 stub) |
| `xiao-carrier.kicad_sch` | Main sheet — references the 5 sub-sheets |
| `power.kicad_sch` | P1(-Pro) input, 5 V/3.3 V rails, battery sense, protection |
| `compute-rf.kicad_sch` | XIAO ESP32-S3 (Sense), Wio-SX1262, IPEX→SMA, RF ESD |
| `gnss.kicad_sch` | L76K (clock-only, UART D6/D7 @ 9600), GNSS antenna supply |
| `sensors.kicad_sch` | RM3100-CB + MCP9808 (I²C D4/D5), pull-ups, ESD |
| `status.kicad_sch` | User LED, test points, provisioning hooks |
| `xiao-carrier.kicad_pcb` | Board (empty — no outline yet) |
| `fp-lib-table` / `sym-lib-table` | Project library tables (below) |

KiCad **10.0** file formats (sch 20260306 / pcb 20260206). `.kicad_prl`
(per-user view settings) is gitignored.

## Library tables

- **RM3100-CB** — repo assets: `../assets/RM3100-CB.pretty`
  (module + pads-only footprints) and `../assets/RM3100-CB.kicad_sym`.
- **Seeed_Studio_XIAO_Series** — symbols from the OPL library checkout.
  ⚠️ **Machine-local absolute path** into
  `~/Projects/KiCad-dev/OPL_Kicad_Library_(2026-06-23)/…` — update or
  vendor the file into the repo if that checkout moves.

## Footprint / symbol sources & gaps

| Component | Status | Source |
|---|---|---|
| XIAO ESP32-S3 (Sense) SMD | available | OPL `Seeed Studio XIAO Series Library/XIAO-ESP32-S3-SMD.kicad_mod` (confirm the footprint matches the *Sense* variant with the mic + IMU) |
| Wio-SX1262 (for XIAO) | available | OPL `Projects/Seeed Studio Wio SX1262 for XIAO ESP32S3 (Project)/` — complete reference project (sch + pcb + pdf); also `Seeed Studio Wio Series/Wio-SX1262/` (ProLib `.elibz` with `.esym` + `.efoo`, 3D step). `assets/Wio-SX1262_3D_File.zip` (in `../wio-mag-lora/assets/`) has the 3D model |
| L76K (for XIAO) | **GAP** | Not in OPL. Options: Seeed wiki KiCad file for SKU 109100021, or author a 14-pin XIAO add-on footprint (1.27 mm) from the datasheet in `../assets/` |
| RM3100-CB | available | Repo assets (see tables) |
| MCP9808 (TOF-20) | available | KiCad stock `PackageTOFPad` lib |
| IPEX / SMA / JST / XT30 / USB-C | mixed | KiCad stock `Connector*` libs + vendor footprints as needed |

## Validate from CLI

```sh
kicad-cli sch export pdf -o /tmp/xiao-carrier.pdf xiao-carrier.kicad_sch
kicad-cli sch erc xiao-carrier.kicad_sch
kicad-cli pcb drc xiao-carrier.kicad_pcb   # expect: "no edges found on Edge.Cuts" until the outline is drawn
```

## Pin map reference

`../src/firmware/include/xiao_pins.hpp` — SPI D8/D9/D10 (confirmed);
CS/DIO1/BUSY/RST D11/D12/D1/D0 (**[BENCH-VERIFY]**); GNSS UART D6/D7 @
9600; I²C D4/D5; LED GPIO21.
