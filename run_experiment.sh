#!/bin/bash

if [[ $(id -u) -ne 0 ]]; then 
  echo "Please run as root"
  exit
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

VANILLA_AFL=1

if [[ -z "${VANILLA_AFL}" ]]; then
  echo "Building AFL_info_leakage experiment"
  OUTPUT_DIR="experimental_results/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
else
  echo "Building VANILLA AFL experiment"
  OUTPUT_DIR="experimental_results_vanilla/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
fi
mkdir -p $OUTPUT_DIR

if [[ -z "${VANILLA_AFL}" ]]; then
  IMAGE=leaks_vanilla
else
  IMAGE=leaks
fi

docker build --build-arg VANILLA=1 -t $IMAGE .
NAME="leaks_exp"
if [[ -z "${VANILLA_AFL}" ]]; then
  docker build --build-arg -t $IMAGE .
else
  docker build --build-arg VANILLA=1 -t $IMAGE .
fi
docker container run --name $NAME $IMAGE
docker cp $NAME:/app/leakage_test $OUTPUT_DIR/ && docker rm $NAME

echo "COMPLETED EXPERIMENT - COPIED RESULTS INTO $OUTPUT_DIR"
