#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#ifndef VANILLA_AFL
  #include "decode_inputs.h"
#endif

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


int LLVMFuzzerTestOneInput(const uint8_t *data, uint32_t length) {
  int h;
  int64_t ppos;

  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;

#ifdef VANILLA_AFL
  if (length != sizeof(ppos) + sizeof(h)) {
    return 1;
  }
  public_in = data;
  public_len = sizeof(ppos);
  secret_in = data + public_len;
  secret_len = sizeof(h);
#else
  find_public_and_secret_inputs((const char *)data, length,
                                &public_in, &public_len,
                                &secret_in, &secret_len);
  if (!public_in || !secret_in) {
    printf("Failed to parse public / secret inputs JSON (expected \'{\"PUBLIC\": \"base64_input\", \"SECRET\": \"base64_input\"}\')\n");
    return 1;
  }
#endif
  
  if (public_len != sizeof(ppos)) {
    printf("expected public_len == %lu, got %u\n", sizeof(ppos), public_len);
    return 1;
  }

  if (secret_len != sizeof(h)) {
    print_boring();
    return 1;
  }

  h = *(int *)secret_in;
  ppos = *(int64_t *)public_in;

  printf("%d\n", underflow(h, ppos));
  return 0;
}
