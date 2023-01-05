#!/usr/bin/env python3

import os
import re
import json

results = {}

for filename in os.listdir(os.getcwd()):
    timeout = False
    oom = False
    segfault = False
    exit_code = 0
    exit_meaning = ''
    runtime_millis = 0
    runtime_str = ''
    max_memory = 0

    if re.search('\.log', filename) is None:
        continue

    with open(os.path.join(os.getcwd(), filename), 'r') as f: # open in readonly mode
        for line in f.readlines():
            matches = re.findall('Command exited with non-zero status ([0-9]+)', line)
            if len(matches) > 0:
                exit_code = int(matches[0])
                timeout = exit_code == 124
                if timeout:
                    exit_meaning = 'timeout'

            if re.match('Out of memory', line) is not None:
                oom = True
                exit_meaning = 'OOM'

            matches = re.findall('Command terminated by signal ([0-9]+)', line)
            if len(matches) > 0:
                exit_code = int(matches[0])
                segfault = exit_code == 11

            matches = re.findall('Elapsed \(wall clock\) time \(h\:mm\:ss or m\:ss\)\: ([0-9:\.]+)', line)
            if len(matches) > 0:
                runtime_str = matches[0]
                components = matches[0].split(':')
                if len(components) > 2:
                    runtime_millis = int(components[0]) * 3600000 + int(components[1]) * 60000 + int(components[2]) * 1000
                else:
                    runtime_millis = int(components[0]) * 60000 + int(float(components[1]) * 1000)

            matches = re.findall('Maximum resident set size \(kbytes\)\: ([0-9]+)', line)
            if len(matches) > 0:
                max_memory = int(matches[0]) * 1024

#        print('{}: exit: {} ({}), runtime: {} ({} hours), memory: {}'
#                .format(filename, 
#                        exit_code if not timeout else 11, exit_meaning, 
#                        runtime_str, runtime_millis / (1000 * 3600), 
#                        max_memory / pow(1024, 3)))

        sut = filename.split('_model_check.log')[0]
        results[sut] = {
            'exit_code': exit_code, 
            'runtime': runtime_millis, 
            'max_memory': max_memory,
            'OOM': oom,
            'timeout': timeout,
            'segfault': segfault
        }

print(json.dumps(results))


