#!/bin/bash

set -e 

if [[ $(id -u) -ne 0 ]]; then 
  echo "Please run as root"
  exit
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

VANILLA_AFL=1
# CBMC=1
DOCKER_ARGS=""

if ! [[ -z "${VANILLA_AFL}" ]]; then
  echo "Building VANILLA AFL experiment"
  OUTPUT_DIR="../experimental_results_vanilla/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
  NAME="vanilla_leaks_exp"
  DOCKER_ARGS="--build-arg VANILLA=1"
elif ! [[ -z "${CBMC}" ]]; then
  echo "Building CBMC model checking experiment"
  OUTPUT_DIR="../experimental_results_cbmc/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
  NAME="cbmc_leaks_exp"
  DOCKER_ARGS="--build-arg CBMC=1"
else
  echo "Building AFL_info_leakage experiment"
  OUTPUT_DIR="../experimental_results/$(hostname)_$(date -d "today" +"%Y_%m_%d_%H%M")"
  NAME="leakfuzzer_leaks_exp"
fi
mkdir -p $OUTPUT_DIR

IMAGE="${NAME}_image"
docker build $DOCKER_ARGS -t $IMAGE .

# Disable ASLR for the run
echo 0 | tee /proc/sys/kernel/randomize_va_space

docker container run --shm-size=2g --ulimit core=0 --name $NAME $IMAGE

# Reenable ASLR
echo 2 | tee /proc/sys/kernel/randomize_va_space

docker cp $NAME:/app/leakage_test $OUTPUT_DIR/ && docker rm $NAME

chown -R dblackwell $OUTPUT_DIR

echo "COMPLETED EXPERIMENT - COPIED RESULTS INTO $OUTPUT_DIR"
