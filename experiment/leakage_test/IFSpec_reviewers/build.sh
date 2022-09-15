#!/bin/bash

gcc -O3 -c ../memory.c -o m.o
gcc -O3 -c ../decode_inputs.c -o d.o
gcc -O3 -c ../base64.c -o b.o
gcc -O3 -c ../json.c -o j.o

afl-clang-fast++ reviewers_secure.cpp m.o d.o b.o j.o -I../ -lm -o fuzz_secure
afl-clang-fast++ reviewers_insecure.cpp m.o d.o b.o j.o -I../ -lm -o fuzz_insecure

rm *.o
