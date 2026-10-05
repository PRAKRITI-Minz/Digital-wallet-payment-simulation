
#include "Wallet.h"

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

// ========================================
// GET CURRENT DATE
// ========================================

std::string Wallet::getCurrentDate()
{
    std::time_t now = std::time(nullptr);

    std::tm* localTime = std::localtime(&now);

    if (localTime == nullptr)
    {
        return "";
    }

    std::ostringstream dateStream;

    dateStream << std::put_time(localTime, "%Y-%m-%d");

    return dateStream.str();
}


// ========================================
// AUTOMATIC DAILY RESET
// ========================================

void Wallet::checkAndResetDailyLimit() const
{
    std::string today = getCurrentDate();

    if (today.empty())
    {
        return;
    }

    // First time this wallet is checked.
    if (lastTransactionDate.empty())
    {
        lastTransactionDate = today;
        dailySpentInPaise = 0;
        return;
    }

    // New day detected.
    if (lastTransactionDate != today)
    {
        dailySpentInPaise = 0;
        lastTransactionDate = today;
    }
}


// ========================================
// CONSTRUCTORS
// ========================================

Wallet::Wallet()
    : walletId(""),
      ownerId(""),
      balanceInPaise(0),
      dailyLimitInPaise(1000000),
      dailySpentInPaise(0),
      status(WalletStatus::ACTIVE),
      lastTransactionDate(getCurrentDate())
{
}

Wallet::Wallet(const std::string& walletId,
               const std::string& ownerId,
               long long dailyLimitInPaise)
    : walletId(walletId),
      ownerId(ownerId),
      balanceInPaise(0),
      dailyLimitInPaise(dailyLimitInPaise),
      dailySpentInPaise(0),
      status(WalletStatus::ACTIVE),
      lastTransactionDate(getCurrentDate())
{
}


// ========================================
// GETTERS
// ========================================

std::string Wallet::getWalletId() const
{
    return walletId;
}

std::string Wallet::getOwnerId() const
{
    return ownerId;
}

long long Wallet::getBalanceInPaise() const
{
    return balanceInPaise;
}

long long Wallet::getDailyLimitInPaise() const
{
    return dailyLimitInPaise;
}

long long Wallet::getDailySpentInPaise() const
{
    checkAndResetDailyLimit();

    return dailySpentInPaise;
}

long long Wallet::getRemainingDailyLimitInPaise() const
{
    checkAndResetDailyLimit();

    return dailyLimitInPaise - dailySpentInPaise;
}

std::string Wallet::getLastTransactionDate() const
{
    return lastTransactionDate;
}

WalletStatus Wallet::getStatus() const
{
    return status;
}

bool Wallet::isActive() const
{
    return status == WalletStatus::ACTIVE;
}

bool Wallet::isBlocked() const
{
    return status == WalletStatus::BLOCKED;
}

bool Wallet::isFrozen() const
{
    return status == WalletStatus::FROZEN;
}


// ========================================
// MONEY OPERATIONS
// ========================================

bool Wallet::addMoney(long long amountInPaise)
{
    if (amountInPaise <= 0)
    {
        return false;
    }

    if (!isActive())
    {
        return false;
    }

    balanceInPaise += amountInPaise;

    return true;
}

bool Wallet::canSpend(long long amountInPaise) const
{
    // Check whether a new day has started.
    checkAndResetDailyLimit();

    if (!isActive())
    {
        return false;
    }

    if (amountInPaise <= 0)
    {
        return false;
    }

    if (amountInPaise > balanceInPaise)
    {
        return false;
    }

    if (dailySpentInPaise + amountInPaise > dailyLimitInPaise)
    {
        return false;
    }

    return true;
}

bool Wallet::deductMoney(long long amountInPaise)
{
    if (!canSpend(amountInPaise))
    {
        return false;
    }

    balanceInPaise -= amountInPaise;
    dailySpentInPaise += amountInPaise;

    lastTransactionDate = getCurrentDate();

    return true;
}


// ========================================
// WALLET STATUS
// ========================================

