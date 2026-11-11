#!/bin/sh
# Host logic tests across profiles/architecture macros. Needs only g++.
# NOT a board build: it uses shims (tests/host/shim) with an emulated flash space.
set -e
cd "$(dirname "$0")"
OUT=${TMPDIR:-/tmp}/dumirror_tests; mkdir -p "$OUT"
FLAGS="-std=gnu++11 -Wall -Wextra -Wshadow -Werror -no-pie -fno-pie -O1 -Ishim -I../../src"
run() { name=$1; shift
  echo "=== $name"
  g++ $FLAGS "$@" test_dumirror.cpp shim/shim.cpp -o "$OUT/t_$name"
  "$OUT/t_$name"; }
run generic_full
run avr_uno_auto_minimal -DARDUINO_ARCH_AVR -DFLASHEND=0x7FFFUL
run avr_64k_auto_compact -DARDUINO_ARCH_AVR -DFLASHEND=0xFFFFUL
run compact -DDUMIRROR_PROFILE_COMPACT
run avr_mega_full    -DARDUINO_ARCH_AVR -DFLASHEND=0x3FFFFUL
run esp32            -DARDUINO_ARCH_ESP32
run esp8266          -DARDUINO_ARCH_ESP8266
run rp2040           -DARDUINO_ARCH_RP2040
for cfg in "full:" "avr_minimal:-DARDUINO_ARCH_AVR -DFLASHEND=0x7FFFUL"; do
  n=${cfg%%:*}; f=${cfg#*:}
  echo "=== touchpet_sim_$n"
  g++ $FLAGS $f touchpet_sim.cpp shim/shim.cpp -o "$OUT/tp_$n"
  "$OUT/tp_$n"
done
echo "ALL HOST TEST CONFIGURATIONS PASSED"
