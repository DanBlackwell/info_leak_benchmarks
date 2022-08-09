#!/bin/bash

afl-clang-fast++ repo/atalk.c  ../memory.c ../decode_inputs.c ../base64.c ../json.c -I../ -Irepo/ -lm -o fuzz
