#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#ifdef VANILLA_AFL
#  define DFSAN
#endif 

#ifdef DFSAN
#  include <sanitizer/dfsan_interface.h>
#endif

extern "C" {
  #include <stdint.h>
  #include <unistd.h>
  #include <string.h>
  #include <assert.h>
#if !defined MSAN && !defined DFSAN
  #include "decode_inputs.h"
#endif
}

#ifdef DFSAN
    dfsan_label secret_label = 1;
#endif

class PasswordManager {
private:
	std::string password;
	int invalidTries;
	int maximumTries;
	bool loggedIn;
	
public:
	PasswordManager() {
		password = "supersecret";
		invalidTries = 0;
		maximumTries = 10;
		loggedIn = false;
	}

	PasswordManager(int max) {
		password = "supersecret";
		invalidTries = 0;
		maximumTries = max;
		loggedIn = false;
	}

	void tryLogin(std::string tryedPassword) {
		if (invalidTries < maximumTries) {
			if (password == tryedPassword) {
				loggedIn = true;
			} else {
				loggedIn = false;
				invalidTries++;
			}
		}
    std::string message = "Login Attempt Completed";
#ifdef DFSAN
    assert(!dfsan_has_label(dfsan_read_label(&message, sizeof(message)), secret_label));
#endif
		std::cout << message << std::endl;
	}
};


#ifdef DFSAN
int main(void) {

    char *Data = (char *)malloc(1024*1024+1);
    int length = read(STDIN_FILENO, Data, 1024*1024+1);
    if (length == -1 || length == 1024*1024+1) {
        printf("Error! too long\n");
        exit(1);
    }
#else
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t length) {
#endif
  
  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;
#ifdef DFSAN
    public_in = (uint8_t *)Data;
    public_len = length < 8 ? length : 8;
    secret_in = (uint8_t *)Data + public_len;
    secret_len = length - public_len;
#else
    find_public_and_secret_inputs((const char *)Data, length, &public_in, &public_len, &secret_in, &secret_len);
    if (!public_in || !secret_in) {
      printf("Failed to parse public / secret inputs JSON (expected \'{\"PUBLIC\": \"base64_input\", \"SECRET\": \"base64_input\"}\')\n");
      free(public_in);
      free(secret_in);
      return 1;
    }
#endif

  int passwordAttempts = 0;
  for (int i = 0; i < (secret_len < 4 ? secret_len : 4); i++) {
      passwordAttempts |= secret_in[i] << 8 * i;
  }

  const char *exitKeyword = "exit";
  bool exit = false;
	
  PasswordManager pm = PasswordManager(passwordAttempts);
	
  std::cout << "To exit, type: " << exitKeyword << std::endl;

  char *str = (char *)malloc(public_len + 1);
  memcpy(str, public_in, public_len + 1);
  str[public_len] = 0; // terminate the string

#ifdef DFSAN
    dfsan_set_label(secret_label, &passwordAttempts, sizeof(passwordAttempts));
#endif

  char *input, *temp;
  input = strtok_r(str, "\n", &temp);
  while (input) {
    std::cout << "Enter password:" << std::endl;

    exit |= strcmp(input, exitKeyword) == 0;
    exit |= input == exitKeyword;
    pm.tryLogin(input);
    input = strtok_r(NULL, "\n", &temp);
  }

  std::cout << "Run completed, run again" << std::endl;

#ifdef DFSAN
    // no need to free anything
#else
  free(public_in);
  free(secret_in);
#endif

  return 0;
}
