#include <iostream>
#include <string>
#include <cstdio>

#ifdef DFSAN
#  include <sanitizer/dfsan_interface.h>
#endif

extern "C" {
  #include <unistd.h>
  #include <stdint.h>
  #include <stdlib.h>
  #include <string.h>
#if !defined MSAN && !defined DFSAN
  #include "decode_inputs.h"
#endif
}

#ifdef DFSAN
    dfsan_label secret_label = 1;
#endif

class ErrorLog {
public:
    void logError(std::string message) {
#ifdef DFSAN
        assert(!dfsan_has_label(dfsan_read_label(&message, sizeof(message)), secret_label));
#endif
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
                char buf[2048];
                sprintf(buf, "Account has insufficient funds to withdraw %.2f", amount);
                logError(buf);
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
        char buf[2048];
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
  char bytes[sizeof(double)];
  double floatVal;
} converter;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t length) {

    uint8_t *public_in, *secret_in;
    uint32_t public_len, secret_len;
#if defined MSAN || defined DFSAN
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

    for (int i = 0; i < (secret_len < 8 ? secret_len : 8); i++) {
        converter.bytes[i] = secret_in[i];
    }
    double deposit = converter.floatVal;
    deposit = (deposit != deposit || deposit < 0.01) ? 0.01 : deposit;

    memset(converter.bytes, 0, sizeof(converter.bytes));
    for (int i = 0; i < (public_len < 8 ? public_len : 8); i++) {
        converter.bytes[i] = public_in[i];
    }
    double transfer = converter.floatVal;

#ifdef DFSAN
    dfsan_set_label(secret_label, &deposit, sizeof(deposit));
#endif

    Account account = Account();
    account.deposit(deposit);
    AccountOwner owner = AccountOwner(account);
    Beneficiary beneficiary = Beneficiary();
    owner.payBeneficiary(beneficiary, transfer);

#if defined MSAN || defined DFSAN
    // no need to free anything
#else
    free(public_in);
    free(secret_in);
#endif

    return 0;
}

