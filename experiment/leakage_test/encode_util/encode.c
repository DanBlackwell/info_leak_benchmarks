#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

#define PUBLIC_KEY "PUBLIC"
#define SECRET_KEY "SECRET"

int main(void) {
  char *c;
  char **combined_buf = &c;
  int l;
  int *combined_buf_len = &l;

  char secret_input[80], public_input[80];

  printf("enter secret:\n");
  scanf("%s", secret_input);
  printf("enter public:\n");
  scanf("%s", public_input);

  int public_input_len = strlen(public_input);
  int secret_input_len = strlen(secret_input);

  const char *json_out_template = "{\n  \"" PUBLIC_KEY "\": \"%s\",\n  \"" SECRET_KEY "\": \"%s\"\n}";

  unsigned expected_len = strlen(json_out_template) +
                         Base64encode_len((int)public_input_len) +
                         Base64encode_len((int)secret_input_len);

  char *encoded_public = malloc(expected_len);
  Base64encode(encoded_public, public_input, (int)public_input_len);

  char *encoded_secret = malloc(expected_len);
  Base64encode(encoded_secret, secret_input, (int)secret_input_len);

  printf("encoded_pub: %s\nencoded_sec: %s\n", encoded_public, encoded_secret);

  printf("template: %s\nexpected_len: %u\n\n", json_out_template, expected_len);

  *combined_buf = malloc(expected_len);
  *combined_buf_len = snprintf(*combined_buf,
                               expected_len,
                               json_out_template,
                               encoded_public,
                               encoded_secret);

  if (*combined_buf_len >= expected_len) {
    printf("Would expect the output str to be shorter than %u characters, \nRAW: %s", expected_len, *combined_buf);
    exit(1);
  }

  free(encoded_public);
  free(encoded_secret);

  printf("Combined_buf (len: %u): %s\n", *combined_buf_len, *combined_buf);

}
