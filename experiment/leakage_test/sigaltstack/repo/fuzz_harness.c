#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define SS_ONSTACK      1
#define SS_DISABLE      2

#define MINSIGSTKSZ     2048

#define EPERM            1      /* Operation not permitted */
#define ENOMEM          12      /* Out of memory */
#define EFAULT          14      /* Bad address */
#define EINVAL          22      /* Invalid argument */

typedef struct task_struct_t {
	void *sas_ss_sp;
	// char *sas_ss_sp;
	size_t sas_ss_size;
} task_struct;

task_struct _current;
task_struct *current = &_current;

typedef struct sigaltstack {
	void /*__user*/ *ss_sp;
	int ss_flags; // 4 bytes padding on 64-bit systems
	size_t ss_size;
} stack_t;

static inline int on_sig_stack(unsigned long sp)
{
	// return 1;
	return (sp - (uintptr_t)current->sas_ss_sp < current->sas_ss_size);
}

static inline int sas_ss_flags(unsigned long sp)
{
        return (current->sas_ss_size == 0 ? SS_DISABLE
                : on_sig_stack(sp) ? SS_ONSTACK : 0);
}

int copy_to_user(void *user_dest, void *kernel_buf, size_t size)
{
  write(1, kernel_buf, size);
  putc('\n', stdout);
  return 0;
}

// uss: user signal-stack
// uoss: user old signal-stack
int
do_sigaltstack (const stack_t /*__user*/ *uss, stack_t /*__user*/ *uoss, unsigned long sp)
{
	stack_t oss;
	int error;

	if (uoss) {
		oss.ss_sp = (void /*__user*/ *) current->sas_ss_sp;
		oss.ss_size = current->sas_ss_size;
		oss.ss_flags = sas_ss_flags(sp);
	}

	if (uss) {
		void /*__user*/ *ss_sp;
		size_t ss_size;
		int ss_flags;

		error = -EFAULT;
//		if (!access_ok(VERIFY_READ, uss, sizeof(*uss))
//		    || __get_user(ss_sp, &uss->ss_sp)
//		    || __get_user(ss_flags, &uss->ss_flags)
//		    || __get_user(ss_size, &uss->ss_size))
//			goto out;
		ss_sp = uss->ss_sp;
		ss_flags = uss->ss_flags;
		ss_size = uss->ss_size;

		error = -EPERM;
		if (on_sig_stack(sp))
			goto out;

		error = -EINVAL;
		/*
		 *
		 * Note - this code used to test ss_flags incorrectly
		 *  	  old code may have been written using ss_flags==0
		 *	  to mean ss_flags==SS_ONSTACK (as this was the only
		 *	  way that worked) - this fix preserves that older
		 *	  mechanism
		 */
		if (ss_flags != SS_DISABLE && ss_flags != SS_ONSTACK && ss_flags != 0)
			goto out;

		if (ss_flags == SS_DISABLE) {
			ss_size = 0;
			ss_sp = NULL;
		} else {
			error = -ENOMEM;
			if (ss_size < MINSIGSTKSZ)
				goto out;
		}

		current->sas_ss_sp = ss_sp;
		current->sas_ss_size = ss_size;
	}

	if (uoss) {
		error = -EFAULT;
		if (copy_to_user(uoss, &oss, sizeof(oss)))
			goto out;
	}

	error = 0;
out:
	return error;
}


///////////////////////////////////////////////////////////////////////////////
// FUZZ HARNESS
///////////////////////////////////////////////////////////////////////////////

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

uint8_t *stack;

int main(int argc, char **argv) 
{
  __AFL_INIT();
	
  unsigned char *Data = __AFL_FUZZ_TESTCASE_BUF;  // must be after __AFL_INIT
  int length = __AFL_FUZZ_TESTCASE_LEN;

  static uint8_t *in;
  static uint32_t len;

  static stack_t uss = {0};

#ifdef VANILLA_AFL
  if (length < sizeof(uss.ss_flags)) {
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
  if (public_len < sizeof(uss.ss_flags)) {
  	return 1;
  }

  static uint32_t seed = 0;
  for (int i = 0; i < (secret_len < 4 ? secret_len : 4); i++) {
      seed |= secret_in[i] << 8 * i;
  }
  SEED_MEMORY(seed);

  in = public_in; len = public_len;
#endif

  // handle PUBLIC

  static int pos = 0;
  uss.ss_flags = *(int *)in;
  pos += sizeof(uss.ss_flags);
  uss.ss_size = len - pos;
  // just be safe and make a copy of public_in in case we overwrite the shared mem somehow
  stack = malloc(len - pos);
  memcpy(stack, in + pos, len - pos);
  uss.ss_sp = stack;
  
  static stack_t uoss = {0};
  
  current->sas_ss_sp = &uoss;
  current->sas_ss_size = 1024;

#ifndef VANILLA_AFL
  FILL_STACK();
#endif

  // execute the function
  do_sigaltstack(&uss, &uoss, 0);

  return 0;
}


// #include "base64.h"

// int main() {
//     stack_t uss = {0};
// 	uss.ss_flags = 0;

// 	long stack = 0x1234567890abcdef;

// 	char buf[sizeof(uss.ss_flags) + sizeof(stack)];
// 	memcpy(buf, &uss.ss_flags, sizeof(uss.ss_flags));
// 	memcpy(buf + sizeof(uss.ss_flags), &stack, sizeof(stack));

// 	int enc_len = Base64encode_len(sizeof(buf));
// 	char *encoded = malloc(enc_len);
// 	int res = Base64encode(encoded, buf, sizeof(buf));
//         printf("{\n  \"PUBLIC\": \"");
// 	for (int i = 0; i < res; i++) {
// 		printf("%c", encoded[i]);
// 	}
//         printf("\",\n  \"SECRET\": \"MDAwMA==\"\n}\n");
// 	printf("\npredicted len: %d, actual: %d\n", enc_len, res);
// }
