#!/bin/bash

CFLAGS=""
if ! [[ -z "${VANILLA}" ]]; then
  CFLAGS="-D VANILLA_AFL"
  export AFL_USE_MSAN=1
fi

gcc -c ../memory.c -o m.o
gcc -c ../decode_inputs.c -o d.o
gcc -c ../base64.c -o b.o
gcc -c ../json.c -o j.o
afl-clang-fast $CFLAGS fuzz_harness.c m.o d.o b.o j.o -I../ -lm -o fuzz

rm *.o
