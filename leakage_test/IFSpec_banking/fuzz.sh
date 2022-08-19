#!/bin/bash

afl-fuzz -i IN -o SECURE_OUT -- ./fuzz_secure > fuzzing_secure.log 2>&1 &
afl-fuzz -i IN -o INSECURE_OUT -- ./fuzz_insecure > fuzzing_insecure.log 2>&1 &
