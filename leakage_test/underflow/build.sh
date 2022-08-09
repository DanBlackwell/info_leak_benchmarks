#!/bin/bash

afl-clang-fast repo/underflow.c  ../memory.c ../decode_inputs.c ../base64.c ../json.c -I../ -lm -o fuzz
