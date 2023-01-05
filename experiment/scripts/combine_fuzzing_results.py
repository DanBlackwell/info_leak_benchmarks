#!/usr/bin/env python3

import os
import re
import subprocess
import json

suts = {}

results_dirs = []
for f in os.listdir(os.getcwd()):
    results_dir = os.path.join(os.getcwd(), f, 'leakage_test', 'results')
    if os.path.exists(results_dir):
        results_dirs.append(results_dir)

for index, results_dir in enumerate(results_dirs):
    cur_dir = os.getcwd()
    os.chdir(results_dir)
    output = subprocess.check_output(['./parse_fuzzing_results.py']).decode('ascii')
    run = json.loads(str(output))

    for sut, result in run.items():
        if sut in suts:
            d = suts[sut]
            for key, value in result.items():
                d[key].append(value)
        else:
            d = {}
            for key, value in result.items():
                d[key] = [value]
            suts[sut] = d
    os.chdir(cur_dir)
    print('Processed {} of {} runs'.format(index + 1, len(results_dirs)))

print(json.dumps(suts, indent = 2))

