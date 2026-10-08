#include "Transaction.h"
#include <iostream>

using namespace std;

Transaction::Transaction(
    string id,
    string sender,
    string receiver,
    long long amount,
    TransactionType transactionType,
    TransactionStatus transactionStatus,
    string date
) {
    transactionId = id;
    senderId = sender;
    receiverId = receiver;
    amountInPaise = amount;
    type = transactionType;
    status = transactionStatus;
    dateTime = date;
}

string Transaction::getTransactionId() const {
    return transactionId;
}

string Transaction::getSenderId() const {
    return senderId;
}

string Transaction::getReceiverId() const {
    return receiverId;
}

long long Transaction::getAmountInPaise() const {
    return amountInPaise;
}

TransactionType Transaction::getType() const {
    return type;
}

TransactionStatus Transaction::getStatus() const {
    return status;
}

string Transaction::getDateTime() const {
    return dateTime;
}

string Transaction::getTypeString() const {
    switch (type) {
        case TransactionType::ADD_MONEY:
            return "ADD_MONEY";

        case TransactionType::USER_TRANSFER:
            return "USER_TRANSFER";

        case TransactionType::MERCHANT_PAYMENT:
            return "MERCHANT_PAYMENT";

        case TransactionType::REFUND:
            return "REFUND";
    }

    return "UNKNOWN";
}

string Transaction::getStatusString() const {
    switch (status) {
        case TransactionStatus::SUCCESS:
            return "SUCCESS";

        case TransactionStatus::FAILED:
            return "FAILED";

        case TransactionStatus::REFUNDED:
            return "REFUNDED";
    }

    return "UNKNOWN";
}

void Transaction::display() const {
    cout << "\n-----------------------------\n";
    cout << "Transaction ID : " << transactionId << endl;
    cout << "Sender ID      : " << senderId << endl;
    cout << "Receiver ID    : " << receiverId << endl;
    cout << "Amount         : Rs. " << amountInPaise / 100.0 << endl;
    cout << "Type           : " << getTypeString() << endl;
    cout << "Status         : " << getStatusString() << endl;
    cout << "Date & Time    : " << dateTime << endl;
    cout << "-----------------------------\n";
}