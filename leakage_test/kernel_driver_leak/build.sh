#!/bin/bash

gcc -c ../memory.c -o m.o
gcc -c ../decode_inputs.c -o d.o
gcc -c ../base64.c -o b.o
gcc -c ../json.c -o j.o
afl-clang-fast -fsanitize=fuzzer fuzz_harness.c m.o d.o b.o j.o -I../ -Irepo/ -lm -o fuzz

rm *.o
