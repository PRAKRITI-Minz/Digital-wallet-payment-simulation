#include "WalletManager.h"

#include <iostream>
#include <fstream>

// ========================================
// CONSTRUCTOR
// ========================================

WalletManager::WalletManager()
    : nextWalletNumber(1001)
{
}


// ========================================
// GENERATE WALLET ID
// ========================================

std::string WalletManager::generateWalletId()
{
    return "W" + std::to_string(nextWalletNumber++);
}


// ========================================
// CREATE WALLET
// ========================================

bool WalletManager::createWallet(const std::string& ownerId)
{
    // Owner must have only one wallet.
    if (getWalletByOwnerId(ownerId) != nullptr)
    {
        return false;
    }

    std::string walletId = generateWalletId();

    Wallet newWallet(walletId, ownerId);

    wallets.emplace(walletId, newWallet);

    return true;
}


// ========================================
// FIND WALLET BY OWNER ID
// ========================================

Wallet* WalletManager::getWalletByOwnerId(
    const std::string& ownerId)
{
    for (auto& pair : wallets)
    {
        if (pair.second.getOwnerId() == ownerId)
        {
            return &pair.second;
        }
    }

    return nullptr;
}


// ========================================
// FIND WALLET BY WALLET ID
// ========================================

Wallet* WalletManager::getWalletByWalletId(
    const std::string& walletId)
{
    auto it = wallets.find(walletId);

    if (it != wallets.end())
    {
        return &it->second;
    }

    return nullptr;
}


// ========================================
// DISPLAY ALL WALLETS
// ========================================

void WalletManager::displayAllWallets() const
{
    if (wallets.empty())
    {
        std::cout << "\nNo wallets found.\n";
        return;
    }

    std::cout << "\n========================================\n";
    std::cout << "             ALL WALLETS\n";
    std::cout << "========================================\n";

    for (const auto& pair : wallets)
    {
        pair.second.displayWallet();
    }
}


// ========================================
// GET WALLET COUNT
// ========================================

int WalletManager::getWalletCount() const
{
    return static_cast<int>(wallets.size());
}


// ========================================
// SAVE WALLETS
// ========================================

bool WalletManager::saveToFile(
    const std::string& filename) const
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    for (const auto& pair : wallets)
    {
        file << pair.second.serialize() << '\n';
    }

    return true;
}


// ========================================
// LOAD WALLETS
// ========================================

bool WalletManager::loadFromFile(
    const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    wallets.clear();

    std::string line;

    int highestWalletNumber = 1000;

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        Wallet wallet;

        if (!wallet.deserialize(line))
        {
            // Ignore invalid wallet record.
            continue;
        }

        std::string walletId = wallet.getWalletId();

        // Store wallet.
        wallets.emplace(walletId, wallet);

        // Find numeric part of wallet ID.
        if (walletId.size() > 1 &&
            walletId[0] == 'W')
        {
            try
            {
                int number = std::stoi(walletId.substr(1));

                if (number > highestWalletNumber)
                {
                    highestWalletNumber = number;
                }
            }
            catch (...)
            {
                // Ignore invalid wallet ID.
            }
        }
    }

    // Continue wallet numbering after
    // the highest existing wallet ID.
    nextWalletNumber = highestWalletNumber + 1;

    return true;
}