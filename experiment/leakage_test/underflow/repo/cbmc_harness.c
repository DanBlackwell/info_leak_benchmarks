#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

int underflow(int h, int64_t ppos)
{
    int bufsz;
    uint32_t nbytes;
    bufsz  = 1024;
    nbytes = 20;

    if (ppos + nbytes > bufsz)     // (A)
        nbytes = bufsz - ppos;     // (B)

    if (ppos + nbytes > bufsz){
        return h; // (C)
    } else {
        return 0;
    }
}

void print_boring() {
  printf("0\n");
}

void cbmc_test() {
  int64_t ppos;
  int h1, h2;

  assert(underflow(ppos, h1) == underflow(ppos, h2));
}
