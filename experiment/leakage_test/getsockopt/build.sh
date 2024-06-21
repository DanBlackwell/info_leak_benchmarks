#!/bin/bash

set -e

if ! [[ -z "${VANILLA}" ]]; then
  CFLAGS="-D VANILLA_AFL"
  export AFL_USE_MSAN=1

elif ! [[ -z "${CBMC}" ]]; then
  goto-cc repo/cbmc_harness.c -D CBMC -Irepo --function cbmc_test -o test
  exit 0

else
  CFLAGS="-Wl,--wrap=malloc"
  
  gcc -O3 -c ../memory.c -o m.o
  gcc -O3 -c ../decode_inputs.c -o d.o
  gcc -O3 -c ../base64.c -o b.o
  gcc -O3 -c ../json.c -o j.o

  EXTRA_FILES="m.o d.o b.o j.o"
fi

afl-clang-fast -fsanitize=fuzzer $CFLAGS repo/fuzz_harness.c $EXTRA_FILES -I../ -Irepo/ -lm -o fuzz

rm -f *.o
