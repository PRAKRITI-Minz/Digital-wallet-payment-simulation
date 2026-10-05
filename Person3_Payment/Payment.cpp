#include "Payment.h"
#include <iostream>

PaymentManager::PaymentManager(WalletManager& manager)
    : walletManager(manager)
{
}

bool PaymentManager::validateAmount(long long amountInPaise) const
{
    if (amountInPaise <= 0)
        return false;

    if (amountInPaise > MAX_TRANSACTION_AMOUNT)
        return false;

    return true;
}

PaymentStatus PaymentManager::transferMoney(
    const std::string& senderId,
    const std::string& receiverId,
    long long amountInPaise)
{
    // 1. Validate amount
    if (amountInPaise <= 0)
        return PaymentStatus::INVALID_AMOUNT;

    // 2. Check maximum transaction amount
    if (amountInPaise > MAX_TRANSACTION_AMOUNT)
        return PaymentStatus::TRANSACTION_LIMIT_EXCEEDED;

    // 3. Sender and receiver cannot be the same
    if (senderId == receiverId)
        return PaymentStatus::SAME_ACCOUNT;

    // 4. Find sender wallet
    Wallet* senderWallet =
        walletManager.getWalletByOwnerId(senderId);

    if (senderWallet == nullptr)
        return PaymentStatus::SENDER_NOT_FOUND;

    // 5. Find receiver wallet
    Wallet* receiverWallet =
        walletManager.getWalletByOwnerId(receiverId);

    if (receiverWallet == nullptr)
        return PaymentStatus::RECEIVER_NOT_FOUND;

    // 6. Both wallets must be active
    if (!senderWallet->isActive() ||
        !receiverWallet->isActive())
    {
        return PaymentStatus::WALLET_NOT_ACTIVE;
    }

    // 7. Check balance first
    if (amountInPaise > senderWallet->getBalanceInPaise())
        return PaymentStatus::INSUFFICIENT_BALANCE;

    // 8. Check daily spending limit
    if (amountInPaise >
        senderWallet->getRemainingDailyLimitInPaise())
    {
        return PaymentStatus::DAILY_LIMIT_EXCEEDED;
    }

    // 9. Deduct from sender
    if (!senderWallet->deductMoney(amountInPaise))
        return PaymentStatus::FAILED;

    // 10. Credit receiver
    if (!receiverWallet->addMoney(amountInPaise))
    {
        // Rollback if receiver cannot be credited
        senderWallet->addMoney(amountInPaise);
        return PaymentStatus::FAILED;
    }

    return PaymentStatus::SUCCESS;
}

PaymentStatus PaymentManager::payMerchant(
    const std::string& customerId,
    const std::string& merchantId,
    long long amountInPaise)
{
    // 1. Validate amount
    if (amountInPaise <= 0)
        return PaymentStatus::INVALID_AMOUNT;

    // 2. Check maximum transaction amount
    if (amountInPaise > MAX_TRANSACTION_AMOUNT)
        return PaymentStatus::TRANSACTION_LIMIT_EXCEEDED;

    // 3. Find customer wallet
    Wallet* customerWallet =
        walletManager.getWalletByOwnerId(customerId);

    if (customerWallet == nullptr)
        return PaymentStatus::SENDER_NOT_FOUND;

    // 4. Find merchant wallet
    Wallet* merchantWallet =
        walletManager.getWalletByOwnerId(merchantId);

    if (merchantWallet == nullptr)
        return PaymentStatus::MERCHANT_NOT_FOUND;

    // 5. Both wallets must be active
    if (!customerWallet->isActive() ||
        !merchantWallet->isActive())
    {
        return PaymentStatus::WALLET_NOT_ACTIVE;
    }

    // 6. Check balance
    if (amountInPaise > customerWallet->getBalanceInPaise())
        return PaymentStatus::INSUFFICIENT_BALANCE;

    // 7. Check daily spending limit
    if (amountInPaise >
        customerWallet->getRemainingDailyLimitInPaise())
    {
        return PaymentStatus::DAILY_LIMIT_EXCEEDED;
    }

    // 8. Deduct from customer
    if (!customerWallet->deductMoney(amountInPaise))
        return PaymentStatus::FAILED;

    // 9. Credit merchant
    if (!merchantWallet->addMoney(amountInPaise))
    {
        // Rollback customer payment
        customerWallet->addMoney(amountInPaise);
        return PaymentStatus::FAILED;
    }

    return PaymentStatus::SUCCESS;
}
std::string PaymentManager::getPaymentStatusMessage(
    PaymentStatus status) const
{
    switch (status)
    {
        case PaymentStatus::SUCCESS:
            return "Payment successful.";

        case PaymentStatus::INVALID_AMOUNT:
            return "Payment failed: Invalid amount.";

        case PaymentStatus::TRANSACTION_LIMIT_EXCEEDED:
            return "Payment failed: Transaction limit exceeded.";

        case PaymentStatus::SAME_ACCOUNT:
            return "Payment failed: Sender and receiver cannot be the same.";

        case PaymentStatus::SENDER_NOT_FOUND:
            return "Payment failed: Sender wallet not found.";

        case PaymentStatus::RECEIVER_NOT_FOUND:
            return "Payment failed: Receiver wallet not found.";

        case PaymentStatus::MERCHANT_NOT_FOUND:
            return "Payment failed: Merchant wallet not found.";

        case PaymentStatus::WALLET_NOT_ACTIVE:
            return "Payment failed: Wallet is blocked or frozen.";

        case PaymentStatus::INSUFFICIENT_BALANCE:
            return "Payment failed: Insufficient balance.";

        case PaymentStatus::DAILY_LIMIT_EXCEEDED:
            return "Payment failed: Daily spending limit exceeded.";

        case PaymentStatus::FAILED:
            return "Payment failed: Unable to complete payment.";
    }

    return "Payment failed: Unknown error.";
}