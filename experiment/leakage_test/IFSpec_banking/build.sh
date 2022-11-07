#!/bin/bash

CFLAGS=""
EXTRA_FILES=""
if ! [[ -z "${VANILLA}" ]]; then
  #CFLAGS="-D MSAN"
  #export AFL_USE_MSAN=1

  CXX="clang++-dfsan"
  CFLAGS="-D DFSAN"
else
  CXX="afl-clang-fast++"
  CFLAGS="-fsanitize=fuzzer -Wl,--wrap=malloc"

  gcc -O3 -c ../memory.c -o m.o
  gcc -O3 -c ../decode_inputs.c -o d.o
  gcc -O3 -c ../base64.c -o b.o
  gcc -O3 -c ../json.c -o j.o

  EXTRA_FILES="m.o d.o b.o j.o"
fi

$CXX $CFLAGS banking_secure.cpp $EXTRA_FILES -I../ -lm -o fuzz_secure
$CXX $CFLAGS banking_insecure.cpp $EXTRA_FILES -I../ -lm -o fuzz_insecure

rm -f *.o
