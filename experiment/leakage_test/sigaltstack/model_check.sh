#!/usr/bin/env bash

/usr/bin/time -v timeout $1 cbmc test > model_check.log 2>&1
