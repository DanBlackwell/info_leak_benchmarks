#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
extern "C" {
#include "json.h"
#include "base64.h"
#include "atalk.h"
#include "decode_inputs.h"
#include "memory.h"
}

int atalk_getname(atptr sock, atptr &uaddr, int peer) {
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
	err = sizeof(atalk_sock);

out:
	return err;
}

int main(int argc, char *argv[])
{

  char *Data = (char *)malloc(1024*1024+1);
  int length = read(STDIN_FILENO, Data, 1024*1024+1);
  if (length == -1 || length == 1024*1024+1) {
    printf("Error! too long\n");
    exit(1);
  }

  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;
  find_public_and_secret_inputs(Data, length, &public_in, &public_len, &secret_in, &secret_len);

  if (public_len != sizeof(struct atalk_sock) + 4) {
    printf("Expected public len %lu, was %u bytes\n", sizeof(struct atalk_sock) + 4, public_len);
    exit(1);
  }

  uint32_t seed = 0;
  for (int i = 0; i < (secret_len < 4 ? secret_len : 4); i++) {
      seed |= secret_in[i] << 8 * i;
  }

//  short pos;
//  uintptr_t stack_size = (uintptr_t)&pos - (uintptr_t)get_min_stack_bottom();
//
//  printf("stack: ");
//  for (int i = 0; i < stack_size; i++) {
//    if (i % 64 == 0)
//      printf("\n");
//    printf("%02hhx", *((char *)&pos - i)); 
//  }
//  return 0;

  atptr sock = (atptr)public_in;
  atptr uaddr = (atptr)calloc(1, sizeof(struct atalk_sock));
  int peer = *(int *)(Data + sizeof(struct atalk_sock)) % 2;

  SEED_MEMORY(seed);
  fill_stack();

  int res = atalk_getname(sock, uaddr, peer);

  printf("%d\n", res);
  for (int i = 0; i < sizeof(* uaddr); i++) {
    printf("%02hhX", ((char *)uaddr)[i]);
  }
  printf("\n");
  free(uaddr);

  return 0;
}

