#include "skbuff.h"
#include "usbnet.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifndef VANILLA_AFL
  #include "decode_inputs.h"
  #include "memory.h"
#endif

#define	SR_RX_OVERHEAD		7	/* 3bytes header + 4crc tail */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */
#define unlikely(cond) cond


typedef enum {
	GFP_KERNEL,
	GFP_ATOMIC,
	__GFP_HIGHMEM,
	__GFP_HIGH
} gfp_t;


void netdev_err(struct net_device *dev, const char *err) {
  printf("%s", err);
}


static inline void *__skb_pull(struct sk_buff *skb, unsigned int len)
{
	skb->len -= len;
	assert(skb->len >= skb->data_len);
	return skb->data += len;
}

static inline void *skb_pull_inline(struct sk_buff *skb, unsigned int len)
{
	return unlikely(len > skb->len) ? NULL : __skb_pull(skb, len);
}

void *skb_pull(struct sk_buff *skb, unsigned int len)
{
	return skb_pull_inline(skb, len);
}


static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
	skb->tail = skb->data;
}

static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb_reset_tail_pointer(skb);
	skb->tail += offset;
}


struct sk_buff *skb_clone(struct sk_buff *skb, gfp_t gfp_mask)
{
  struct sk_buff *clone = malloc(sizeof(struct sk_buff));
  memcpy(clone, skb, sizeof(struct sk_buff));
  return clone;
}

static int sr9700_rx_fixup(struct usbnet *dev, struct sk_buff *skb)
{
	struct sk_buff *sr_skb;
	int len;

	/* skb content (packets) format :
	 *                    p0            p1            p2    ......    pm
	 *                 /      \
	 *            /                \
	 *        /                            \
	 *  /                                        \
	 * p0b0 p0b1 p0b2 p0b3 ...... p0b(n-4) p0b(n-3)...p0bn
	 *
	 * p0 : packet 0
	 * p0b0 : packet 0 byte 0
	 *
	 * b0: rx status
	 * b1: packet length (incl crc) low
	 * b2: packet length (incl crc) high
	 * b3..n-4: packet data
	 * bn-3..bn: ethernet packet crc
	 */
	if (unlikely(skb->len < SR_RX_OVERHEAD)) {
		netdev_err(dev->net, "unexpected tiny rx frame\n");
		return 0;
	}

	/* one skb may contains multiple packets */
	while (skb->len > SR_RX_OVERHEAD) {
		if (skb->data[0] != 0x40)
			return 0;

		/* ignore the CRC length */
		len = (skb->data[1] | (skb->data[2] << 8)) - 4;

		if (len > ETH_FRAME_LEN)
			return 0;

		/* the last packet of current skb */
		if (skb->len == (len + SR_RX_OVERHEAD))	{
			skb_pull(skb, 3);
			skb->len = len;
			skb_set_tail_pointer(skb, len);
			skb->truesize = len + sizeof(struct sk_buff);
			return 2;
		}

		/* skb_clone is used for address align */
		sr_skb = skb_clone(skb, GFP_ATOMIC);
		if (!sr_skb)
			return 0;

		sr_skb->len = len;
		sr_skb->data = skb->data + 3;
		skb_set_tail_pointer(sr_skb, len);
		sr_skb->truesize = len + sizeof(struct sk_buff);

		for (int i = 0; i < sizeof(sr_skb); i++) {
		  printf("%02X", ((uint8_t *)sr_skb)[i]);
		}
		printf("\n");
//		usbnet_skb_return(dev, sr_skb);

		skb_pull(skb, len + SR_RX_OVERHEAD);
	}

	return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  struct sk_buff *skb;
  struct usbnet dev;

#ifdef VANILLA_AFL
  uint8_t *public_in = (uint8_t *)Data;
  uint32_t public_len = Size;
#else
  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;
  find_public_and_secret_inputs((const char *)Data, Size, &public_in, &public_len, &secret_in, &secret_len);
  if (!public_in || !secret_in) {
    printf("Failed to parse public / secret inputs JSON (expected \'{\"PUBLIC\": \"base64_input\", \"SECRET\": \"base64_input\"}\')\n");
    return 1;
  }

  uint32_t seed = 0;
  for (uint32_t i = 0; i < (secret_len < 4 ? secret_len : 4); i++) {
      seed |= secret_in[i] << 8 * i;
  }

  SEED_MEMORY(seed);
  fill_stack();
#endif
  int pos = 0;

  size_t requiredSize = sizeof(struct sk_buff) + 50;
  if (public_len < requiredSize) {
    printf("Expected public len %lu, was %u bytes\n", requiredSize, public_len);
    return 1;
  }

  while (public_len > pos && public_len - pos > requiredSize) {
    skb = (struct sk_buff *)(public_in + pos);
    pos += sizeof(struct sk_buff);
    skb->len = public_in[pos] % 50;
    pos += 1;
    skb->data = public_in + pos;
    pos += skb->len;
    dev = (struct usbnet){0};
  
    sr9700_rx_fixup(&dev, skb);
  }

  return 0;
}
