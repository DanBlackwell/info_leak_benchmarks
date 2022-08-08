TARGET  := leak
WARN    := -W -Wall -Wstrict-prototypes -Wmissing-prototypes
INCLUDE := -isystem /lib/modules/`uname -r`/build/include -I../
CFLAGS  := -O2 -DMODULE -D__KERNEL__ ${WARN} ${INCLUDE}
CC      := gcc
	
${TARGET}.o: ${TARGET}.c ../memory.c

.PHONY: clean

clean:
	rm -rf ${TARGET}.o
