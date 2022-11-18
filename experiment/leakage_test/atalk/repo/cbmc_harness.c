#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "atalk.h"

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

void cbmc_test(struct atalk_sock sock, int peer) {
  struct atalk_sock *out1, *out2;

  out1 = (struct atalk_sock *)malloc(sizeof(struct atalk_sock));
  if (!out1) return;
  out2 = (struct atalk_sock *)malloc(sizeof(struct atalk_sock));
  if (!out2) { 
    free(out1);
    return;
  }

  memset(out1, 0, sizeof(*out1));
  memset(out2, 0, sizeof(*out2));

  int res1 = atalk_getname(&sock, out1, peer);
  int res2 = atalk_getname(&sock, out2, peer);

  assert(res1 == res2);
  assert(memcmp(out1, out2, sizeof(*out1)) == 0);

  free(out1);
  free(out2);
}

