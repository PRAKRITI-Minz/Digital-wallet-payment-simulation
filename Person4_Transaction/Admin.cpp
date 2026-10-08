#include "Admin.h"
#include <iostream>

using namespace std;

Admin::Admin(string id, string name) {
    adminId = id;
    adminName = name;
}

string Admin::getAdminId() const {
    return adminId;
}

string Admin::getAdminName() const {
    return adminName;
}

void Admin::displayDashboard(
    const TransactionManager& transactionManager
) const {

    cout << "\n====================================\n";
    cout << "          ADMIN DASHBOARD\n";
    cout << "====================================\n";

    cout << "Admin ID   : " << adminId << endl;
    cout << "Admin Name : " << adminName << endl;

    cout << "\nTotal Transactions : "
         << transactionManager.getTransactionCount() << endl;

    cout << "Successful         : "
         << transactionManager.getSuccessfulCount() << endl;

    cout << "Failed             : "
         << transactionManager.getFailedCount() << endl;

    cout << "Refunded           : "
         << transactionManager.getRefundedCount() << endl;

    cout << "====================================\n";
}

void Admin::viewAllTransactions(
    const TransactionManager& transactionManager
) const {

    transactionManager.displayAllTransactions();
}

void Admin::searchTransaction(
    const TransactionManager& transactionManager,
    string transactionId
) const {

    transactionManager.searchTransaction(transactionId);
}

void Admin::viewUserTransactions(
    const TransactionManager& transactionManager,
    string userId
) const {

    transactionManager.displayUserTransactions(userId);
}

void Admin::displayStatistics(
    const TransactionManager& transactionManager
) const {

    cout << "\n========== TRANSACTION STATISTICS ==========\n";

    cout << "Total Transactions : "
         << transactionManager.getTransactionCount() << endl;

    cout << "Successful         : "
         << transactionManager.getSuccessfulCount() << endl;

    cout << "Failed             : "
         << transactionManager.getFailedCount() << endl;

    cout << "Refunded           : "
         << transactionManager.getRefundedCount() << endl;

    cout << "============================================\n";
}

void Admin::checkSuspiciousTransactions(
    const TransactionManager& transactionManager
) const {

    const long long SUSPICIOUS_AMOUNT = 4000000; // ₹40,000

    bool found = false;

    cout << "\n========== SUSPICIOUS TRANSACTIONS ==========\n";

    for (const Transaction& transaction :
         transactionManager.getAllTransactions()) {

        if (transaction.getAmountInPaise() >= SUSPICIOUS_AMOUNT) {

            transaction.display();
            found = true;
        }
    }

    if (!found) {
        cout << "No suspicious transactions found.\n";
    }

    cout << "=============================================\n";
}

void Admin::processRefund(
    PaymentManager& paymentManager,
    string merchantId,
    string customerId,
    long long amountInPaise
) const
{
    PaymentStatus status =
        paymentManager.refundPayment(
            merchantId,
            customerId,
            amountInPaise
        );

    cout << "\nRefund Result: "
         << paymentManager.getPaymentStatusMessage(status)
         << endl;
}