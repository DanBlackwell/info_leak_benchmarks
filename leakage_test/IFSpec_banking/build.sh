#!/bin/bash

gcc -c ../memory.c -o m.o
gcc -c ../decode_inputs.c -o d.o
gcc -c ../base64.c -o b.o
gcc -c ../json.c -o j.o

afl-clang-fast++ banking_secure.cpp m.o d.o b.o j.o -I../ -lm -o fuzz_secure
afl-clang-fast++ banking_insecure.cpp m.o d.o b.o j.o -I../ -lm -o fuzz_insecure

rm *.o
