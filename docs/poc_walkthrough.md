# PoC bench walkthrough — proving the P2P LoRa link

Goal: get **two Kit nodes** (XIAO ESP32-S3 + Wio-SX1262) talking to each other on
US915, and watch the received frames turn into JSON on the Linux host. This is the
smallest possible proof of the "delay-tolerant P2P data link" before adding GNSS
clock discipline, real sensors, and the P1-Pro power/enclosure.

Both nodes run the **same firmware**. Every node: samples ~1 Hz into a backlog,
flushes it in bursts (up to 10 frames every 5 s), and forwards any frame it
**receives** to its USB serial port. So you attach your USB + decoder to **one**
node (call it **B**) and watch the **other** node's (**A's**) frames arrive. You
never see B's own TX on B's serial — only what B receives from A.

> The one hardware unknown at first flash is the 4 SX1262 **control pins**
> (CS/DIO1/BUSY/RST). SPI is confirmed. The firmware tells you immediately if they
> are wrong — see [the pin gate](#if-begin-fails-the-pin-gate) below.

---

## What you need

- 2× Kit (XIAO ESP32-S3 + Wio-SX1262, SKU 113010003), each with a LoRa antenna on the IPEX
- 2× **USB-C cables that carry data** (some are charge-only)
- A Linux box with `pio` on PATH (PlatformIO Core) and `python3`
- `pyserial` for live decode: `python3 -m pip install --user pyserial` (optional if you capture to a file first)

---

## 1) Build

```sh
cd src/firmware
pio run -e wio_s3_lora          # -> SUCCESS, produces .pio/build/wio_s3_lora/firmware.bin
```

(Optional sanity, no hardware: `./test/run_host_test.sh` → 31/31.)

## 2) Flash node A, then node B

Plug one node in over USB-C. Find its port (Linux usually auto-creates it):

```sh
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null        # or: pio device list
```

Flash to that port (be explicit to avoid the wrong port):

```sh
pio run -e wio_s3_lora -t upload -p /dev/ttyACM0     # node A
# unplug A, plug in B
pio run -e wio_s3_lora -t upload -p /dev/ttyACM0     # node B (its port; may differ)
```

The XIAO ESP32-S3 auto-resets into the bootloader on the USB DTR pulse; if an upload
hangs in "A fatal error occurred: Timed out waiting for packet header", hold the
**BOOT** (GPIO0) button while pressing reset, or add `--upload-baud 921600`.

## 3) Verify the radio initialized (per node)

Open a serial monitor at 115200 (any of these):

```sh
pio device monitor -p /dev/ttyACM0 -b 115200 -e wio_s3_lora      # press Ctrl-] to exit
```

On boot you must see:

```
[xiao-mag-lora] boot
[radio] SX1262 pins: SCK=7 MOSI=9 MISO=8 CS=42 DIO1=41 BUSY=2 RST=1
[radio] SX1262 begin: OK
[node] sf=9 freq=915.9 MHz store=3600
[hb] tx=... rx=... lost=... crc_err=... backlog=... clock=local
```

**`begin: OK` = the SX1262 answered over SPI — the pin map is right and the radio
is live.** If you see `begin: FAIL (RadioLib error N)`, stop and do the pin gate below
before anything else.

## 4) Watch the link (run the decoder on node B)

Put both nodes in range (a meter or two, antennas not jammed by a hand), powered.
Attach USB to **node B** and decode its serial stream live:

```sh
python3 src/host/decode.py -p /dev/ttyACM1 -v
```

`-v` echoes the node's ASCII heartbeat/status to **stderr**; the decoded JSON frames
go to **stdout**. After ~5 s (the first flush burst) you should start getting lines like:

```json
{"type": "data", "seq": 0, "ts": 1761488400, "ts_iso": "2025-10-26T14:20:00Z", "rt": 23.125, "x": 12345.678, "y": -234.5, "z": 987.001}
```

in bursts. On the stderr heartbeat from node B, `rx` and `lost` are the counters
that matter: **`rx` climbing and `lost` staying ~0 is the link working.** The sample
values are the dummy sensor's drifting sine waves (rt ≈ 23.0–23.5 °C, x/y/z smooth).

To log to a file instead (or in addition):

```sh
python3 src/host/decode.py -p /dev/ttyACM1 2>/dev/null | tee poc.log
```

## 5) Success criteria

- Both nodes print `begin: OK`.
- Node B's stdout yields JSON frames; `seq` increments; values are the A-side sine drift.
- Node B's `[hb] ... rx=N lost=0` with N growing over time.
- Pull the nodes apart (or move one behind a wall) and watch `lost` rise / frames thin
  out — that is the link budget behaving as expected, and confirms gap detection.

---

## If `begin` fails (the pin gate)

`begin: FAIL (RadioLib error N)` means the SX1262 did not answer over SPI. The SPI
pins (D8/D9/D10) are fixed and correct; the **4 control pins are the thing to fix**
(`include/xiao_pins.hpp`):

```cpp
constexpr int kSx1262Cs   = 42;   // XIAO D11  chip select (NSS) [BENCH-VERIFY]
constexpr int kSx1262Dio1 = 41;   // XIAO D12  DIO1 / IRQ        [BENCH-VERIFY]
constexpr int kSx1262Busy = 2;    // XIAO D1   BUSY              [BENCH-VERIFY]
constexpr int kSx1262Rst  = 1;    // XIAO D0   NRESET (active-low) [BENCH-VERIFY]
```

To get the real values, read them off the board/schematic:
- **Silkscreen:** the Wio-SX1262-for-XIAO board is stacked on the XIAO headers; the
  traces from the SX1262's NSS/DIO1/BUSY/NRESET pins land on specific XIAO D-pins.
- **Schematic:** last page of `assets/Wio-SX1262 for XIAO -113010003.pdf` (image).

Map each to the XIAO ESP32-S3 GPIO using (D0=1, D1=2, D2=3, D3=4, D4=5, D5=6, D6=43,
D7=44, D8=7, D9=8, D10=9, D11=42, D12=41 — see `include/xiao_pins.hpp`). Edit the four
constants, rebuild, reflash:

```sh
cd src/firmware
pio run -e wio_s3_lora -t upload -p /dev/ttyACM0
```

RadioLib error codes (from `begin()`): `ERR_RADIO_NOT_FOUND`(104) is the classic
"wrong CS/BUSY" signature; SPI init errors also surface here. If it's still failing
with the correct pins, check the antenna is seated, the IPEX is not kinked, and try a
clean power cycle.

## Troubleshooting

- **`begin: OK` but no frames arrive at B:** both nodes must share SF (9) and
  frequency (915.9 MHz) — they do by default. Check A is actually flushing (its
  `[hb] tx=` climbing). Check B's `rx=`. Antennas on, nodes in range, no big metal
  between them.
- **Frames then thin out / `lost` climbs:** expected with distance/obstacles; note the
  range where SF9 still delivers, and try SF10 (lower `kDefaultSf`) for reach if needed.
- **Decoder prints "0 frames":** you're likely on the node's *own* TX side (no RX
  happening) or the wrong port. Watch the *other* node's port. Confirm with `-v` that
  you see its `[hb]` lines.
- **Serial garbled / baud:** USB-CDC is a byte stream; `decode.py` resyncs on the
  `0xA5 0x5A` magic regardless, so a little noise won't matter.
- **RadioLib "USB CDC might stop on first sleep" warning:** harmless on the bench
  (no sleep, powered from USB). Note it for the battery/sleep build on the P1-Pro.

---

## Capture + replay (no live decode needed)

If you'd rather not depend on `pyserial`, capture raw bytes then decode offline:

```sh
# capture ~60 s from node B (Ctrl-C to stop):
timeout 60 cat /dev/ttyACM1 > capture.bin
# decode:
python3 src/host/decode.py -f capture.bin -v
```

The decoder is a pure byte-stream scanner (resync on magic, CRC-verify each 26-byte
frame), so it works identically on a live port, a file, or stdin.
