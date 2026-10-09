#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <chrono>
#include <ctime>
#include <stdexcept>

using namespace std;

// =============================================================
// DIGITAL WALLET AND PAYMENT SIMULATION SYSTEM
// Final single-file C++ console application
// =============================================================
// OOP concepts demonstrated:
// 1. Encapsulation  - private/protected data with public methods
// 2. Inheritance    - Customer and Merchant derive from User
// 3. Polymorphism   - virtual accountDescription() overridden by children
// 4. Abstraction    - service/manager classes hide implementation details
// 5. Composition     - PaymentManager uses WalletManager and TransactionManager
// 6. STL             - vector, unordered_map, unique_ptr
// 7. File Handling   - users, wallets and transactions are persisted
// 8. Exception       - numeric/date parsing and input validation safeguards
// =============================================================

// -------------------- UTILITY FUNCTIONS --------------------

string currentDateTime() {
    auto now = chrono::system_clock::now();
    time_t timeNow = chrono::system_clock::to_time_t(now);
    tm* localTime = std::localtime(&timeNow);

if (localTime == nullptr)
{
    return "";
}

ostringstream out;
out << put_time(localTime, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

string readLine(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

bool parseInteger(const string& input, int& value)
{
    if (input.empty())
        return false;

    size_t pos = 0;

    try
    {
        long long number = stoll(input, &pos);

        if (pos != input.size())
            return false;

        if (number < numeric_limits<int>::min() ||
            number > numeric_limits<int>::max())
            return false;

        value = static_cast<int>(number);
        return true;
    }
    catch (...)
    {
        return false;
    }
}
long long parseAmountInPaise(const string& input) {
    // Accept amounts such as 500 or 500.50 without using floating point.
    if (input.empty() || input[0] == '-') {
        throw invalid_argument("Invalid amount");
    }

    size_t dot = input.find('.');
    if (dot != string::npos && input.find('.', dot + 1) != string::npos) {
        throw invalid_argument("Invalid amount");
    }

    string rupees = (dot == string::npos) ? input : input.substr(0, dot);
    string paise = (dot == string::npos) ? "" : input.substr(dot + 1);

    if (rupees.empty()) rupees = "0";
    if (paise.size() > 2) throw invalid_argument("Maximum two decimal places allowed");

    if (!all_of(rupees.begin(), rupees.end(), [](unsigned char c) { return isdigit(c); }))
        throw invalid_argument("Invalid amount");
    if (!paise.empty() && !all_of(paise.begin(), paise.end(), [](unsigned char c) { return isdigit(c); }))
        throw invalid_argument("Invalid amount");

    long long r = stoll(rupees);
    long long p = paise.empty() ? 0 : stoll(paise + (paise.size() == 1 ? "0" : ""));

    if (r > (numeric_limits<long long>::max() - p) / 100)
        throw invalid_argument("Amount too large");

    return r * 100 + p;
}

string formatMoney(long long paise) {
    ostringstream out;
    out << "Rs. " << (paise / 100);
    long long cents = llabs(paise % 100);
    out << '.' << setw(2) << setfill('0') << cents << setfill(' ');
    return out.str();
}

// =============================================================
// USER / AUTHENTICATION MODULE
// =============================================================

enum class AccountStatus { ACTIVE, BLOCKED };

class User {
protected:
    string userId;
    string name;
    string mobile;
    string email;
    string pin;
    string role;
    int failedAttempts;
    AccountStatus status;

public:
    User(const string& id, const string& userName, const string& userMobile,
         const string& userEmail, const string& userPin, const string& userRole)
        : userId(id), name(userName), mobile(userMobile), email(userEmail),
          pin(userPin), role(userRole), failedAttempts(0), status(AccountStatus::ACTIVE) {}

    virtual ~User() = default;

    string getUserId() const { return userId; }
    string getName() const { return name; }
    string getMobile() const { return mobile; }
    string getEmail() const { return email; }
    string getRole() const { return role; }
    int getFailedAttempts() const { return failedAttempts; }
    bool isBlocked() const { return status == AccountStatus::BLOCKED; }

    bool verifyPin(const string& enteredPin) {
        if (isBlocked()) return false;
        if (enteredPin == pin) {
            failedAttempts = 0;
            return true;
        }
        ++failedAttempts;
        if (failedAttempts >= 3) status = AccountStatus::BLOCKED;
        return false;
    }

    bool changePin(const string& oldPin, const string& newPin) {
        if (isBlocked() || oldPin != pin || !isValidPin(newPin) || oldPin == newPin)
            return false;
        pin = newPin;
        failedAttempts = 0;
        return true;
    }

    void blockAccount() { status = AccountStatus::BLOCKED; }
    void unblockAccount() { status = AccountStatus::ACTIVE; failedAttempts = 0; }

    string serialize() const {
        return userId + "|" + name + "|" + mobile + "|" + email + "|" + pin + "|" +
               role + "|" + to_string(failedAttempts) + "|" +
               (isBlocked() ? "BLOCKED" : "ACTIVE");
    }

    static bool isValidPin(const string& value) {
        return value.size() == 4 &&
               all_of(value.begin(), value.end(), [](unsigned char c) { return isdigit(c); });
    }

    void restoreState(int attempts, const string& state) {
        failedAttempts = max(0, attempts);
        status = (state == "BLOCKED") ? AccountStatus::BLOCKED : AccountStatus::ACTIVE;
    }

    // Pure virtual function demonstrates abstraction and runtime polymorphism.
    virtual string accountDescription() const = 0;
};

class Customer : public User {
public:
    Customer(const string& id, const string& name, const string& mobile,
             const string& email, const string& pin)
        : User(id, name, mobile, email, pin, "Customer") {}

    string accountDescription() const override { return "Customer Account"; }
};

class Merchant : public User {
public:
    Merchant(const string& id, const string& name, const string& mobile,
             const string& email, const string& pin)
        : User(id, name, mobile, email, pin, "Merchant") {}

    string accountDescription() const override { return "Merchant Account"; }
};

class AuthService {
private:
    vector<unique_ptr<User>> users;
    int nextCustomerId = 1001;
    int nextMerchantId = 1001;
    const string filename = "users.txt";

    bool validField(const string& value) const {
        return !value.empty() && value.find('|') == string::npos &&
               value.find('\n') == string::npos && value.find('\r') == string::npos;
    }

    bool validMobile(const string& mobile) const {
        return mobile.size() == 10 &&
               all_of(mobile.begin(), mobile.end(), [](unsigned char c) { return isdigit(c); });
    }

    bool validEmail(const string& email) const {
        size_t at = email.find('@');
        return at != string::npos && at > 0 && at < email.size() - 1 &&
               email.find('.', at) != string::npos && email.find(' ') == string::npos;
    }

    bool detailsAlreadyExist(const string& mobile, const string& email) const {
        for (const auto& user : users)
            if (user->getMobile() == mobile || user->getEmail() == email) return true;
        return false;
    }

public:
    AuthService() { loadUsers(); }

    User* findById(const string& id) const {
        for (const auto& user : users)
            if (user->getUserId() == id) return user.get();
        return nullptr;
    }

    string registerUser(const string& name, const string& mobile, const string& email,
                        const string& pin, const string& role) {
        if (!validField(name) || !validField(mobile) || !validField(email)) {
            cout << "Error: Fields cannot be empty or contain '|'.\n";
            return "";
        }
        if (!validMobile(mobile)) {
            cout << "Error: Mobile number must contain 10 digits.\n";
            return "";
        }
        if (!validEmail(email)) {
            cout << "Error: Enter a valid email address.\n";
            return "";
        }
        if (!User::isValidPin(pin)) {
            cout << "Error: PIN must be exactly 4 digits.\n";
            return "";
        }
        if (detailsAlreadyExist(mobile, email)) {
            cout << "Error: Mobile number or email already registered.\n";
            return "";
        }

        string id;
        if (role == "Customer") {
            do { id = "U" + to_string(nextCustomerId++); } while (findById(id));
            users.push_back(make_unique<Customer>(id, name, mobile, email, pin));
        } else if (role == "Merchant") {
            do { id = "M" + to_string(nextMerchantId++); } while (findById(id));
            users.push_back(make_unique<Merchant>(id, name, mobile, email, pin));
        } else {
            cout << "Error: Invalid account type.\n";
            return "";
        }
        saveUsers();
        return id;
    }

    User* login(const string& id, const string& enteredPin) {
        User* user = findById(id);
        if (!user) {
            cout << "Error: Account ID not found.\n";
            return nullptr;
        }
        if (user->isBlocked()) {
            cout << "Account is BLOCKED after failed login attempts.\n";
            return nullptr;
        }
        if (!user->verifyPin(enteredPin)) {
            if (user->isBlocked()) cout << "Three incorrect attempts. Account blocked.\n";
            else cout << "Incorrect PIN. Attempts used: " << user->getFailedAttempts() << "/3\n";
            saveUsers();
            return nullptr;
        }
        saveUsers();
        return user;
    }

    bool changePin(User* user, const string& oldPin, const string& newPin) {
        if (!user || !user->changePin(oldPin, newPin)) return false;
        saveUsers();
        return true;
    }

    bool setAccountBlocked(const string& id, bool blocked) {
        User* user = findById(id);
        if (!user) return false;
        if (blocked) user->blockAccount(); else user->unblockAccount();
        saveUsers();
        return true;
    }

    void listAccounts() const {
        cout << "\n----------- REGISTERED ACCOUNTS -----------\n";
        if (users.empty()) { cout << "No accounts registered yet.\n"; return; }
        for (const auto& user : users) {
            cout << "ID: " << user->getUserId()
                 << " | Name: " << user->getName()
                 << " | Role: " << user->getRole()
                 << " | Type: " << user->accountDescription()
                 << " | Status: " << (user->isBlocked() ? "BLOCKED" : "ACTIVE") << '\n';
        }
    }

    void saveUsers() const {
        ofstream file(filename);
        if (!file) { cerr << "Warning: Could not save users to file.\n"; return; }
        for (const auto& user : users) file << user->serialize() << '\n';
    }

    void loadUsers() {
        ifstream file(filename);
        if (!file) return;
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            vector<string> f;
            string field;
            while (getline(ss, field, '|')) f.push_back(field);
            if (f.size() != 8) continue;
            try {
                int attempts = stoi(f[6]);
                unique_ptr<User> user;
                if (f[5] == "Customer" && f[0].rfind("U", 0) == 0) {
                    user = make_unique<Customer>(f[0], f[1], f[2], f[3], f[4]);
                    nextCustomerId = max(nextCustomerId, stoi(f[0].substr(1)) + 1);
                } else if (f[5] == "Merchant" && f[0].rfind("M", 0) == 0) {
                    user = make_unique<Merchant>(f[0], f[1], f[2], f[3], f[4]);
                    nextMerchantId = max(nextMerchantId, stoi(f[0].substr(1)) + 1);
                }
                if (!user || !validField(f[1]) || !validMobile(f[2]) || !validEmail(f[3]) ||
                    !User::isValidPin(f[4]) || findById(f[0]) || detailsAlreadyExist(f[2], f[3])) continue;
                user->restoreState(attempts, f[7]);
                users.push_back(move(user));
            } catch (...) { continue; }
        }
    }
};

// =============================================================
// WALLET MODULE
// =============================================================

enum class WalletStatus { ACTIVE, BLOCKED, FROZEN };

class Wallet {
private:
    string walletId;
    string ownerId;
    long long balanceInPaise;
    long long dailyLimitInPaise;
    mutable long long dailySpentInPaise;
    WalletStatus status;
    mutable string lastTransactionDate;

    static string getCurrentDate() {
        auto now = chrono::system_clock::now();
        time_t t = chrono::system_clock::to_time_t(now);
        tm* lt = std::localtime(&t);

if (lt == nullptr)
{
    return "";
}

ostringstream out;
out << put_time(lt, "%Y-%m-%d");
        return out.str();
    }

    void checkAndResetDailyLimit() const {
        string today = getCurrentDate();
        if (lastTransactionDate != today) {
            dailySpentInPaise = 0;
            lastTransactionDate = today;
        }
    }

public:
    Wallet() : balanceInPaise(0), dailyLimitInPaise(1000000), dailySpentInPaise(0), status(WalletStatus::ACTIVE) {}

    Wallet(const string& id, const string& owner, long long limit = 1000000)
        : walletId(id), ownerId(owner), balanceInPaise(0), dailyLimitInPaise(limit),
          dailySpentInPaise(0), status(WalletStatus::ACTIVE), lastTransactionDate(getCurrentDate()) {}

    string getWalletId() const { return walletId; }
    string getOwnerId() const { return ownerId; }
    long long getBalanceInPaise() const { return balanceInPaise; }
    long long getDailyLimitInPaise() const { return dailyLimitInPaise; }
    long long getDailySpentInPaise() const { checkAndResetDailyLimit(); return dailySpentInPaise; }
    long long getRemainingDailyLimitInPaise() const {
        checkAndResetDailyLimit();
        return max(0LL, dailyLimitInPaise - dailySpentInPaise);
    }
    string getLastTransactionDate() const { return lastTransactionDate; }
    WalletStatus getStatus() const { return status; }
    bool isActive() const { return status == WalletStatus::ACTIVE; }
    bool isBlocked() const { return status == WalletStatus::BLOCKED; }
    bool isFrozen() const { return status == WalletStatus::FROZEN; }

    bool addMoney(long long amountInPaise) {
        if (amountInPaise <= 0 || !isActive()) return false;
        balanceInPaise += amountInPaise;
        lastTransactionDate = getCurrentDate();
        return true;
    }

    bool canSpend(long long amountInPaise) const {
        checkAndResetDailyLimit();
        return isActive() && amountInPaise > 0 && balanceInPaise >= amountInPaise &&
               amountInPaise <= getRemainingDailyLimitInPaise();
    }

    bool deductMoney(long long amountInPaise) {
        if (!canSpend(amountInPaise)) return false;
        balanceInPaise -= amountInPaise;
        dailySpentInPaise += amountInPaise;
        lastTransactionDate = getCurrentDate();
        return true;
    }

    void blockWallet() { status = WalletStatus::BLOCKED; }
    void unblockWallet() { status = WalletStatus::ACTIVE; }
    void freezeWallet() { status = WalletStatus::FROZEN; }
    void unfreezeWallet() { status = WalletStatus::ACTIVE; }
    void resetDailySpent() { dailySpentInPaise = 0; lastTransactionDate = getCurrentDate(); }
    void setLastTransactionDate(const string& date) { lastTransactionDate = date; }

    void displayWallet() const {
        cout << "\n----------------------------------------\n";
        cout << "WALLET DETAILS\n";
        cout << "----------------------------------------\n";
        cout << "Wallet ID      : " << walletId << '\n';
        cout << "Owner ID       : " << ownerId << '\n';
        cout << "Balance        : " << formatMoney(balanceInPaise) << '\n';
        cout << "Daily Limit    : " << formatMoney(dailyLimitInPaise) << '\n';
        cout << "Daily Spent    : " << formatMoney(getDailySpentInPaise()) << '\n';
        cout << "Remaining Limit: " << formatMoney(getRemainingDailyLimitInPaise()) << '\n';
        cout << "Last Checked   : " << lastTransactionDate << '\n';
        cout << "Status         : " << (isActive() ? "ACTIVE" : (isFrozen() ? "FROZEN" : "BLOCKED")) << '\n';
        cout << "----------------------------------------\n";
    }

    string serialize() const {
        string state = isActive() ? "ACTIVE" : (isFrozen() ? "FROZEN" : "BLOCKED");
        return walletId + "|" + ownerId + "|" + to_string(balanceInPaise) + "|" +
               to_string(dailyLimitInPaise) + "|" + to_string(getDailySpentInPaise()) + "|" +
               state + "|" + lastTransactionDate;
    }

    bool deserialize(const string& data) {
        stringstream ss(data);
        vector<string> f;
        string field;
        while (getline(ss, field, '|')) f.push_back(field);
        if (f.size() != 7) return false;
        try {
            walletId = f[0]; ownerId = f[1]; balanceInPaise = stoll(f[2]);
            dailyLimitInPaise = stoll(f[3]); dailySpentInPaise = stoll(f[4]);
            status = (f[5] == "FROZEN") ? WalletStatus::FROZEN :
                     (f[5] == "BLOCKED") ? WalletStatus::BLOCKED : WalletStatus::ACTIVE;
            lastTransactionDate = f[6];
            return true;
        } catch (...) { return false; }
    }
};

class WalletManager {
private:
    unordered_map<string, Wallet> wallets;
    int nextWalletNumber = 1001;

    string generateWalletId() { return "W" + to_string(nextWalletNumber++); }

public:
    bool createWallet(const string& ownerId) {
        if (getWalletByOwnerId(ownerId)) return false;
        string id = generateWalletId();
        wallets.emplace(id, Wallet(id, ownerId));
        return true;
    }

    Wallet* getWalletByOwnerId(const string& ownerId) {
        for (auto& pair : wallets) if (pair.second.getOwnerId() == ownerId) return &pair.second;
        return nullptr;
    }

    Wallet* getWalletByWalletId(const string& walletId) {
        auto it = wallets.find(walletId);
        return it == wallets.end() ? nullptr : &it->second;
    }

    void displayAllWallets() const {
        cout << "\n========== ALL WALLETS ==========\n";
        if (wallets.empty()) { cout << "No wallets found.\n"; return; }
        for (const auto& pair : wallets) pair.second.displayWallet();
    }

    int getWalletCount() const { return static_cast<int>(wallets.size()); }

    bool saveToFile(const string& filename) const {
        ofstream file(filename);
        if (!file) return false;
        for (const auto& pair : wallets) file << pair.second.serialize() << '\n';
        return true;
    }

    bool loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file) return false;
        string line;
        while (getline(file, line)) {
            Wallet wallet;
            if (wallet.deserialize(line)) {
                wallets[wallet.getWalletId()] = wallet;
                if (wallet.getWalletId().size() > 1) {
                    try { nextWalletNumber = max(nextWalletNumber, stoi(wallet.getWalletId().substr(1)) + 1); }
                    catch (...) {}
                }
            }
        }
        return true;
    }
};