void Wallet::blockWallet()
{
    status = WalletStatus::BLOCKED;
}

void Wallet::unblockWallet()
{
    status = WalletStatus::ACTIVE;
}

void Wallet::freezeWallet()
{
    status = WalletStatus::FROZEN;
}

void Wallet::unfreezeWallet()
{
    status = WalletStatus::ACTIVE;
}


// ========================================
// DAILY LIMIT
// ========================================

void Wallet::resetDailySpent()
{
    dailySpentInPaise = 0;
    lastTransactionDate = getCurrentDate();
}

void Wallet::setLastTransactionDate(const std::string& date)
{
    lastTransactionDate = date;
}


// ========================================
// DISPLAY
// ========================================

void Wallet::displayWallet() const
{
    checkAndResetDailyLimit();

    std::cout << "\n========================================\n";
    std::cout << "              WALLET DETAILS\n";
    std::cout << "========================================\n";

    std::cout << "Wallet ID       : " << walletId << '\n';
    std::cout << "Owner ID        : " << ownerId << '\n';

    std::cout << "Balance         : ₹"
              << balanceInPaise / 100.0 << '\n';

    std::cout << "Daily Limit     : ₹"
              << dailyLimitInPaise / 100.0 << '\n';

    std::cout << "Daily Spent     : ₹"
              << dailySpentInPaise / 100.0 << '\n';

    std::cout << "Remaining Limit : ₹"
              << getRemainingDailyLimitInPaise() / 100.0 << '\n';

    std::cout << "Last Checked    : "
              << lastTransactionDate << '\n';

    std::cout << "Status          : ";

    if (status == WalletStatus::ACTIVE)
    {
        std::cout << "ACTIVE";
    }
    else if (status == WalletStatus::BLOCKED)
    {
        std::cout << "BLOCKED";
    }
    else
    {
        std::cout << "FROZEN";
    }

    std::cout << '\n';

    std::cout << "========================================\n";
}


// ========================================
// FILE HANDLING
// ========================================

std::string Wallet::serialize() const
{
    checkAndResetDailyLimit();

    std::string statusString;

    if (status == WalletStatus::ACTIVE)
    {
        statusString = "ACTIVE";
    }
    else if (status == WalletStatus::BLOCKED)
    {
        statusString = "BLOCKED";
    }
    else
    {
        statusString = "FROZEN";
    }

    return walletId + "|" +
           ownerId + "|" +
           std::to_string(balanceInPaise) + "|" +
           std::to_string(dailyLimitInPaise) + "|" +
           std::to_string(dailySpentInPaise) + "|" +
           statusString + "|" +
           lastTransactionDate;
}

bool Wallet::deserialize(const std::string& data)
{
    std::stringstream ss(data);

    std::string balance;
    std::string dailyLimit;
    std::string dailySpent;
    std::string statusString;

    if (!std::getline(ss, walletId, '|'))
        return false;

    if (!std::getline(ss, ownerId, '|'))
        return false;

    if (!std::getline(ss, balance, '|'))
        return false;

    if (!std::getline(ss, dailyLimit, '|'))
        return false;

    if (!std::getline(ss, dailySpent, '|'))
        return false;

    if (!std::getline(ss, statusString, '|'))
        return false;

    if (!std::getline(ss, lastTransactionDate))
        return false;

    try
    {
        balanceInPaise = std::stoll(balance);
        dailyLimitInPaise = std::stoll(dailyLimit);
        dailySpentInPaise = std::stoll(dailySpent);
    }
    catch (...)
    {
        return false;
    }

    if (statusString == "ACTIVE")
    {
        status = WalletStatus::ACTIVE;
    }
    else if (statusString == "BLOCKED")
    {
        status = WalletStatus::BLOCKED;
    }
    else if (statusString == "FROZEN")
    {
        status = WalletStatus::FROZEN;
    }
    else
    {
        return false;
    }

    // Compatibility with old wallet records.
    if (lastTransactionDate.empty())
    {
        lastTransactionDate = getCurrentDate();
        dailySpentInPaise = 0;
    }

    return true;
}