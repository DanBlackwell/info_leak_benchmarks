#!/usr/bin/env python3

import os
import re

# from https://stackoverflow.com/a/45960693
def tail(f, window=20):
    """Returns the last `window` lines of file `f` as a list.
    """
    if window == 0:
        return []

    BUFSIZ = 1024
    f.seek(0, 2)
    remaining_bytes = f.tell()
    size = window + 1
    block = -1
    data = []

    while size > 0 and remaining_bytes > 0:
        if remaining_bytes - BUFSIZ > 0:
            # Seek back one whole BUFSIZ
            f.seek(block * BUFSIZ, 2)
            # read BUFFER
            bunch = f.read(BUFSIZ)
        else:
            # file too small, start from beginning
            f.seek(0, 0)
            # only read what was not read
            bunch = f.read(remaining_bytes)

        bunch = bunch.decode('utf-8')
        data.insert(0, bunch)
        size -= bunch.count('\n')
        remaining_bytes -= BUFSIZ
        block -= 1

    return ''.join(data).splitlines()[-window:]

results = {}

for filename in os.listdir(os.getcwd()):
    result = {
        'timeout': False,
        'oom': False,
        'segfault': False,
        'exit_code': 0,
        'exit_meaning': '',
        'runtime_millis': 0,
        'runtime_str': '',
        'max_memory': 0,
        'leaking_pairs': 0,
        'first_leak_time': -1,
        'edges_found': 0,
        'total_edges': 0,
        'cycles_done': 0,
        'execs_done': 0,
        'crash_count': 0,
        'hang_count': 0
    }

    if re.search('\.log', filename) is None:
        continue

    sut = filename.split('_fuzzing.log')[0]
    if 'insecure' in sut:
        out_path = 'INSECURE_OUT'
        sut_dir = sut.split('_insecure')[0]
    elif 'secure' in sut:
        out_path = 'SECURE_OUT'
        sut_dir = sut.split('_secure')[0]
    else:
        out_path = 'OUT'
        sut_dir = sut

    fuzzer_out_dir = os.path.join(os.path.dirname(os.getcwd()), sut_dir, out_path, 'default')

    # parse fuzzer stats
    with open(os.path.join(fuzzer_out_dir, 'fuzzer_stats')) as f:
        for line in f.readlines():
            splitty = line.split(' ')
            for key in ['cycles_done', 'execs_done', 'edges_found', 'total_edges']:
                if splitty[0] == key:
                    result[key] = int(splitty[-1]) 

    # parse output dirs
    leaks_dir_path = os.path.join(fuzzer_out_dir, 'leaks')
    if os.path.exists(leaks_dir_path):
        files = os.listdir(leaks_dir_path)
        earliest = 999999999999
        for fname in files:
            time = int(fname.split(',')[1].split(':')[1])
            earliest = min(time, earliest)
        if len(files) > 0:
            result['first_leak_time'] = earliest

    hangs_dir_path = os.path.join(fuzzer_out_dir, 'hangs')
    result['hang_count'] = len(os.listdir(hangs_dir_path))
    crashes_dir_path = os.path.join(fuzzer_out_dir, 'crashes')
    result['crashes_count'] = len(os.listdir(crashes_dir_path))
        
    # parse fuzzing log
    exit_found = False
    runtime_found = False
    memory_found = False
    with open(os.path.join(os.getcwd(), filename), 'rb') as f: # open in readonly mode
        for line in tail(f, 100):
            matches = re.findall('Command exited with non-zero status ([0-9]+)', line)
            if len(matches) > 0:
                result['exit_code'] = int(matches[0])
                result['timeout'] = result['exit_code'] == 124
                if result['timeout']:
                    result['exit_meaning'] = 'timeout'

            if re.match('Out of memory', line) is not None:
                result['oom'] = True
                result['exit_meaning'] = 'OOM'

            matches = re.findall('Command terminated by signal ([0-9]+)', line)
            if len(matches) > 0:
                result['exit_code'] = int(matches[0])
                result['segfault'] = exit_code == 11

            matches = re.findall('Elapsed \(wall clock\) time \(h\:mm\:ss or m\:ss\)\: ([0-9:\.]+)', line)
            if len(matches) > 0:
                result['runtime_str'] = matches[0]
                components = matches[0].split(':')
                if len(components) > 2:
                    result['runtime_millis'] = int(components[0]) * 3600000 + int(components[1]) * 60000 + int(components[2]) * 1000
                else:
                    result['runtime_millis'] = int(components[0]) * 60000 + int(float(components[1]) * 1000)

            matches = re.findall('Maximum resident set size \(kbytes\)\: ([0-9]+)', line)
            if len(matches) > 0:
                result['max_memory'] = int(matches[0]) * 1024

#        print('{}: exit: {} ({}), runtime: {} ({} hours), memory: {}'
#                .format(filename, 
#                        exit_code if not timeout else 11, exit_meaning, 
#                        runtime_str, runtime_millis / (1000 * 3600), 
#                        max_memory / pow(1024, 3)))

    results[sut] = result
print(results)

