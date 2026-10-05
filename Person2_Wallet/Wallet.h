#ifndef WALLET_H
#define WALLET_H

#include <string>

enum class WalletStatus
{
    ACTIVE,
    BLOCKED,
    FROZEN
};

class Wallet
{
private:
    std::string walletId;
    std::string ownerId;

    // Money is stored in paise.
    // Example: ₹500.50 = 50050 paise
    long long balanceInPaise;

    long long dailyLimitInPaise;

    // Mutable because the daily counter may need to reset
    // even when accessed through a const function.
    mutable long long dailySpentInPaise;

    WalletStatus status;

    // Date of the last daily-limit check.
    // Format: YYYY-MM-DD
    mutable std::string lastTransactionDate;

    // Returns today's date in YYYY-MM-DD format.
    static std::string getCurrentDate();

    // Automatically resets daily spending when the date changes.
    void checkAndResetDailyLimit() const;

public:
    // Constructors
    Wallet();

    Wallet(const std::string& walletId,
           const std::string& ownerId,
           long long dailyLimitInPaise = 1000000);

    // Getters
    std::string getWalletId() const;
    std::string getOwnerId() const;

    long long getBalanceInPaise() const;
    long long getDailyLimitInPaise() const;
    long long getDailySpentInPaise() const;
    long long getRemainingDailyLimitInPaise() const;

    std::string getLastTransactionDate() const;

    WalletStatus getStatus() const;

    bool isActive() const;
    bool isBlocked() const;
    bool isFrozen() const;

    // Wallet operations
    bool addMoney(long long amountInPaise);
    bool canSpend(long long amountInPaise) const;
    bool deductMoney(long long amountInPaise);

    // Wallet status
    void blockWallet();
    void unblockWallet();

    void freezeWallet();
    void unfreezeWallet();

    // Daily limit
    void resetDailySpent();
    void setLastTransactionDate(const std::string& date);

    // Display
    void displayWallet() const;

    // File handling
    std::string serialize() const;
    bool deserialize(const std::string& data);
};

#endif