// =============================================================
// TRANSACTION MODULE
// =============================================================

enum class TransactionType { ADD_MONEY, USER_TRANSFER, MERCHANT_PAYMENT, REFUND };
enum class TransactionStatus { SUCCESS, FAILED, REFUNDED };

class Transaction {
private:
    string transactionId;
    string senderId;
    string receiverId;
    long long amountInPaise;
    TransactionType type;
    TransactionStatus status;
    string dateTime;

public:
    Transaction(string id, string sender, string receiver, long long amount,
                TransactionType transactionType, TransactionStatus transactionStatus,
                string date)
        : transactionId(move(id)), senderId(move(sender)), receiverId(move(receiver)),
          amountInPaise(amount), type(transactionType), status(transactionStatus), dateTime(move(date)) {}

    string getTransactionId() const { return transactionId; }
    string getSenderId() const { return senderId; }
    string getReceiverId() const { return receiverId; }
    long long getAmountInPaise() const { return amountInPaise; }
    TransactionType getType() const { return type; }
    TransactionStatus getStatus() const { return status; }
    string getDateTime() const { return dateTime; }

    string getTypeString() const {
        switch (type) {
            case TransactionType::ADD_MONEY: return "ADD_MONEY";
            case TransactionType::USER_TRANSFER: return "USER_TRANSFER";
            case TransactionType::MERCHANT_PAYMENT: return "MERCHANT_PAYMENT";
            case TransactionType::REFUND: return "REFUND";
        }
        return "UNKNOWN";
    }

