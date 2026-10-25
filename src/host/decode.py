#!/usr/bin/env python3
"""
xiao-mag-lora host decoder: 26-byte LoRa air frame -> JSON.

The node forwards each CRC-valid air frame to the host as its raw 26 bytes over
USB serial, interleaved with ASCII heartbeat/status lines. This tool resynchronizes
on the 0xA5 0x5A magic, extracts 26-byte frames, re-validates the CCITT-FALSE CRC,
and prints one JSON object per valid frame (one per line). The ASCII lines never
contain the 0xA5 byte, so they are filtered out automatically (echoed to stderr
with -v).

Frame layout (26 bytes, little-endian) -- source of truth: docs/payload_format.md
  off  len  field
  0    2   magic 0xA5 0x5A
  2    2   seq   (u16)
  4    1   type  (u8)   0x01=data 0x02=ack 0x03=keepalive 0x04=config
  5    1   rsvd  (u8)
  6    2   crc   (u16)  CCITT-FALSE over bytes 0-5 then 8-25 (all but this field)
  8    4   ts    (u32)  UTC epoch seconds (GNSS-disciplined)
  12   2   rt    (u16)  record temperature x1000  (deg C, 0..65.535)
  14   4   x     (i32)  magnetometer X x1000
  18   4   y     (i32)  magnetometer Y x1000
  22   4   z     (i32)  magnetometer Z x1000

Usage:
  python3 src/host/decode.py --selftest            # prove it works (no hardware)
  python3 src/host/decode.py -p /dev/ttyACM0 -v    # live USB-serial (needs pyserial)
  python3 src/host/decode.py -f capture.bin        # from a saved capture
  cat capture.bin | python3 src/host/decode.py     # from stdin

Install pyserial for live serial:  python3 -m pip install --user pyserial
"""
import argparse
import datetime
import json
import struct
import sys

MAGIC = b"\xa5\x5a"
FRAME_LEN = 26
TYPE_NAMES = {0x01: "data", 0x02: "ack", 0x03: "keepalive", 0x04: "config"}


def crc16_ccitt_false_run(crc, data):
    """CCITT-FALSE running form (poly 0x1021, no reflection, no xor-out)."""
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if (crc & 0x8000) else (crc << 1)
            crc &= 0xFFFF
    return crc


def frame_crc(buf):
    # Bytes 0-5, then 8-25 (everything except the 2-byte crc field at 6-7).
    return crc16_ccitt_false_run(0xFFFF, buf[0:6] + buf[8:26])


def frame_is_valid(buf):
    return len(buf) >= FRAME_LEN and buf[0:2] == MAGIC and struct.unpack_from("<H", buf, 6)[0] == frame_crc(buf)


def scale(v, n=3):
    return round(v / (10.0 ** n), n)


