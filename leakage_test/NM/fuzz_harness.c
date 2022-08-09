#include "nm-setting-vpn.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "decode_inputs.h"
#include "memory.h"

GType nm_setting_connection_get_type() {
  return 1;
}


char *fetchString(int minLen, int maxLen, const char *buf, int *bufLen, int *bufPos) {
  if (*bufPos == *bufLen) {
    char *string = calloc(1, 1);
    return string;
  }

  int stringLen = buf[*bufPos] % (maxLen - minLen) + minLen; // select len between 5 and 20
  (*bufPos)++;
  stringLen = *bufLen - *bufPos >= stringLen ? stringLen : *bufLen - *bufPos;

  char *string = calloc(stringLen + 1, 1);
  memcpy(string, buf + *bufPos, stringLen);
  (*bufPos) += stringLen;

  return string;
}

int main(void) {

  char *buf = (char *)malloc(1 * 1024 * 1024 + 1);
  int len = read(STDIN_FILENO, buf, 1024 * 1024 + 1);
  if (len == -1 || len == 1024*1024+1) {
    printf("Error! too long\n");
    exit(1);
  }

  NMSetting *setting;

  setting = nm_setting_vpn_new();

  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;
  find_public_and_secret_inputs(buf, len, &public_in, &public_len, &secret_in, &secret_len);

  uint32_t seed = 0;
  for (int i = 0; i < (secret_len < 4 ? secret_len : 4); i++) {
      seed |= secret_in[i] << 8 * i;
  }
 
  int pub_pos = 0, sec_pos = secret_len < 4 ? secret_len - 1 : 4;

  SEED_MEMORY(seed);
  fill_stack();

  while (pub_pos < public_len) {
    int fn = public_in[pub_pos++] % 7;
 
    switch(fn) {
      case 0:
        printf("user_name: %s\n", nm_setting_vpn_get_user_name(setting));
        break;
      case 1:
        printf("service_type: %s\n", nm_setting_vpn_get_service_type(setting));
        break;
      case 2:
        {
        char *key = fetchString(5, 20, public_in, &public_len, &pub_pos);
        char *item = fetchString(5, 20, public_in, &public_len, &pub_pos);
        printf("adding item %s: %s\n", key, item);
        nm_setting_vpn_add_data_item(setting, key, item);
        free(key);
        free(item);
        }
        break;
      case 3:
        {
        char *key = fetchString(5, 20, public_in, &public_len, &pub_pos);
        printf("item %s: %s\n", key, nm_setting_vpn_get_data_item(setting, key));
        free(key);
        }
        break;
      case 4:
        {
        char *key = fetchString(5, 20, public_in, &public_len, &pub_pos);
        printf("removing item %s\n", key);
        nm_setting_vpn_remove_data_item(setting, key);
        free(key);
        }
        break;
      case 5:
        {
        char *key = fetchString(5, 20, public_in, &public_len, &pub_pos);
        char *item = fetchString(5, 20, secret_in, &secret_len, &sec_pos);
        printf("adding secret %s: %s\n", key, item);
        nm_setting_vpn_add_secret(setting, key, item);
        free(key);
        free(item);
        }
        break;
      case 6:
        {
        char *key = fetchString(5, 20, public_in, &public_len, &pub_pos);
        printf("removing secret %s\n", key);
        nm_setting_vpn_remove_data_item(setting, key);
        free(key);
        }
        break;
    }
  }
  
}