    string getStatusString() const {
        switch (status) {
            case TransactionStatus::SUCCESS: return "SUCCESS";
            case TransactionStatus::FAILED: return "FAILED";
            case TransactionStatus::REFUNDED: return "REFUNDED";
        }
        return "UNKNOWN";
    }

    void display() const {
        cout << "\n-----------------------------\n";
        cout << "Transaction ID : " << transactionId << '\n';
        cout << "Sender ID      : " << senderId << '\n';
        cout << "Receiver ID    : " << receiverId << '\n';
        cout << "Amount         : " << formatMoney(amountInPaise) << '\n';
        cout << "Type           : " << getTypeString() << '\n';
        cout << "Status         : " << getStatusString() << '\n';
        cout << "Date & Time    : " << dateTime << '\n';
        cout << "-----------------------------\n";
    }

    string serialize() const {
        return transactionId + "|" + senderId + "|" + receiverId + "|" +
               to_string(amountInPaise) + "|" + getTypeString() + "|" +
               getStatusString() + "|" + dateTime;
    }

    static bool deserialize(const string& data, Transaction& out) {
        stringstream ss(data);
        vector<string> f; string field;
        while (getline(ss, field, '|')) f.push_back(field);
        if (f.size() != 7) return false;
        try {
            TransactionType type;
            if (f[4] == "ADD_MONEY") type = TransactionType::ADD_MONEY;
            else if (f[4] == "USER_TRANSFER") type = TransactionType::USER_TRANSFER;
            else if (f[4] == "MERCHANT_PAYMENT") type = TransactionType::MERCHANT_PAYMENT;
            else if (f[4] == "REFUND") type = TransactionType::REFUND;
            else return false;

            TransactionStatus status;
            if (f[5] == "SUCCESS") status = TransactionStatus::SUCCESS;
            else if (f[5] == "FAILED") status = TransactionStatus::FAILED;
            else if (f[5] == "REFUNDED") status = TransactionStatus::REFUNDED;
            else return false;

            out = Transaction(f[0], f[1], f[2], stoll(f[3]), type, status, f[6]);
            return true;
        } catch (...) { return false; }
    }
};

