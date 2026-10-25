# xiao-mag-lora — 2-node PoC bench loop
#
# No extra dependencies beyond what already builds the firmware: pio, python3,
# and the standard Unix `stty`/`cat` for pyserial-free serial capture.
#
# Quick start (two Kits; you watch node B's serial for node A's frames):
#   make ports
#   make build
#   make flashA PORTA=/dev/ttyACM0
#   make flashB PORTB=/dev/ttyACM1
#   make linktest PORTB=/dev/ttyACM1        # live JSON for 30 s (Ctrl-C to stop)
#
# Everything overrides inline, e.g.:
#   make cap PORTB=/dev/ttyUSB1 DUR=60 CAPTURE=run2.bin
#
# Raw one-liners (no Makefile needed):
#   stty -F /dev/ttyACM1 115200 raw -echo -crtscts min 0 time 1
#   timeout 30 cat /dev/ttyACM1 | python3 src/host/decode.py -v

PORTA   ?= /dev/ttyACM0
PORTB   ?= /dev/ttyACM1
BAUD    ?= 115200
DUR     ?= 30
CAPTURE ?= capture.bin
FILE    ?= $(CAPTURE)
FW      := src/firmware
HOST    := src/host
PYTHON  ?= python3

.DEFAULT_GOAL := help

.PHONY: help ports build test selftest flashA flashB flash monitorA monitorB cap decode linktest clean

help:
	@echo "xiao-mag-lora bench loop   (ports: PORTA=$(PORTA)  PORTB=$(PORTB))"
	@echo ""
	@echo "  make ports                          list available serial ports"
	@echo "  make build                          build firmware (ESP32-S3)"
	@echo "  make test                           host unit test (31/31)"
	@echo "  make selftest                       decoder self-test + host unit test"
	@echo "  make flashA   [PORTA=...]           flash node A"
	@echo "  make flashB   [PORTB=...]           flash node B"
	@echo "  make flash    [PORTA=...]           build + flash to PORTA"
	@echo "  make monitorB [PORTB=...]           raw serial monitor (pio)"
	@echo "  make linktest [PORTB=... DUR=30]    LIVE capture of B -> JSON (no pyserial)"
	@echo "  make cap      [PORTB=... DUR=30]    capture raw bytes to $(CAPTURE) (no pyserial)"
	@echo "  make decode   [FILE=$(CAPTURE)]     decode a capture file to JSON"
	@echo "  make clean"
	@echo ""
	@echo "  One-liner (no Makefile):"
	@echo "    stty -F $(PORTB) $(BAUD) raw -echo -crtscts min 0 time 1"
	@echo "    timeout $(DUR) cat $(PORTB) | $(PYTHON) $(HOST)/decode.py -v"

ports:
	@ls -l /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || echo "no /dev/ttyACM* or /dev/ttyUSB* found"

build:
	cd $(FW) && pio run -e wio_s3_lora

test:
	cd $(FW) && ./test/run_host_test.sh

selftest:
	$(PYTHON) $(HOST)/decode.py --selftest
	cd $(FW) && ./test/run_host_test.sh

flashA:
	cd $(FW) && pio run -e wio_s3_lora -t upload -p $(PORTA)

flashB:
	cd $(FW) && pio run -e wio_s3_lora -t upload -p $(PORTB)

flash: build
	cd $(FW) && pio run -e wio_s3_lora -t upload -p $(PORTA)

monitorB:
	cd $(FW) && pio device monitor -p $(PORTB) -b $(BAUD) -e wio_s3_lora

monitorA:
	cd $(FW) && pio device monitor -p $(PORTA) -b $(BAUD) -e wio_s3_lora

# pyserial-free: set raw non-blocking, capture for DUR seconds.
cap:
	@stty -F $(PORTB) $(BAUD) raw -echo -crtscts min 0 time 1 2>/dev/null || true
	@echo "[cap] capturing $(PORTB) for $(DUR) s -> $(CAPTURE)  (Ctrl-C stops early)"
	@timeout $(DUR) cat $(PORTB) > $(CAPTURE)
	@echo "[cap] saved $$(wc -c < $(CAPTURE)) bytes"

# Live: stream B's serial to the decoder (JSON appears as frames arrive).
linktest:
	@stty -F $(PORTB) $(BAUD) raw -echo -crtscts min 0 time 1 2>/dev/null || true
	@echo "[linktest] $(PORTB) -> JSON for up to $(DUR) s  (Ctrl-C stops early)"
	@timeout $(DUR) cat $(PORTB) | $(PYTHON) $(HOST)/decode.py -v

decode:
	$(PYTHON) $(HOST)/decode.py -f $(FILE) -v

clean:
	cd $(FW) && pio run -e wio_s3_lora -t clean
	rm -f $(CAPTURE)