def iso(ts):
    try:
        return datetime.datetime.fromtimestamp(ts, datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    except (OverflowError, OSError, ValueError):
        return None


def frame_to_json(buf):
    seq = struct.unpack_from("<H", buf, 2)[0]
    ftype = buf[4]
    ts = struct.unpack_from("<I", buf, 8)[0]
    rt = struct.unpack_from("<H", buf, 12)[0]
    x = struct.unpack_from("<i", buf, 14)[0]
    y = struct.unpack_from("<i", buf, 18)[0]
    z = struct.unpack_from("<i", buf, 22)[0]
    return {
        "type": TYPE_NAMES.get(ftype, ftype),
        "seq": seq,
        "ts": ts,
        "ts_iso": iso(ts),
        "rt": scale(rt),
        "x": scale(x),
        "y": scale(y),
        "z": scale(z),
    }


def scan(chunks, emit, verbose_lines=None, max_frames=None):
    """
    Consume an iterable of byte chunks, yielding decoded frames.

    `emit(frame_dict, raw_26_bytes)` is called for each valid frame. `verbose_lines`
    (optional callable) receives ASCII lines (bytes, no newline) found between
    frames. `max_frames` (optional) stops after N frames (for tests/captures).
    Returns a (n_frames, n_bad_magic) counter.
    """
    buf = bytearray()
    skipped = bytearray()
    n_good = 0
    n_bad = 0

    def flush_text():
        if skipped and verbose_lines:
            verbose_lines(bytes(skipped))
            del skipped[:]

    for chunk in chunks:
        if not chunk:
            continue
        buf.extend(chunk)
        while True:
            i = buf.find(MAGIC)
            if i < 0:
                # keep a tail (MAGIC may straddle the next chunk); dump any text run
                if len(buf) > 1:
                    skipped.extend(buf[:-1])
                    del buf[:-1]
                flush_text()
                break
            if i > 0:
                skipped.extend(buf[:i])
                del buf[:i]
                flush_text()
            if len(buf) < FRAME_LEN:
                break  # wait for the rest of this frame
            frame = bytes(buf[0:FRAME_LEN])
            if frame_is_valid(frame):
                del buf[:FRAME_LEN]
                emit(frame_to_json(frame), frame)
                n_good += 1
                if max_frames is not None and n_good >= max_frames:
                    return n_good, n_bad
            else:
                # spurious / corrupt magic: step past it and resync
                del buf[:2]
                n_bad += 1
    flush_text()
    return n_good, n_bad


def read_chunks_stdin():
    while True:
        d = sys.stdin.buffer.read(4096)
        if not d:
            break
        yield d


def read_chunks_file(path):
    with open(path, "rb") as fh:
        while True:
            d = fh.read(65536)
            if not d:
                break
            yield d


def read_chunks_serial(port, baud):
    import serial  # pyserial (lazy)
    with serial.Serial(port, baud, timeout=1) as s:
        print(f"[decode] reading {port} @ {baud}", file=sys.stderr)
        while True:
            d = s.read(4096)
            if d:
                yield d


def selftest():
    # Documented worked example (docs/payload_format.md) -- byte-for-byte.
    # Exact bytes the C++ codec emits for the documented sample (verified against
    # lora_frame.cpp: CRC field 0x4CF6). rt is 55 5A (LE) == 23125 == 23.125.
    good = bytes.fromhex(
        "A55A01000100F64C102EFE68555A4E61BC00FC6BFCFF790F0F00"
    )
    assert len(good) == FRAME_LEN, "worked example must be 26 bytes"
    assert struct.unpack_from("<H", good, 6)[0] == 0x4CF6, "worked example CRC must be 0x4CF6"
    assert frame_is_valid(good), "worked example must validate"
    j = frame_to_json(good)
    assert j["seq"] == 1 and j["type"] == "data"
    assert j["ts"] == 1761488400, j
    assert j["ts_iso"] == "2025-10-26T14:20:00Z", j
    assert j["rt"] == 23.125 and j["x"] == 12345.678
    assert j["y"] == -234.5 and j["z"] == 987.001, j

    # A second frame (seq 2) with different values to prove field parsing is positional.
    def make(seq, ts, rt, x, y, z):
        b = bytearray(26)
        b[0:2] = MAGIC
        struct.pack_into("<H", b, 2, seq)
        b[4] = 0x01
        struct.pack_into("<I", b, 8, ts)
        struct.pack_into("<H", b, 12, rt)
        struct.pack_into("<i", b, 14, x)
        struct.pack_into("<i", b, 18, y)
        struct.pack_into("<i", b, 22, z)
        c = crc16_ccitt_false_run(0xFFFF, b[0:6] + b[8:26])
        struct.pack_into("<H", b, 6, c)
        return bytes(b)

    good2 = make(2, 1761488401, 0, -12345678, 999999, -999999)
    assert frame_is_valid(good2), "synthetic frame must validate"

    # Stream: ASCII noise + good frame + ASCII + spurious 0xA5 0x5A (bad CRC) + good2.
    noise_a = b"[hb] tx=5 rx=0 lost=0 crc_err=0 backlog=10 clock=local\n"
    spurious = b"\xa5\x5a\x00\x00\x01\x00\x00\x00" + b"\x00" * 18  # magic + garbage, CRC fails
    stream = noise_a + good + noise_a + spurious + good2
    got = []
    seen_text = []
    n_good, n_bad = scan([stream], emit=lambda j, r: got.append(j),
                         verbose_lines=lambda t: seen_text.append(t), max_frames=None)
    assert n_good == 2, f"expected 2 good frames, got {n_good}"
    assert n_bad >= 1, "expected the spurious magic to be skipped"
    assert got[0]["seq"] == 1 and got[1]["seq"] == 2
    assert any(b"[hb]" in t for t in seen_text), "ASCII heartbeat should be captured in verbose mode"
    print("selftest OK: worked example reproduces byte-for-byte; resync + CRC skip verified")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    src = ap.add_mutually_exclusive_group()
    src.add_argument("-p", "--port", help="USB-serial port (e.g. /dev/ttyACM0); needs pyserial")
    src.add_argument("-f", "--file", help="read a saved binary capture from FILE")
    src.add_argument("-b", "--baud", type=int, default=115200, help="serial baud (nominal for USB CDC; default 115200)")
    ap.add_argument("-v", "--verbose", action="store_true", help="echo ASCII heartbeat/status lines to stderr")
    ap.add_argument("--raw", action="store_true", help="also print the 26 raw frame bytes (hex) after each JSON line")
    ap.add_argument("--selftest", action="store_true", help="run the built-in byte-level self-test and exit")
    args = ap.parse_args(argv)

    if args.selftest:
        selftest()
        return 0

    def emit(j, raw):
        print(json.dumps(j), flush=True)
        if args.raw:
            print("raw:", raw.hex(" "), flush=True)

    vlines = (lambda t: print(t.decode("utf-8", "replace").rstrip(), file=sys.stderr)) if args.verbose else None

    if args.port:
        try:
            chunks = read_chunks_serial(args.port, args.baud)
        except ImportError:
            print("error: pyserial not installed. `python3 -m pip install --user pyserial`,\n"
                  "       or capture first (pio device monitor -e wio_s3_lora --record) then: decode.py -f <file>",
                  file=sys.stderr)
            return 2
    elif args.file:
        chunks = read_chunks_file(args.file)
    else:
        chunks = read_chunks_stdin()

    try:
        n_good, n_bad = scan(chunks, emit, vlines)
        print(f"[decode] {n_good} frames decoded, {n_bad} bad magic skipped", file=sys.stderr)
    except KeyboardInterrupt:
        print("\n[decode] interrupted", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