class TransactionManager {
private:
    vector<Transaction> transactions;
    int nextTransactionNumber = 1001;
    string filename = "transactions.txt";

public:
    TransactionManager() { loadFromFile(filename); }

    string generateTransactionId() { return "T" + to_string(nextTransactionNumber++); }

    void addTransaction(const string& senderId, const string& receiverId,
                        long long amountInPaise, TransactionType type,
                        TransactionStatus status, const string& dateTime) {
        transactions.emplace_back(generateTransactionId(), senderId, receiverId,
                                   amountInPaise, type, status, dateTime);
        saveToFile(filename);
    }

    void displayAllTransactions() const {
        if (transactions.empty()) { cout << "\nNo transactions found.\n"; return; }
        cout << "\n========== ALL TRANSACTIONS ==========\n";
        for (const auto& transaction : transactions) transaction.display();
    }

    void displayUserTransactions(const string& userId) const {
        bool found = false;
        for (const auto& transaction : transactions) {
            if (transaction.getSenderId() == userId || transaction.getReceiverId() == userId) {
                transaction.display(); found = true;
            }
        }
        if (!found) cout << "\nNo transactions found for User ID: " << userId << '\n';
    }

    void searchTransaction(const string& transactionId) const {
        for (const auto& transaction : transactions) {
            if (transaction.getTransactionId() == transactionId) {
                transaction.display(); return;
            }
        }
        cout << "\nTransaction not found.\n";
    }

