#!/usr/bin/python3

import base64
import json
import sys
import os

if len(sys.argv) != 3:
    print('Usage: {} "public value" "secret value"'.format(sys.argv[0]))
    sys.exit(1)

args = [os.fsencode(arg) for arg in sys.argv]
public = args[1]
secret = args[2]

pub_b64 = base64.b64encode(public)
sec_b64 = base64.b64encode(secret)

print(json.dumps({'PUBLIC': pub_b64.decode('utf-8'), 'SECRET': sec_b64.decode('utf-8')}))

