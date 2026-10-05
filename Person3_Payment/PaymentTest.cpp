#include <iostream>
#include "Payment.h"

int main()
{
    WalletManager walletManager;

    // Create test wallets
    walletManager.createWallet("U1001");
    walletManager.createWallet("U1002");
    walletManager.createWallet("M1001");

    // Add ₹1000 to U1001
    Wallet* customer1 =
        walletManager.getWalletByOwnerId("U1001");

    if (customer1 != nullptr)
    {
        customer1->addMoney(100000);
    }

    PaymentManager paymentManager(walletManager);

    std::cout << "\n========================================\n";
    std::cout << "       PAYMENT MODULE TESTING\n";
    std::cout << "========================================\n";

    // ----------------------------------------
    // TEST 1: USER TO USER TRANSFER
    // ----------------------------------------

    std::cout << "\nTest 1: U1001 -> U1002 (₹500)\n";

    PaymentStatus status =
        paymentManager.transferMoney(
            "U1001",
            "U1002",
            50000
        );

    std::cout << paymentManager.getPaymentStatusMessage(status)
              << '\n';


    // ----------------------------------------
    // TEST 2: MERCHANT PAYMENT
    // ----------------------------------------

    std::cout << "\nTest 2: U1001 -> M1001 (₹200)\n";

    status =
        paymentManager.payMerchant(
            "U1001",
            "M1001",
            20000
        );

    std::cout << paymentManager.getPaymentStatusMessage(status)
              << '\n';


    // ----------------------------------------
    // TEST 3: INSUFFICIENT BALANCE
    // ----------------------------------------

    std::cout << "\nTest 3: U1001 -> U1002 (₹10,000)\n";

    status =
        paymentManager.transferMoney(
            "U1001",
            "U1002",
            1000000
        );

    std::cout << paymentManager.getPaymentStatusMessage(status)
              << '\n';


    // ----------------------------------------
    // TEST 4: INVALID AMOUNT
    // ----------------------------------------

    std::cout << "\nTest 4: U1001 -> U1002 (₹0)\n";

    status =
        paymentManager.transferMoney(
            "U1001",
            "U1002",
            0
        );

    std::cout << paymentManager.getPaymentStatusMessage(status)
              << '\n';


    // ----------------------------------------
    // TEST 5: SAME ACCOUNT
    // ----------------------------------------

    std::cout << "\nTest 5: U1001 -> U1001 (₹100)\n";

    status =
        paymentManager.transferMoney(
            "U1001",
            "U1001",
            10000
        );

    std::cout << paymentManager.getPaymentStatusMessage(status)
              << '\n';


    // ----------------------------------------
    // DISPLAY FINAL WALLETS
    // ----------------------------------------

    std::cout << "\n========================================\n";
    std::cout << "             FINAL BALANCES\n";
    std::cout << "========================================\n";

    walletManager.displayAllWallets();

    return 0;
}