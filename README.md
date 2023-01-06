# Info Leak Benchmarks

A set of benchmark programs containing examples of information leaks (violations of the program's information flow control policies).

To run an experiment, run the `./experiment/run_experiment.sh` script. This will create a docker image instance containing all of the benchmarks, run the leak finder, and copy the results out of the container to `./experimental_results` (with a suffix indicating the leak finding approach).

## Setting the leak finding approach

These can be set by setting the following environment variables inside the `./experiment/run_experiment.sh` script:
* AFL++ w/ Sanitizers: `VANILLA_AFL=1`
* CBMC (C bounded model checker): `CBMC=1`
* LeakFuzzer: DO NOT SET ANY OF THE ABOVE VARIABLES

In order to add additional leak finding approaches, modifications will need to be made to `./experiment/Dockerfile`.

## Interpreting Results

The results will be output to `./experimental_results` (with suffix indicating leak finding approach) in a subdirectory named `${HOSTNAME}_${START_TIME_OF_EXPERIMENT}`. Within this is a further subdirectory `leakage_tests` which contains a full copy of the experiment (including executables). Within `leakage_tests` is a directory `results` and within here a script `parse_${LEAK_FINDING_APPROACH}_results.py`. Running this script will collect up a statistics from each benchmark and output them to a JSON over stdout. These results for individual runs can be aggregated using the `./experiments/scripts/combine_${LEAK_FINDING_APPROACH}_results.py` for further analysis (note that this script should be copied into `./experimental_results` and ran from here).
