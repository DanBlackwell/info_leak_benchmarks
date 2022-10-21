#!/bin/bash

set -e 

if [[ $(id -u) -ne 0 ]]; then 
  echo "Please run as root"
  exit
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

VANILLA_AFL=1

if [[ -z "${VANILLA_AFL}" ]]; then
  echo "Building AFL_info_leakage experiment"
  OUTPUT_DIR="../experimental_results/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
else
  echo "Building VANILLA AFL experiment"
  OUTPUT_DIR="../experimental_results_vanilla/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
fi
mkdir -p $OUTPUT_DIR

NAME="leaks_exp"
if [[ -z "${VANILLA_AFL}" ]]; then
  IMAGE=leaks
  docker build -t $IMAGE .
else
  IMAGE=leaks_vanilla
  docker build --build-arg VANILLA=1 -t $IMAGE .
fi

# Disable ASLR for the run
echo 0 | tee /proc/sys/kernel/randomize_va_space

docker container run --ulimit core=0 --name $NAME $IMAGE

# Reenable ASLR
echo 2 | tee /proc/sys/kernel/randomize_va_space

docker cp $NAME:/app/leakage_test $OUTPUT_DIR/ && docker rm $NAME

chown -R dblackwell $OUTPUT_DIR

echo "COMPLETED EXPERIMENT - COPIED RESULTS INTO $OUTPUT_DIR"
