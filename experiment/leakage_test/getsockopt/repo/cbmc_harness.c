#include "getsockopt.c"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

int main(void) 
{
  struct sock mySock;
  struct socket mySocket = { .sk = &mySock };

  int res1 = 0;
  char optname1;
  char optval1[80] = {0};
  int optlen1 = 0;
  res1 = sco_sock_getsockopt_old(&socket, optname1, optval1, &optlen1);

  int res2 = 0;
  char optname2;
  char optval2[80] = {0};
  int optlen2 = 0;
  res2 = sco_sock_getsockopt_old(&socket, optname2, optval2, &optlen2);

  assert(res1 == res2);
  assert(optlen1 == optlen2);
  assert(!memcmp(optval1, optval2, optlen1));
}
