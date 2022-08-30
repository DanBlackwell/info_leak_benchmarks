#!/bin/bash

if [[ $(id -u) -ne 0 ]]; then 
  echo "Please run as root"
  exit
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

OUTPUT_DIR="experimental_results_vanilla/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
mkdir -p $OUTPUT_DIR

IMAGE=leaks_vanilla
docker build -t $IMAGE .
NAME="leaks_exp_vanilla"
docker container run --name $NAME $IMAGE
docker cp $NAME:/app/leakage_test $OUTPUT_DIR/ && docker rm $NAME

echo "COMPLETED EXPERIMENT - COPIED RESULTS INTO $OUTPUT_DIR"
