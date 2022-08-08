#!/usr/bin/python3

import base64
import json
import sys

if len(sys.argv) != 3:
    print('Usage: {} "public value" "secret value"'.format(sys.argv[0]))
    sys.exit(1)

public = sys.argv[1]
secret = sys.argv[2]

b64_bytes = public.encode('utf-8')
pub_b64 = base64.b64encode(b64_bytes)

b64_bytes = secret.encode('utf-8')
sec_b64 = base64.b64encode(b64_bytes)

print(json.dumps({'PUBLIC': pub_b64.decode('utf-8'), 'SECRET': sec_b64.decode('utf-8')}))

