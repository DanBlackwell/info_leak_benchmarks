#!/bin/bash

afl-clang-fast -c ../memory.c -o m.o
afl-clang-fast -c ../decode_inputs.c -o d.o
afl-clang-fast -c ../base64.c -o b.o
afl-clang-fast -c ../json.c -o j.o

afl-clang-fast++ reviewers_secure.cpp m.o d.o b.o j.o -I../ -lm -o fuzz_secure
afl-clang-fast++ reviewers_insecure.cpp m.o d.o b.o j.o -I../ -lm -o fuzz_insecure

rm *.o
