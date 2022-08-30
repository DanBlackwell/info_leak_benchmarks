#!/usr/bin/env bash

re='^[0-9]+$'
if ! [[ $1 =~ $re ]] || [ $# -ne 1 ]; then
  echo "Usage: `basename $0` [number_of_seconds_to_run_AFL_for]"
  exit 0
fi

echo "WILL BEGIN FUZZING WITH TIMEOUT OF $(( $1 / 3600 )) hours"

cd "$(dirname "${BASH_SOURCE[0]}")"

for F in leakage_test/*/; do
  pushd $F;
    if ! [[ -f fuzz.sh ]]; then
      popd
      continue
    fi

    echo "Beginning fuzzing $(basename $F)"
    ./fuzz.sh $1 &
  popd
done

wait
