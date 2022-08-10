#!/bin/bash

afl-clang-fast -c ../memory.c -o m.o
afl-clang-fast -c ../decode_inputs.c -o d.o
afl-clang-fast -c ../base64.c -o b.o
afl-clang-fast -c ../json.c -o j.o
afl-clang-fast++ repo/atalk.c m.o d.o b.o j.o -I../ -Irepo/ -lm -o fuzz
