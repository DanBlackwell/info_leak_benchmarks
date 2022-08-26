#!/bin/bash

if [[ $(id -u) -ne 0 ]]; then 
  echo "Please run as root"
  exit
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

OUTPUT_DIR="experimental_results/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
mkdir -p $OUTPUT_DIR

docker build -t leaks .
NAME="leaks_exp"
docker container run --name $NAME leaks 
docker cp $NAME:/app/leakage_test $OUTPUT_DIR/ && docker rm $NAME

echo "COMPLETED EXPERIMENT - COPIED RESULTS INTO $OUTPUT_DIR"
