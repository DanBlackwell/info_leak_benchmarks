#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "decode_inputs.h"

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

int main(void) {
  int h;
  int64_t ppos;

  uint8_t *data = malloc(65536);
  int length = read(STDIN_FILENO, data, 65536);
	
  if (length >= 65536) {
    printf("Input longer than 65536!\n");
    return 1;
  }

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
