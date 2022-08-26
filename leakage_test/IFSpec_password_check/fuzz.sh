#!/bin/bash

re='^[0-9]+$'
if ! [[ $1 =~ $re ]] || [ $# -ne 1 ]; then
  echo "Usage: `basename $0` [number_of_seconds_to_run_AFL_for]"
  exit 0
fi

timeout $1 afl-fuzz -i IN -o SECURE_OUT -- ./fuzz_secure > fuzzing_secure.log 2>&1 &
timeout $1 afl-fuzz -i IN -o INSECURE_OUT -- ./fuzz_insecure > fuzzing_insecure.log 2>&1 &
