#!/bin/bash

afl-clang-fast fuzz_harness.c ../memory.c ../decode_inputs.c ../base64.c ../json.c -I../ -lm -o fuzz
