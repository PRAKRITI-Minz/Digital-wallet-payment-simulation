#include "TransactionManager.h"
#include <iostream>

using namespace std;

TransactionManager::TransactionManager() {
    nextTransactionNumber = 1001;
}

string TransactionManager::generateTransactionId() {
    return "T" + to_string(nextTransactionNumber++);
}

void TransactionManager::addTransaction(
    string senderId,
    string receiverId,
    long long amountInPaise,
    TransactionType type,
    TransactionStatus status,
    string dateTime
) {
    string transactionId = generateTransactionId();

    Transaction transaction(
        transactionId,
        senderId,
        receiverId,
        amountInPaise,
        type,
        status,
        dateTime
    );

    transactions.push_back(transaction);
}

void TransactionManager::displayAllTransactions() const {
    if (transactions.empty()) {
        cout << "\nNo transactions found.\n";
        return;
    }

    cout << "\n========== ALL TRANSACTIONS ==========\n";

    for (const Transaction& transaction : transactions) {
        transaction.display();
    }
}

void TransactionManager::displayUserTransactions(string userId) const {
    bool found = false;

    for (const Transaction& transaction : transactions) {
        if (transaction.getSenderId() == userId ||
            transaction.getReceiverId() == userId) {

            transaction.display();
            found = true;
        }
    }

    if (!found) {
        cout << "\nNo transactions found for User ID: "
             << userId << endl;
    }
}

void TransactionManager::searchTransaction(string transactionId) const {
    for (const Transaction& transaction : transactions) {

        if (transaction.getTransactionId() == transactionId) {
            transaction.display();
            return;
        }
    }

    cout << "\nTransaction not found.\n";
}

int TransactionManager::getTransactionCount() const {
    return transactions.size();
}

int TransactionManager::getSuccessfulCount() const {
    int count = 0;

    for (const Transaction& transaction : transactions) {
        if (transaction.getStatus() == TransactionStatus::SUCCESS) {
            count++;
        }
    }

    return count;
}

int TransactionManager::getFailedCount() const {
    int count = 0;

    for (const Transaction& transaction : transactions) {
        if (transaction.getStatus() == TransactionStatus::FAILED) {
            count++;
        }
    }

    return count;
}

int TransactionManager::getRefundedCount() const {
    int count = 0;

    for (const Transaction& transaction : transactions) {
        if (transaction.getStatus() == TransactionStatus::REFUNDED) {
            count++;
        }
    }

    return count;
}
const vector<Transaction>& TransactionManager::getAllTransactions() const {
    return transactions;
}