#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <vector>

using namespace std;

enum class TransactionType {
    ADD_MONEY,
    USER_TRANSFER,
    MERCHANT_PAYMENT,
    REFUND
};

enum class TransactionStatus {
    SUCCESS,
    FAILED,
    REFUNDED
};

class Transaction {
private:
    string transactionId;
    string senderId;
    string receiverId;
    long long amountInPaise;
    TransactionType type;
    TransactionStatus status;
    string dateTime;

public:
    Transaction(
        string id,
        string sender,
        string receiver,
        long long amount,
        TransactionType transactionType,
        TransactionStatus transactionStatus,
        string date
    );

    string getTransactionId() const;
    string getSenderId() const;
    string getReceiverId() const;
    long long getAmountInPaise() const;
    TransactionType getType() const;
    TransactionStatus getStatus() const;
    string getDateTime() const;

    string getTypeString() const;
    string getStatusString() const;

    void display() const;
};

#endif