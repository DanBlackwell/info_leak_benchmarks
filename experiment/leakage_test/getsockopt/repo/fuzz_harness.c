#include "getsockopt.c"

#include "memory.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#ifndef VANILLA_AFL
  #include "json.h"
  #include "base64.h"
  #include "decode_inputs.h"
  #include "memory.h"
#endif

__AFL_FUZZ_INIT();

// int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t length) {
int main() {

#ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();
#endif

  unsigned char *Data = __AFL_FUZZ_TESTCASE_BUF;  // must be after __AFL_INIT
  int length = __AFL_FUZZ_TESTCASE_LEN;

  static uint8_t *in;
  static uint32_t len;
#ifdef VANILLA_AFL
  if (length < sizeof(struct hci_conn) + sizeof(struct sco_conn) + sizeof(struct sco_pinfo) + 1) {
    return 1;
  }
  in = Data; len = length;
#else
  static uint8_t *public_in, *secret_in;
  static uint32_t public_len, secret_len;
  find_public_and_secret_inputs(Data, length, &public_in, &public_len, &secret_in, &secret_len);
  if (!public_in || !secret_in) {
    printf("Failed to parse public / secret inputs JSON (expected \'{\"PUBLIC\": \"base64_input\", \"SECRET\": \"base64_input\"}\')\n");
    return 1;
  }

  // handle PUBLIC
  if (public_len < sizeof(struct hci_conn) + sizeof(struct sco_conn) + sizeof(struct sco_pinfo) + 1) {
    return 1;
  }

  static uint32_t seed = 0, i = 0;
  for (i = 0; i < (secret_len < 4 ? secret_len : 4); i++) {
      seed |= secret_in[i] << 8 * i;
  }
  printf("added seed\n");

  SEED_MEMORY(seed);
  FILL_STACK();

  printf("filled stack\n");

  in = public_in; len = public_len;
#endif


  // static struct sock mySock;
  // memcpy(&mySock, EXPLICIT_PUBLIC_IN, sizeof(mySock));
  // static struct socket mySocket = {0};
  // mySocket.sk = &mySock;
  // static char optname; optname = *(EXPLICIT_PUBLIC_IN + sizeof(mySock));
  // static char optval[80];
  // static int optlen = sizeof(optval);

  static int pos = 0;

  static struct hci_conn myHciConn;
  myHciConn = *(struct hci_conn *)(in);
  pos += sizeof(myHciConn);

  static struct sco_conn myScoConn;
  myScoConn = *(struct sco_conn *)(in + pos);
  pos += sizeof(myScoConn);
  myScoConn.hcon = &myHciConn;

  static struct sco_pinfo myPinfo = {};
  myPinfo = *(struct sco_pinfo *)(in + pos);
  pos += sizeof(myPinfo);
  myPinfo.conn = &myScoConn;

  static struct sock *mySock = (struct sock *)&myPinfo;
  static struct socket mySocket = {0};
  mySocket.sk = mySock;

  static int optname;
  optname = *(in + pos);
  static char optval[80];
  static int optlen = sizeof(optval);

  printf("about to call func\n");

  static int res;
  res = sco_sock_getsockopt_old(&mySocket, optname, optval, &optlen);
  printf("called func, writing: %lu\n", sizeof(res) + optlen);
  write(1, &res, sizeof(res));
  write(1, optval, optlen);

  return 0;
}
