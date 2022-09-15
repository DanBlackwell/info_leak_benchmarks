#!/bin/bash

CFLAGS=""
if ! [[ -z "${VANILLA}" ]]; then
  CFLAGS="-D VANILLA_AFL"
  export AFL_USE_MSAN=1
fi

gcc -O3 -c ../memory.c -o m.o
gcc -O3 -c ../decode_inputs.c -o d.o
gcc -O3 -c ../base64.c -o b.o
gcc -O3 -c ../json.c -o j.o
afl-clang-fast $CFLAGS -fsanitize=fuzzer fuzz_harness.c m.o d.o b.o j.o -I../ -Irepo/ -lm -o fuzz

rm *.o
