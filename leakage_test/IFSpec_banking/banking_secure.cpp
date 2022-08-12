#include <iostream>
#include <string>
#include <cstdio>
extern "C" {
  #include <unistd.h>
  #include <stdint.h>
  #include <stdlib.h>
  #include "decode_inputs.h"
}

class ErrorLog {
public:
    void logError(std::string message) {
        std::cout << message << std::endl;
    }
};

class TransactionLog {
public:
    void logTransaction(std::string message) {}
};

class Account {
public:
    double balance;
    ErrorLog errorLog;
    TransactionLog transactionLog;

    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            logTransaction(true);
        } else {
            logError("Cannot deposit a non-positive amount.");
        }
    }

    bool withdraw(double amount) {
        if (amount > 0) {
            double newAmount = balance - amount;
            if (newAmount > 0) {
                balance = newAmount;
                logTransaction(false);
                return true;
            } else {
                return false;
            }
        }
        logError("Cannot withdraw a non-positive amount.");
        return false;
    }

    void logError(std::string message) {
        errorLog.logError(message);
    }

private:
    void logTransaction(bool isDeposit) {
        std::string transaction = isDeposit ? "Deposit" : "Withdrawal";
        char buf[120];
        sprintf(buf, "%s completed, new balance: %.2f", transaction.c_str(), balance);
        transactionLog.logTransaction(buf);
    }

};

class Beneficiary {
public:
    double received;

    void receive(double amount) {
        received += amount;
    }
};

class AccountOwner {
private:
    Account account;

public:
    AccountOwner(Account account) {
        this->account = account;
    }

    void payBeneficiary(Beneficiary b, double amount) {
        bool transactionPossible = account.withdraw(amount);
        if (transactionPossible) {
            b.receive(amount);
        }
    }
};

union converter {
  long intVal;
  double floatVal;
} converter;

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

    converter.intVal = 0;
    for (int i = 0; i < (secret_len < 8 ? secret_len : 8); i++) {
        converter.intVal |= secret_in[i] << (8 * i);
    }
    double deposit = converter.floatVal;
    
    converter.intVal = 0;
    for (int i = 0; i < (public_len < 8 ? public_len : 8); i++) {
        converter.intVal |= public_in[i] << (8 * i);
    }
    double transfer = converter.floatVal;
  
    Account account = Account();
    account.deposit(deposit);
    AccountOwner owner = AccountOwner(account);
    Beneficiary beneficiary = Beneficiary();
    owner.payBeneficiary(beneficiary, transfer);
}

