#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "decode_inputs.h"

void print_boring() {
  printf("10\n");
}

int main(void) {
  uint32_t secret, public;

  uint8_t *data = malloc(65536);
  int length = read(STDIN_FILENO, data, 65536);
	
  if (length >= 65536) {
    printf("Input longer than 65536!\n");
    return 1;
  }

  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;

  find_public_and_secret_inputs(data, length,
                                &public_in, &public_len,
                                &secret_in, &secret_len);
  
  if (public_len != 4) {
    printf("expected public_len == 4, got %u\n", public_len);
    return 1;
  }

  if (secret_len != 4) {
    print_boring();
    return 1;
  }

  public = *(uint32_t *)public_in;
  secret = *(uint32_t *)secret_in;

  if (public > UINT32_MAX / 2) {
    printf("%d\n", secret);
  } else {
    print_boring();
  }

  free(public_in);
  free(secret_in);

}
