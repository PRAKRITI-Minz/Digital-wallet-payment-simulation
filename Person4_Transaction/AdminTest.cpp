#include "Admin.h"
#include <iostream>

using namespace std;

int main() {

    TransactionManager transactionManager;

    // Sample transactions
    transactionManager.addTransaction(
        "U1001",
        "U1002",
        50000,
        TransactionType::USER_TRANSFER,
        TransactionStatus::SUCCESS,
        "08-10-2026 20:00"
    );

    transactionManager.addTransaction(
        "U1002",
        "M1001",
        25000,
        TransactionType::MERCHANT_PAYMENT,
        TransactionStatus::SUCCESS,
        "08-10-2026 20:05"
    );

    transactionManager.addTransaction(
        "U1001",
        "U1002",
        4500000,
        TransactionType::USER_TRANSFER,
        TransactionStatus::SUCCESS,
        "08-10-2026 20:10"
    );

    Admin admin("A1001", "System Admin");

    cout << "\n===== ADMIN REFUND TEST =====\n";

WalletManager walletManager;

walletManager.createWallet("M1001");
walletManager.createWallet("U1001");

Wallet* merchantWallet =
    walletManager.getWalletByOwnerId("M1001");

if (merchantWallet != nullptr)
{
    merchantWallet->addMoney(50000);
}

PaymentManager paymentManager(
    walletManager,
    transactionManager
);

admin.processRefund(
    paymentManager,
    "M1001",
    "U1001",
    20000
);

    // Admin dashboard
    admin.displayDashboard(transactionManager);

    // View all transactions
    admin.viewAllTransactions(transactionManager);

    // Search transaction
    cout << "\n===== SEARCH T1002 =====\n";
    admin.searchTransaction(transactionManager, "T1002");

    // User transaction history
    cout << "\n===== U1001 TRANSACTIONS =====\n";
    admin.viewUserTransactions(transactionManager, "U1001");

    // Statistics
    admin.displayStatistics(transactionManager);

    // Suspicious transactions
    admin.checkSuspiciousTransactions(transactionManager);

    return 0;
}