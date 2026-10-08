#include "TransactionManager.h"
#include <iostream>

using namespace std;

int main() {

    TransactionManager manager;

    // Add some sample transactions
    manager.addTransaction(
        "U1001",
        "U1002",
        50000,
        TransactionType::USER_TRANSFER,
        TransactionStatus::SUCCESS,
        "08-10-2026 20:00"
    );

    manager.addTransaction(
        "U1002",
        "M1001",
        25000,
        TransactionType::MERCHANT_PAYMENT,
        TransactionStatus::SUCCESS,
        "08-10-2026 20:05"
    );

    manager.addTransaction(
        "U1001",
        "U1002",
        90000,
        TransactionType::USER_TRANSFER,
        TransactionStatus::FAILED,
        "08-10-2026 20:10"
    );

    manager.addTransaction(
        "M1001",
        "U1001",
        25000,
        TransactionType::REFUND,
        TransactionStatus::REFUNDED,
        "08-10-2026 20:15"
    );

    // Display all transactions
    manager.displayAllTransactions();

    // Search by user
    cout << "\n\n===== TRANSACTIONS OF U1001 =====\n";
    manager.displayUserTransactions("U1001");

    // Search by transaction ID
    cout << "\n\n===== SEARCH TRANSACTION T1002 =====\n";
    manager.searchTransaction("T1002");

    // Statistics
    cout << "\n\n===== STATISTICS =====\n";
    cout << "Total transactions : "
         << manager.getTransactionCount() << endl;

    cout << "Successful         : "
         << manager.getSuccessfulCount() << endl;

    cout << "Failed             : "
         << manager.getFailedCount() << endl;

    cout << "Refunded           : "
         << manager.getRefundedCount() << endl;

    return 0;
}