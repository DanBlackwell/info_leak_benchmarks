#!/bin/bash

afl-clang-fast fuzz_harness.c -I../ -o fuzz
