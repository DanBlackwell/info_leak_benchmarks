#include <iostream>
#include <string>

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
                char buf[120];
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

int main(void) {
    Account account = Account();
    account.deposit(100);
    AccountOwner owner = AccountOwner(account);
    Beneficiary beneficiary = Beneficiary();
    owner.payBeneficiary(beneficiary, 150);
}

