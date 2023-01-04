#!/usr/bin/env bash

re='^[0-9]+$'
if ! [[ $1 =~ $re ]] || [ $# -ne 1 ]; then
  echo "Usage: `basename $0` [max_number_of_seconds_to_model_check_for]"
  exit 0
fi

echo "WILL BEGIN MODEL CHECKING WITH TIMEOUT OF $(( $1 / 3600 )) hours"

cd "$(dirname "${BASH_SOURCE[0]}")"
mkdir -p leakage_test/results
cp parse_model_check_results.py leakage_test/results/

for F in leakage_test/*/; do
  pushd $F;
    if ! [[ -f model_check.sh ]]; then
      popd
      continue
    fi

    echo "Beginning model checking $(basename $F)"
    # model check with timeout $1
    ./model_check.sh $1
  popd
done

wait

cd leakage_test/results
  ./parse_model_check_results.py > model_check_results.json
cd -
