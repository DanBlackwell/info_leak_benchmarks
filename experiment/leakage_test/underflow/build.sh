#!/bin/bash

CC="afl-clang-fast"
CFLAGS=""
EXTRA_FILES=""
if ! [[ -z "${VANILLA}" ]]; then
  CFLAGS="-D VANILLA_AFL"
  CC="clang-dfsan"
  # export AFL_USE_MSAN=1

elif ! [[ -z "${CBMC}" ]]; then
  goto-cc repo/cbmc_harness.c -D CBMC --function cbmc_test -o test
  exit 0

else
  CFLAGS="-Wl,--wrap=malloc -fsanitize=fuzzer"

  gcc -O3 -c ../memory.c -o m.o
  gcc -O3 -c ../decode_inputs.c -o d.o
  gcc -O3 -c ../base64.c -o b.o
  gcc -O3 -c ../json.c -o j.o

  EXTRA_FILES="m.o d.o b.o j.o"
fi

$CC $CFLAGS repo/underflow.c $EXTRA_FILES -I../ -lm -o fuzz

rm -f *.o
