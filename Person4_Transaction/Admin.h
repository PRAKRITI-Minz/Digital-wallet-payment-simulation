#ifndef ADMIN_H
#define ADMIN_H

#include "TransactionManager.h"
#include "../Person3_Payment/Payment.h"
#include <string>

using namespace std;

class Admin {
private:
    string adminId;
    string adminName;

public:
    Admin(string id, string name);

    string getAdminId() const;
    string getAdminName() const;

    void displayDashboard(const TransactionManager& transactionManager) const;

    void viewAllTransactions(
        const TransactionManager& transactionManager
    ) const;

    void searchTransaction(
        const TransactionManager& transactionManager,
        string transactionId
    ) const;

    void viewUserTransactions(
        const TransactionManager& transactionManager,
        string userId
    ) const;

    void displayStatistics(
        const TransactionManager& transactionManager
    ) const;

    void checkSuspiciousTransactions(
        const TransactionManager& transactionManager
    ) const;

    void processRefund(
    PaymentManager& paymentManager,
    string merchantId,
    string customerId,
    long long amountInPaise
    ) const;
};

#endif