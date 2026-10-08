#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>
#include "../Person2_Wallet/WalletManager.h"

#include "../Person4_Transaction/TransactionManager.h"

enum class PaymentType
{
    USER_TRANSFER,
    MERCHANT_PAYMENT
};

enum class PaymentStatus
{
    SUCCESS,
    INVALID_AMOUNT,
    TRANSACTION_LIMIT_EXCEEDED,
    SAME_ACCOUNT,
    SENDER_NOT_FOUND,
    RECEIVER_NOT_FOUND,
    MERCHANT_NOT_FOUND,
    WALLET_NOT_ACTIVE,
    INSUFFICIENT_BALANCE,
    DAILY_LIMIT_EXCEEDED,
    FAILED
};

class PaymentManager
{
private:
   
   private:
    WalletManager& walletManager;
    TransactionManager& transactionManager;

    // Maximum amount allowed in one transaction = ₹50,000
    static const long long MAX_TRANSACTION_AMOUNT = 5000000;

    bool validateAmount(long long amountInPaise) const;

public:
    
    PaymentManager(
    WalletManager& manager,
    TransactionManager& transactionManager  
);

    PaymentStatus transferMoney(const std::string& senderId,
                       const std::string& receiverId,
                       long long amountInPaise);

    PaymentStatus payMerchant(const std::string& customerId,
                     const std::string& merchantId,
                     long long amountInPaise);
    PaymentStatus refundPayment(
    const std::string& merchantId,
    const std::string& customerId,
    long long amountInPaise
);

    std::string getPaymentStatusMessage(PaymentStatus status) const;
    
};

#endif