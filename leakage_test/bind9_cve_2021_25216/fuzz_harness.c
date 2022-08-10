#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include "decode_inputs.h"
#include "memory.h"

typedef struct oid {
    size_t length;
    unsigned *components;
} oid;

typedef enum asn1_error_number {
	ASN1_BAD_TIMEFORMAT = 1859794432,
	ASN1_MISSING_FIELD = 1859794433,
	ASN1_MISPLACED_FIELD = 1859794434,
	ASN1_TYPE_MISMATCH = 1859794435,
	ASN1_OVERFLOW = 1859794436,
	ASN1_OVERRUN = 1859794437,
	ASN1_BAD_ID = 1859794438,
	ASN1_BAD_LENGTH = 1859794439,
	ASN1_BAD_FORMAT = 1859794440,
	ASN1_PARSE_ERROR = 1859794441
} asn1_error_number;


static void
free_oid(oid *k) {
	free(k->components);
	k->components = NULL;
}


static int
der_get_oid(const unsigned char *p, size_t len, oid *data, size_t *size) {
	int n;
	size_t oldlen = len;

	data->components = NULL;
	data->length = 0;
	if (len < 1U) {
		return (ASN1_OVERRUN);
	}

	data->components = malloc((len + 1) * sizeof(*data->components));
	if (data->components == NULL) {
		return (ENOMEM);
	}
	data->components[0] = (*p) / 40;
	data->components[1] = (*p) % 40;
	--len;
	++p;
	for (n = 2; len > 0U; ++n) {
		unsigned u = 0;

		do {
			--len;
			u = u * 128 + (*p++ % 128);
		} while (len > 0U && p[-1] & 0x80);
		data->components[n] = u;
	}
	if (p[-1] & 0x80) {
		free_oid(data);
		return (ASN1_OVERRUN);
	}
	data->length = n;
	if (size) {
		*size = oldlen;
	}
	return (0);
}

__AFL_FUZZ_INIT();

int main(void) {

  __AFL_INIT();

  char *data = __AFL_FUZZ_TESTCASE_BUF;

  while(__AFL_LOOP(1000)) {
    int len = __AFL_FUZZ_TESTCASE_LEN;

    uint8_t *public_in, *secret_in;
    uint32_t public_len, secret_len;

    find_public_and_secret_inputs(data, len,
                                  &public_in, &public_len,
                                  &secret_in, &secret_len);

    uint32_t seed = 0;
    for (int i = 0; i < (secret_len < 4 ? secret_len : 4); i++) {
        seed |= secret_in[i] << 8 * i;
    }

    oid target;
    size_t size;

    SEED_MEMORY(seed);
    fill_stack();

    if (!der_get_oid(public_in, public_len, &target, &size)) {
      for (int i = 0; i < target.length; i++) {
        printf("%d: %u\n", i, target.components[i]);
      }

      free_oid(&target);
    }

    free(public_in);
    free(secret_in);
  }
}
