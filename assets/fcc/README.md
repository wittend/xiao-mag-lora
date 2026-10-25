# FCC Compliance Basis (902–928 MHz, US915 LoRa)

Public-domain FCC filings for an **SX1262-class US915 LoRa module** (the RAK
Wireless RAK3172 module, as used in the RAK7258 gateway — same radio family as
the Wio-S3 LoRa). Kept in-repo as the authoritative evidence that our radio path
is an *authorized configuration* under 47 CFR §15.247. Source: RAK Wireless
compliance downloads (mirroring the FCC OET grant, FCC ID 2AF6B-RAK7258 /
module 2AF6B-RAK2247).

## Files
- `RAK7258_FCC_grant_2AF6B.pdf` — TCB Grant of Equipment Authorization, FCC ID
  **2AF6B-RAK7258** (Shenzhen Rakwireless Technology). States: Equipment Class
  "Digital Transmission System"; FCC Rule Part **15C** (Part 15 Subpart C =
  §15.247); LoRa range **923.3–927.5 MHz**; 0.019 W (≈13 dBm) conducted.
- `RAK7258_FCC_15.247_LoRa_test_report.pdf` — the §15.247 LoRa (chirp
  spread-spectrum) RF test report (Bay Area Compliance Laboratories,
  report RSZ190308005-00C).

## What the grant + test report establish (verified 2026-10-24)
- **Modulation: chirp-based spread-spectrum (CSS)**; band **923.3–927.5 MHz**
  (US915 LoRa uplink channels); rule basis **§15.247 (Part 15 Subpart C)**.
- Tested and **Compliant** with:
  - §15.247(a)(2) — 6 dB Emission Bandwidth
  - §15.247(b)(3) — Maximum Conducted Output Power
  - §15.247(d) — 100 kHz band-edge / out-of-band
  - §15.247(e) — Power Spectral Density
  - plus §15.205/§15.209 spurious, §15.207(a) line-conducted emissions, and MPE
    (§15.247(i) / §1.1310 / §2.1091).
- Emissions were measured at **100 % duty** (continuous TX) — i.e., there is
  **no TX duty-cycle limit** for this class (the "1 %" previously assumed does
  not exist).
- Antenna: certified with a **≤ 2.0 dBi** external antenna. Per §15.247(b), any
  directional antenna > 6 dBi requires a conducted-power backoff equal to
  (gain − 6 dBi) dB — **there is no 902–928 MHz point-to-point exemption** (that
  exemption exists only for 2.4 / 5.8 GHz).

## How this closes open question Q9
Q9 asked how CSS LoRa at 125 kHz maps to §15.247(a)(2)'s 500 kHz 6-dB-bandwidth
minimum. A commercial SX1262-class US915 LoRa module is **explicitly tested and
found Compliant** with §15.247(a)(2) under the §15.247 (Part 15C) basis, at 100 %
duty. The Wio-S3 is itself FCC/CE-certified (per Seeed) with a 2 dBi dipole.

→ **Q9 CLOSED:** our radio path is an authorized configuration. Operate per the
US915 LoRa channel / data-rate plan (LoRa Alliance RP002 regional parameters;
see `docs/payload_format.md`).
