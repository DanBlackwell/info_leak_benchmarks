#!/bin/bash

gcc fuzz_harness.c ../memory.c ../decode_inputs.c ../base64.c ../json.c -I../ -Irepo/ -lm -o fuzz
