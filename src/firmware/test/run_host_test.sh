#!/usr/bin/env bash
# Host-side unit test for the portable firmware modules (no PlatformIO needed).
# Compiles the pure-C++ modules + the test with a standard compiler and runs it.
set -u
cd "$(dirname "$0")/.."

CXX="${CXX:-g++}"
out="$(mktemp)"

"$CXX" -std=c++17 -Wall -Wextra -O2 -Iinclude \
  test/host_test.cpp \
  src/lora_frame.cpp \
  src/sample_store.cpp \
  src/link.cpp \
  src/gnss_nmea.cpp \
  src/radio_stub.cpp \
  -o "$out"
if [ $? -ne 0 ]; then
  rm -f "$out"
  echo "BUILD FAILED"
  exit 2
fi

"$out"
rc=$?
rm -f "$out"
exit $rc
