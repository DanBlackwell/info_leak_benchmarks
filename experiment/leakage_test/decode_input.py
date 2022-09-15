#!/usr/bin/python

import base64
import json
import sys

f = open(sys.argv[1])
j = json.loads(f.read())
msg = j["SECRET"]
b64_bytes = msg.encode("utf-8")
msg_bytes = base64.b64decode(b64_bytes)
print('SECRET: ', end='')
print(msg_bytes)

msg = j["PUBLIC"]
b64_bytes = msg.encode("utf-8")
msg_bytes = base64.b64decode(b64_bytes)
print('PUBLIC: ', end='')
print(msg_bytes)
