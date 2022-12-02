#!/bin/bash

re='^[0-9]+$'
if ! [[ $1 =~ $re ]] || [ $# -ne 1 ]; then
  echo "Usage: `basename $0` [number_of_seconds_to_run_AFL_for]"
  exit 0
fi

/usr/bin/time -v timeout $1 afl-fuzz -i IN -o OUT -- ./fuzz @@ > fuzzing.log 2>&1 &
cp model_check.log ../results/$(basename pwd)_model_check.log

wait
