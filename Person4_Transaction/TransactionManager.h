#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include "Transaction.h"
#include <vector>
#include <string>

using namespace std;

class TransactionManager {
private:
    vector<Transaction> transactions;
    int nextTransactionNumber;

public:
    TransactionManager();

    string generateTransactionId();

    void addTransaction(
        string senderId,
        string receiverId,
        long long amountInPaise,
        TransactionType type,
        TransactionStatus status,
        string dateTime
    );

    void displayAllTransactions() const;

    void displayUserTransactions(string userId) const;

    void searchTransaction(string transactionId) const;

    int getTransactionCount() const;

    int getSuccessfulCount() const;
    int getFailedCount() const;
    int getRefundedCount() const;
    const vector<Transaction>& getAllTransactions() const;
};

#endif