    int getTransactionCount() const { return static_cast<int>(transactions.size()); }
    int getSuccessfulCount() const {
        return static_cast<int>(count_if(transactions.begin(), transactions.end(), [](const Transaction& t) {
            return t.getStatus() == TransactionStatus::SUCCESS;
        }));
    }
    int getFailedCount() const {
        return static_cast<int>(count_if(transactions.begin(), transactions.end(), [](const Transaction& t) {
            return t.getStatus() == TransactionStatus::FAILED;
        }));
    }
    int getRefundedCount() const {
        return static_cast<int>(count_if(transactions.begin(), transactions.end(), [](const Transaction& t) {
            return t.getStatus() == TransactionStatus::REFUNDED;
        }));
    }
    const vector<Transaction>& getAllTransactions() const { return transactions; }

    bool saveToFile(const string& fileName) const {
        ofstream file(fileName);
        if (!file) return false;
        for (const auto& transaction : transactions) file << transaction.serialize() << '\n';
        return true;
    }

    bool loadFromFile(const string& fileName) {
        ifstream file(fileName);
        if (!file) return false;
        string line;
        while (getline(file, line)) {
            Transaction transaction("", "", "", 0, TransactionType::ADD_MONEY,
                                    TransactionStatus::FAILED, "");
            if (Transaction::deserialize(line, transaction)) {
                transactions.push_back(transaction);
                string id = transaction.getTransactionId();
                if (id.size() > 1) {
                    try { nextTransactionNumber = max(nextTransactionNumber, stoi(id.substr(1)) + 1); }
                    catch (...) {}
                }
            }
        }
        return true;
    }
};

// =============================================================
// PAYMENT MODULE
// =============================================================

enum class PaymentStatus {
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

enum class PaymentType { USER_TRANSFER, MERCHANT_PAYMENT };

class PaymentManager {
private:
    WalletManager& walletManager;
    TransactionManager& transactionManager;
    static const long long MAX_TRANSACTION_AMOUNT = 5000000; // Rs. 50,000

    bool validateAmount(long long amountInPaise) const {
        return amountInPaise > 0;
    }

    void record(const string& sender, const string& receiver, long long amount,
                TransactionType type, TransactionStatus status) {
        transactionManager.addTransaction(sender, receiver, amount, type, status, currentDateTime());
    }

public:
    PaymentManager(WalletManager& manager, TransactionManager& tm)
        : walletManager(manager), transactionManager(tm) {}

    PaymentStatus transferMoney(const string& senderId, const string& receiverId, long long amountInPaise) {
        if (!validateAmount(amountInPaise)) return PaymentStatus::INVALID_AMOUNT;
        if (amountInPaise > MAX_TRANSACTION_AMOUNT) return PaymentStatus::TRANSACTION_LIMIT_EXCEEDED;
        if (senderId == receiverId) return PaymentStatus::SAME_ACCOUNT;

        Wallet* sender = walletManager.getWalletByOwnerId(senderId);
        if (!sender) return PaymentStatus::SENDER_NOT_FOUND;
        Wallet* receiver = walletManager.getWalletByOwnerId(receiverId);
        if (!receiver) return PaymentStatus::RECEIVER_NOT_FOUND;
        if (!sender->isActive() || !receiver->isActive()) return PaymentStatus::WALLET_NOT_ACTIVE;
        if (amountInPaise > sender->getBalanceInPaise()) return PaymentStatus::INSUFFICIENT_BALANCE;
        if (amountInPaise > sender->getRemainingDailyLimitInPaise()) return PaymentStatus::DAILY_LIMIT_EXCEEDED;

        if (!sender->deductMoney(amountInPaise)) return PaymentStatus::FAILED;
        if (!receiver->addMoney(amountInPaise)) {
            // Roll back the sender if the receiver cannot be credited.
            sender->addMoney(amountInPaise);
            return PaymentStatus::FAILED;
        }

        walletManager.saveToFile("wallets.txt");
        record(senderId, receiverId, amountInPaise, TransactionType::USER_TRANSFER, TransactionStatus::SUCCESS);
        return PaymentStatus::SUCCESS;
    }

    PaymentStatus payMerchant(const string& customerId, const string& merchantId, long long amountInPaise) {
        if (!validateAmount(amountInPaise)) return PaymentStatus::INVALID_AMOUNT;
        if (amountInPaise > MAX_TRANSACTION_AMOUNT) return PaymentStatus::TRANSACTION_LIMIT_EXCEEDED;
        if (customerId == merchantId) return PaymentStatus::SAME_ACCOUNT;

        Wallet* customer = walletManager.getWalletByOwnerId(customerId);
        if (!customer) return PaymentStatus::SENDER_NOT_FOUND;
        Wallet* merchant = walletManager.getWalletByOwnerId(merchantId);
        if (!merchant) return PaymentStatus::MERCHANT_NOT_FOUND;
        if (!customer->isActive() || !merchant->isActive()) return PaymentStatus::WALLET_NOT_ACTIVE;
        if (amountInPaise > customer->getBalanceInPaise()) return PaymentStatus::INSUFFICIENT_BALANCE;
        if (amountInPaise > customer->getRemainingDailyLimitInPaise()) return PaymentStatus::DAILY_LIMIT_EXCEEDED;

        if (!customer->deductMoney(amountInPaise)) return PaymentStatus::FAILED;
        if (!merchant->addMoney(amountInPaise)) {
            customer->addMoney(amountInPaise);
            return PaymentStatus::FAILED;
        }

        walletManager.saveToFile("wallets.txt");
        record(customerId, merchantId, amountInPaise, TransactionType::MERCHANT_PAYMENT, TransactionStatus::SUCCESS);
        return PaymentStatus::SUCCESS;
    }

