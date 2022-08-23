#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")"

for F in leakage_test/*/; do
  pushd $F;
    if ! [[ -f fuzz.sh ]]; then
      popd
      continue
    fi
    
    echo "Beginning fuzzing $(basename $F)"
    ./fuzz.sh
  popd
done
