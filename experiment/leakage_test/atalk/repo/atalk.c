#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "atalk.h"

#ifndef VANILLA_AFL
  #include "json.h"
  #include "base64.h"
  #include "decode_inputs.h"
  #include "memory.h"
#endif

__attribute__((optnone))
int atalk_getname(atptr sock, atptr uaddr, int peer) {
/*
    atptr sock:  low input
    int peer: low input
    atptr uaddr: low output
    secret: is the machine information
*/
  struct atalk_sock sat;
	int err = -ENOBUF;
	if (sock_flag(sock))
		goto out;

	if (peer)
	{
		err = -ENOTCON;
		if (sock->sk_state != TCP_ESTABLISHED)
			goto out;
		sat.src_node  	= sock->dst_node;
		sat.src_port	= sock->dst_port;
		sat.dst_node 	= sock->src_node;
		sat.dst_port	= sock->src_port;
	} else {
		sat.src_node  	= sock->src_node;
		sat.src_port	= sock->src_port;
		sat.dst_node 	= sock->dst_node;
		sat.dst_port	= sock->dst_port;
	}

  sat.sk_state = sock->sk_state;
	memcpy(uaddr, &sat, sizeof(sat));
	err = sizeof(struct atalk_sock);

out:
	return err;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t length) {

#ifdef VANILLA_AFL
  if (length < sizeof(struct atalk_sock)) {
    return 1;
  }
  atptr sock = (atptr)Data;
#else
  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;
  find_public_and_secret_inputs(Data, length, &public_in, &public_len, &secret_in, &secret_len);
  if (!public_in || !secret_in) {
    printf("Failed to parse public / secret inputs JSON (expected \'{\"PUBLIC\": \"base64_input\", \"SECRET\": \"base64_input\"}\')\n");
    return 1;
  }

  if (public_len != sizeof(struct atalk_sock) + 4) {
    printf("Expected public len %lu, was %u bytes\n", sizeof(struct atalk_sock) + 4, public_len);
    exit(1);
  }

  uint32_t seed = 0;
  for (int i = 0; i < (secret_len < 4 ? secret_len : 4); i++) {
      seed |= secret_in[i] << 8 * i;
  }

  atptr sock = (atptr)public_in;
#endif

  atptr uaddr = (atptr)calloc(1, sizeof(struct atalk_sock));
  if (!uaddr) goto cleanup;
  int peer = *(int *)(Data + sizeof(struct atalk_sock)) % 2;

#ifndef VANILLA_AFL
  SEED_MEMORY(seed);
  FILL_STACK();
  // Push the stack down some so we know the next function call will be filled
  volatile int *__stack_bumper = (int *)alloca(100);
  __stack_bumper[0] = 1;
#endif

  int res = atalk_getname(sock, uaddr, peer);

  printf("%d\n", res);
  for (int i = 0; i < sizeof(* uaddr); i++) {
    printf("%02hhX", ((char *)uaddr)[i]);
  }
  printf("\n");
  free(uaddr);
cleanup:
#ifndef VANILLA_AFL
  free(public_in);
  free(secret_in);
#endif

  return 0;
}