    PaymentStatus refundPayment(const string& merchantId, const string& customerId, long long amountInPaise) {
        if (!validateAmount(amountInPaise)) return PaymentStatus::INVALID_AMOUNT;
        if (amountInPaise > MAX_TRANSACTION_AMOUNT) return PaymentStatus::TRANSACTION_LIMIT_EXCEEDED;

        Wallet* merchant = walletManager.getWalletByOwnerId(merchantId);
        if (!merchant) return PaymentStatus::MERCHANT_NOT_FOUND;
        Wallet* customer = walletManager.getWalletByOwnerId(customerId);
        if (!customer) return PaymentStatus::RECEIVER_NOT_FOUND;
        if (!merchant->isActive() || !customer->isActive()) return PaymentStatus::WALLET_NOT_ACTIVE;
        if (amountInPaise > merchant->getBalanceInPaise()) return PaymentStatus::INSUFFICIENT_BALANCE;

        // A refund reverses a merchant payment: merchant -> customer.
        if (!merchant->deductMoney(amountInPaise)) return PaymentStatus::FAILED;
        if (!customer->addMoney(amountInPaise)) {
            merchant->addMoney(amountInPaise);
            return PaymentStatus::FAILED;
        }

        walletManager.saveToFile("wallets.txt");
        record(merchantId, customerId, amountInPaise, TransactionType::REFUND, TransactionStatus::REFUNDED);
        return PaymentStatus::SUCCESS;
    }

    string getPaymentStatusMessage(PaymentStatus status) const {
        switch (status) {
            case PaymentStatus::SUCCESS: return "Payment successful.";
            case PaymentStatus::INVALID_AMOUNT: return "Payment failed: Invalid amount.";
            case PaymentStatus::TRANSACTION_LIMIT_EXCEEDED: return "Payment failed: Transaction limit exceeded.";
            case PaymentStatus::SAME_ACCOUNT: return "Payment failed: Sender and receiver cannot be the same.";
            case PaymentStatus::SENDER_NOT_FOUND: return "Payment failed: Sender wallet not found.";
            case PaymentStatus::RECEIVER_NOT_FOUND: return "Payment failed: Receiver wallet not found.";
            case PaymentStatus::MERCHANT_NOT_FOUND: return "Payment failed: Merchant wallet not found.";
            case PaymentStatus::WALLET_NOT_ACTIVE: return "Payment failed: Wallet is blocked or frozen.";
            case PaymentStatus::INSUFFICIENT_BALANCE: return "Payment failed: Insufficient balance.";
            case PaymentStatus::DAILY_LIMIT_EXCEEDED: return "Payment failed: Daily spending limit exceeded.";
            case PaymentStatus::FAILED: return "Payment failed: Internal wallet operation failed.";
        }
        return "Unknown payment status.";
    }
};

// =============================================================
// ADMIN MODULE
// =============================================================

class Admin {
private:
    string adminId;
    string adminName;

public:
    Admin(const string& id, const string& name) : adminId(id), adminName(name) {}

    void displayDashboard(const TransactionManager& tm) const {
        cout << "\n====================================\n";
        cout << "          ADMIN DASHBOARD\n";
        cout << "====================================\n";
        cout << "Admin ID   : " << adminId << '\n';
        cout << "Admin Name : " << adminName << '\n';
        cout << "\nTotal Transactions : " << tm.getTransactionCount() << '\n';
        cout << "Successful         : " << tm.getSuccessfulCount() << '\n';
        cout << "Failed             : " << tm.getFailedCount() << '\n';
        cout << "Refunded           : " << tm.getRefundedCount() << '\n';
        cout << "====================================\n";
    }

    void viewAllTransactions(const TransactionManager& tm) const { tm.displayAllTransactions(); }
    void searchTransaction(const TransactionManager& tm, const string& id) const { tm.searchTransaction(id); }
    void viewUserTransactions(const TransactionManager& tm, const string& id) const { tm.displayUserTransactions(id); }

    void displayStatistics(const TransactionManager& tm) const {
        cout << "\n========== TRANSACTION STATISTICS ==========\n";
        cout << "Total Transactions : " << tm.getTransactionCount() << '\n';
        cout << "Successful         : " << tm.getSuccessfulCount() << '\n';
        cout << "Failed             : " << tm.getFailedCount() << '\n';
        cout << "Refunded           : " << tm.getRefundedCount() << '\n';
        cout << "============================================\n";
    }

    void checkSuspiciousTransactions(const TransactionManager& tm) const {
        const long long suspiciousAmount = 4000000; // Rs. 40,000
        bool found = false;
        cout << "\n========== SUSPICIOUS TRANSACTIONS ==========\n";
        for (const auto& transaction : tm.getAllTransactions()) {
            if (transaction.getAmountInPaise() >= suspiciousAmount) {
                transaction.display();
                found = true;
            }
        }
        if (!found) cout << "No suspicious transactions found.\n";
        cout << "=============================================\n";
    }

