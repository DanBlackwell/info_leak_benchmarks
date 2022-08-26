#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")"

pushd Grammar-Mutator
  make GRAMMAR_FILE=../sql-grammar.json
popd

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
