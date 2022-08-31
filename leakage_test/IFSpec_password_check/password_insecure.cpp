#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
extern "C" {
  #include <stdint.h>
  #include <unistd.h>
  #include <string.h>
  #include "decode_inputs.h"
}

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
		maximumTries = 10; // THIS IS THE SECRET!!!
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
				invalidTries=0;
			} else {
				loggedIn = false;
				invalidTries++;
			}
		} else {
			std::cout << "No more password tries allowed" << std::endl;
		}
	}
};

int main(void) {
  char *Data = (char *)malloc(1024*1024+1);
  int length = read(STDIN_FILENO, Data, 1024*1024+1);
  if (length == -1 || length == 1024*1024+1) {
    printf("Error! too long\n");
    exit(1);
  }
  
  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;
  find_public_and_secret_inputs(Data, length, &public_in, &public_len, &secret_in, &secret_len);
  if (!public_in || !secret_in) {
    printf("Failed to parse public / secret inputs JSON (expected \'{\"PUBLIC\": \"base64_input\", \"SECRET\": \"base64_input\"}\')\n");
    return 1;
  }


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

  char *input, *temp;
  input = strtok_r(str, "\n", &temp);
  do {
		std::cout << "Enter password:" << std::endl;

    exit |= strcmp(input, exitKeyword) == 0;
		exit |= input == exitKeyword;
		pm.tryLogin(input);
  } while ((input = strtok_r(NULL, "\n", &temp)) != NULL);

  std::cout << "Run completed, run again" << std::endl;
	return 0;
}