    void processRefund(PaymentManager& pm, const string& merchantId,
                       const string& customerId, long long amountInPaise) const {
        PaymentStatus status = pm.refundPayment(merchantId, customerId, amountInPaise);
        cout << "\nRefund Result: " << pm.getPaymentStatusMessage(status) << '\n';
    }
};

// =============================================================
// MENU HELPERS
// =============================================================

void ensureWallet(WalletManager& wm, const string& userId) {
    if (wm.getWalletByOwnerId(userId) == nullptr) wm.createWallet(userId);
}

void registerMenu(AuthService& auth, WalletManager& wallets, const string& role)
{
    cout << "\n===== " << role << " REGISTRATION =====\n";

    string name;
    string mobile;
    string email;
    string pin;

    // -------------------- NAME VALIDATION --------------------
    while (true)
    {
        name = readLine("Enter full name: ");

        if (name.empty())
        {
            cout << "Error: Name cannot be empty.\n";
            continue;
        }

        if (name.find('|') != string::npos)
        {
            cout << "Error: Name cannot contain '|'.\n";
            continue;
        }

        break;
    }

    // -------------------- MOBILE VALIDATION --------------------
    while (true)
    {
        mobile = readLine("Enter 10-digit mobile: ");

        if (mobile.empty())
        {
            cout << "Error: Mobile number cannot be empty.\n";
            continue;
        }

        if (mobile.size() != 10)
        {
            cout << "Error: Mobile number must contain exactly 10 digits.\n";
            continue;
        }

        bool allDigits = true;

        for (char ch : mobile)
        {
            if (!isdigit(static_cast<unsigned char>(ch)))
            {
                allDigits = false;
                break;
            }
        }

        if (!allDigits)
        {
            cout << "Error: Mobile number must contain digits only.\n";
            continue;
        }

        break;
    }

    // -------------------- EMAIL VALIDATION --------------------
    while (true)
    {
        email = readLine("Enter email: ");

        if (email.empty())
        {
            cout << "Error: Email cannot be empty.\n";
            continue;
        }

        size_t at = email.find('@');

        if (at == string::npos ||
            at == 0 ||
            at == email.size() - 1 ||
            email.find('.', at) == string::npos ||
            email.find(' ') != string::npos)
        {
            cout << "Error: Enter a valid email address.\n";
            continue;
        }

        break;
    }

    // -------------------- PIN VALIDATION --------------------
    while (true)
    {
        pin = readLine("Create a 4-digit PIN: ");

        if (pin.empty())
        {
            cout << "Error: PIN cannot be empty.\n";
            continue;
        }

        if (!User::isValidPin(pin))
        {
            cout << "Error: PIN must contain exactly 4 digits.\n";
            continue;
        }

        break;
    }

    // AuthService performs the final business-rule validation
    // and creates the appropriate Customer or Merchant object.
    string id = auth.registerUser(name, mobile, email, pin, role);

    if (!id.empty())
    {
        ensureWallet(wallets, id);
        wallets.saveToFile("wallets.txt");

        cout << "\nRegistration successful!\n";
        cout << "Your " << role << " ID is: " << id << '\n';
        cout << "A wallet has been created for this account.\n";
    }
}

void addMoneyMenu(User* user, WalletManager& wallets, TransactionManager& tm) {
    Wallet* wallet = wallets.getWalletByOwnerId(user->getUserId());
    if (!wallet) { cout << "Wallet not found.\n"; return; }
    try {
        long long amount = parseAmountInPaise(readLine("Enter amount to add: Rs. "));
        if (wallet->addMoney(amount)) {
            wallets.saveToFile("wallets.txt");
            tm.addTransaction("SYSTEM", user->getUserId(), amount,
                              TransactionType::ADD_MONEY, TransactionStatus::SUCCESS, currentDateTime());
            cout << "Money added successfully: " << formatMoney(amount) << '\n';
        } else cout << "Unable to add money. Wallet may be blocked/frozen or amount invalid.\n";
    } catch (const exception& e) { cout << "Error: " << e.what() << '\n'; }
}

void userMenu(User* user, AuthService& auth, WalletManager& wallets,
              PaymentManager& payments, TransactionManager& tm) {
    int choice = 0;
    do {
        Wallet* wallet = wallets.getWalletByOwnerId(user->getUserId());
        cout << "\n========== " << user->getRole() << " MENU ==========\n";
        cout << "Logged in as: " << user->getName() << " (" << user->getUserId() << ")\n";
        if (wallet) cout << "Wallet Balance: " << formatMoney(wallet->getBalanceInPaise()) << '\n';
        cout << "1. View Profile\n";
        cout << "2. View Wallet\n";
        cout << "3. Add Money\n";
        cout << "4. Send Money\n";
        cout << "5. Pay Merchant\n";
        cout << "6. Transaction History\n";
        cout << "7. Change PIN\n";
        cout << "8. Logout\n";

        string input = readLine("Enter choice: ");

if (!parseInteger(input, choice))
{
    choice = 0;
}

        switch (choice) {
            case 1:
                cout << "\n----- PROFILE -----\n";
                cout << "ID       : " << user->getUserId() << '\n';
                cout << "Name     : " << user->getName() << '\n';
                cout << "Mobile   : " << user->getMobile() << '\n';
                cout << "Email    : " << user->getEmail() << '\n';
                cout << "Role     : " << user->getRole() << '\n';
                cout << "Type     : " << user->accountDescription() << '\n';
                cout << "Status   : " << (user->isBlocked() ? "BLOCKED" : "ACTIVE") << '\n';
                break;

            case 2:
                if (wallet) wallet->displayWallet(); else cout << "Wallet not found.\n";
                break;

            case 3:
                addMoneyMenu(user, wallets, tm);
                break;

            case 4: {
                 string receiver;

    while (true)
    {
        receiver = readLine("Enter receiver User ID: ");

        if (receiver.empty())
        {
            cout << "Error: Receiver User ID cannot be empty.\n";
            continue;
        }

        break;
    }
                try {
                    long long amount = parseAmountInPaise(readLine("Enter amount: Rs. "));
                    PaymentStatus status = payments.transferMoney(user->getUserId(), receiver, amount);
                    cout << payments.getPaymentStatusMessage(status) << '\n';
                } catch (const exception& e) { cout << "Error: " << e.what() << '\n'; }
                break;
            }

            case 5: {
                string merchant;

    while (true)
    {
        merchant = readLine("Enter merchant ID: ");

        if (merchant.empty())
        {
            cout << "Error: Merchant ID cannot be empty.\n";
            continue;
        }

        break;
    }
                try {
                    long long amount = parseAmountInPaise(readLine("Enter amount: Rs. "));
                    PaymentStatus status = payments.payMerchant(user->getUserId(), merchant, amount);
                    cout << payments.getPaymentStatusMessage(status) << '\n';
                } catch (const exception& e) { cout << "Error: " << e.what() << '\n'; }
                break;
            }

            case 6:
                tm.displayUserTransactions(user->getUserId());
                break;

            case 7: {
                string oldPin = readLine("Enter current PIN: ");
                string newPin = readLine("Enter new 4-digit PIN: ");
                cout << (auth.changePin(user, oldPin, newPin) ?
                         "PIN changed successfully.\n" : "PIN change failed.\n");
                break;
            }

            case 8:
                cout << "Logged out successfully.\n";
                break;

            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 8);
}

void adminMenu(Admin& admin, AuthService& auth, WalletManager& wallets,
               PaymentManager& payments, TransactionManager& tm) {
    int choice = 0;
    do {
        cout << "\n========== ADMIN MENU ==========\n";
        cout << "1. Dashboard\n";
        cout << "2. View All Transactions\n";
        cout << "3. Search Transaction\n";
        cout << "4. View User Transactions\n";
        cout << "5. Statistics\n";
        cout << "6. Suspicious Transactions\n";
        cout << "7. Block/Unblock User Account\n";
        cout << "8. Freeze/Unfreeze Wallet\n";
        cout << "9. Process Refund\n";
        cout << "10. View All Wallets\n";
        cout << "11. Logout\n";

        string input = readLine("Enter choice: ");

if (!parseInteger(input, choice))
{
    choice = 0;
}

        switch (choice) {
            case 1: admin.displayDashboard(tm); break;
            case 2: admin.viewAllTransactions(tm); break;
            case 3: admin.searchTransaction(tm, readLine("Enter Transaction ID: ")); break;
            case 4: {
    string userId;

    while (true)
    {
        userId = readLine("Enter User ID: ");

        if (userId.empty())
        {
            cout << "Error: User ID cannot be empty.\n";
            continue;
        }

        break;
    }

    admin.viewUserTransactions(tm, userId);
    break;
}
            case 5: admin.displayStatistics(tm); break;
            case 6: admin.checkSuspiciousTransactions(tm); break;
            case 7: {
                string id = readLine("Enter User ID: ");
                string action = readLine("Enter B to block or U to unblock: ");
                bool blocked = !action.empty() && (action[0] == 'B' || action[0] == 'b');
                cout << (auth.setAccountBlocked(id, blocked) ? "Account status updated.\n" : "User not found.\n");
                break;
            }
            case 8: {
                string id = readLine("Enter User ID: ");
                Wallet* wallet = wallets.getWalletByOwnerId(id);
                if (!wallet) { cout << "Wallet not found.\n"; break; }
                string action = readLine("Enter F to freeze or U to unfreeze: ");
                if (!action.empty() && (action[0] == 'F' || action[0] == 'f')) wallet->freezeWallet();
                else if (!action.empty() && (action[0] == 'U' || action[0] == 'u')) wallet->unfreezeWallet();
                else { cout << "Invalid action.\n"; break; }
                wallets.saveToFile("wallets.txt");
                cout << "Wallet status updated.\n";
                break;
            }
            case 9: {
                string merchant = readLine("Enter merchant ID: ");
                string customer = readLine("Enter customer ID: ");
                try {
                    long long amount = parseAmountInPaise(readLine("Enter refund amount: Rs. "));
                    admin.processRefund(payments, merchant, customer, amount);
                } catch (const exception& e) { cout << "Error: " << e.what() << '\n'; }
                break;
            }
            case 10: wallets.displayAllWallets(); break;
            case 11: cout << "Admin logged out.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 11);
}

// =============================================================
// MAIN
// =============================================================

int main() {
    try {
        AuthService auth;
        WalletManager wallets;
        wallets.loadFromFile("wallets.txt");

        TransactionManager transactionManager;
        PaymentManager paymentManager(wallets, transactionManager);
        Admin admin("A1001", "System Administrator");

        cout << "============================================\n";
        cout << "   DIGITAL WALLET AND PAYMENT SIMULATION\n";
        cout << "============================================\n";

        int choice = 0;
        while (choice != 6) {
            cout << "\n=============== MAIN MENU ===============\n";
            cout << "1. Register Customer\n";
            cout << "2. Register Merchant\n";
            cout << "3. Login\n";
            cout << "4. View Accounts (demo)\n";
            cout << "5. Admin Login\n";
            cout << "6. Exit\n";

            string input = readLine("Enter choice: ");

if (!parseInteger(input, choice))
{
    choice = 0;
}

            switch (choice) {
                case 1: registerMenu(auth, wallets, "Customer"); break;
                case 2: registerMenu(auth, wallets, "Merchant"); break;
                case 3: {
    string id;
    string pin;

    // Validate User/Merchant ID
    while (true)
    {
        id = readLine("Enter User/Merchant ID: ");

        if (id.empty())
        {
            cout << "Error: User/Merchant ID cannot be empty.\n";
            continue;
        }

        break;
    }

    // Validate PIN
    while (true)
    {
        pin = readLine("Enter PIN: ");

        if (pin.empty())
        {
            cout << "Error: PIN cannot be empty.\n";
            continue;
        }

        if (!User::isValidPin(pin))
        {
            cout << "Error: PIN must contain exactly 4 digits.\n";
            continue;
        }

        break;
    }

    User* user = auth.login(id, pin);

    if (user)
    {
        ensureWallet(wallets, user->getUserId());
        wallets.saveToFile("wallets.txt");
        userMenu(user, auth, wallets, paymentManager, transactionManager);
    }

    break;
}
                case 4: auth.listAccounts(); break;
                case 5: {
    string id;
    string pin;

    while (true)
    {
        id = readLine("Enter Admin ID: ");

        if (id.empty())
        {
            cout << "Error: Admin ID cannot be empty.\n";
            continue;
        }

        break;
    }

    while (true)
    {
        pin = readLine("Enter Admin PIN: ");

        if (pin.empty())
        {
            cout << "Error: Admin PIN cannot be empty.\n";
            continue;
        }

        if (!User::isValidPin(pin))
        {
            cout << "Error: Admin PIN must contain exactly 4 digits.\n";
            continue;
        }

        break;
    }

    // Demonstration admin credentials for the console simulation.
    if (id == "A1001" && pin == "9999")
        adminMenu(admin, auth, wallets, paymentManager, transactionManager);
    else
        cout << "Invalid admin credentials.\n";

    break;
}
                case 6:
                    wallets.saveToFile("wallets.txt");
                    transactionManager.saveToFile("transactions.txt");
                    cout << "Data saved. Thank you for using the Digital Wallet System.\n";
                    break;
                default: cout << "Invalid choice.\n";
            }
        }
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
