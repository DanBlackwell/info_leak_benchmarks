#!/bin/bash

afl-fuzz -i IN -o OUT -- ./fuzz @@ > fuzzing.log 2>&1 &
