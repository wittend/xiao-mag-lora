# xiao-mag-lora

Solar-powered, GNSS-enabled LoRa mesh radio node built on Seeed Studio
XIAO ESP32-S3 (Wio-S3 Wireless LoRa) hardware, with a Linux host-side control
stack and a single-page web app for testing and monitoring.

> Note: the requirements file references a placeholder folder name
> `xiao-mag-halo`; the actual repository is `xiao-mag-lora`. This is the
> canonical project root.

## Project Workflow (from `requirements/`)

1. Write Requirements — `requirements/xaio-mag-ora_requirements.md`
2. Generate a Hardware Strategy — `docs/hardware_strategy.md`
3. Create an Overall Bill of Materials (BOM) — `docs/bom.md`
4. Set Up Project Repository & Environment — this folder + `software_requirements.md`
5. Develop Hardware & Software — `kicad/`, `src/`
6. Integration, Deployment, and Evaluation
7. Client-side (Linux host) control software + SPA web app

## Repository Layout

| Path       | Purpose                                                      |
| ---------- | ------------------------------------------------------------ |
| `assets/`  | Datasheets, reference PDFs, images, board renderings         |
| `docs/`    | Strategy, BOM, design notes, test plans                      |
| `src/`     | Firmware + client software source (per-language subfolders)  |
| `dist/`    | Build artifacts and packaged releases                        |
| `kicad/`   | KiCad hardware design (carrier/support board)                |
| `requirements/` | Original requirements source of truth                    |

## Status

- [x] Requirements written
- [x] Repository structure initialized
- [x] Hardware strategy draft
- [x] Bill of Materials draft
- [x] Toolkit selection — PlatformIO (confirmed)
- [ ] Firmware build & test
- [ ] Client control software
- [ ] SPA web app
- [ ] Integration / deployment
