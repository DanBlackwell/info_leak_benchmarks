#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#ifdef VANILLA_AFL
#  define DFSAN
#endif 

#ifdef DFSAN
#  include <sanitizer/dfsan_interface.h>
#  include <assert.h>
#endif

#ifndef VANILLA_AFL
  #include "decode_inputs.h"
#endif

#ifdef DFSAN
  dfsan_label secret_label = 1;
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


#ifdef DFSAN
int main(void) {

    char *Data = (char *)malloc(1024*1024+1);
    int length = read(STDIN_FILENO, Data, 1024*1024+1);
    if (length == -1 || length == 1024*1024+1) {
        printf("Error! too long\n");
        exit(1);
    }
#else
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t length) {
#endif
  int h = 0;
  int64_t ppos;

  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;

#ifdef DFSAN
  if (length != sizeof(ppos) + sizeof(h)) {
    return 1;
  }
  public_in = (uint8_t *)data;
  public_len = sizeof(ppos);
  secret_in = (uint8_t *)data + public_len;
  secret_len = sizeof(h);

  dfsan_set_label(secret_label, secret_in, secret_len);
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

  if (secret_len >= sizeof(h)) {
    h = *(int *)secret_in;
  }

  ppos = *(int64_t *)public_in;

  int res = underflow(h, ppos);

  printf("%d\n", res);

#ifdef DFSAN
  assert(!dfsan_has_label(dfsan_read_label(res, sizeof(res)), secret_label));
#endif

  return 0;
}
