#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

#include "Wallet.h"

#include <string>
#include <unordered_map>

class WalletManager
{
private:
    std::unordered_map<std::string, Wallet> wallets;

    int nextWalletNumber;

    std::string generateWalletId();

public:
    WalletManager();

    // Wallet creation
    bool createWallet(const std::string& ownerId);

    // Wallet lookup
    Wallet* getWalletByOwnerId(const std::string& ownerId);
    Wallet* getWalletByWalletId(const std::string& walletId);

    // Display
    void displayAllWallets() const;

    // Information
    int getWalletCount() const;

    // File handling
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
};

#endif