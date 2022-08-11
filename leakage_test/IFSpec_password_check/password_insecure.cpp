#include <string>
#include <iostream>
#include <cstdio>

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
	std::string exitKeyword = "exit";
	bool exit = false;
	
	PasswordManager pm = PasswordManager();
	
	std::cout << "To exit, type: " << exitKeyword << std::endl;

	std::string input;

	while (!exit) {
		std::cout << "Enter password:" << std::endl;
		std::getline(std::cin, input);

		if (!std::cin) {
			std::cout << "Run completed, run again" << std::endl;
			return 0;
		}

		exit |= input == exitKeyword;
		pm.tryLogin(input);
		
		std::cout << "Run completed, run again" << std::endl;
	}
}

