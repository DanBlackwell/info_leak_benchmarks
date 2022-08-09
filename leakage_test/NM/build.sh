#!/bin/bash

afl-clang-fast fuzz_harness.c ../memory.c ../decode_inputs.c ../base64.c ../json.c repo/libnm-util/nm-param-spec-specialized.c repo/libnm-util/nm-setting.c repo/libnm-util/nm-setting-vpn.c -Irepo/libnm-util -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -Irepo/include -I/usr/include/dbus-1.0/ -lglib-2.0 -ldbus-glib-1 -lgobject-2.0 -I../ -lm -o fuzz
