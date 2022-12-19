#!/bin/bash

re='^[0-9]+$'
if ! [[ $1 =~ $re ]] || [ $# -ne 1 ]; then
  echo "Usage: `basename $0` [number_of_seconds_to_run_AFL_for]"
  exit 0
fi

/usr/bin/time -v timeout $1 afl-fuzz -i IN -o SECURE_OUT -- ./fuzz_secure @@ > fuzzing_secure.log 2>&1 &
/usr/bin/time -v timeout $1 afl-fuzz -i IN -o INSECURE_OUT -- ./fuzz_insecure @@ > fuzzing_insecure.log 2>&1 &
wait

cp fuzzing_secure.log ../results/$(basename $PWD)_secure_fuzzing.log
cp fuzzing_insecure.log ../results/$(basename $PWD)_insecure_fuzzing.log
