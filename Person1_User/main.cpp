
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <cctype>
#include <limits>

using namespace std;

// =====================================================
// DIGITAL WALLET AND PAYMENT SIMULATION SYSTEM
// Member 1: User Registration and Authentication
// Single-file C++ CLI application
// =====================================================

enum class AccountStatus {
    ACTIVE,
    BLOCKED
};

// -------------------- USER BASE CLASS --------------------

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
    User(const string& id, const string& userName,
         const string& userMobile, const string& userEmail,
         const string& userPin, const string& userRole)
        : userId(id), name(userName), mobile(userMobile),
          email(userEmail), pin(userPin), role(userRole),
          failedAttempts(0), status(AccountStatus::ACTIVE) {}

    virtual ~User() = default;

    string getUserId() const { return userId; }
    string getName() const { return name; }
    string getMobile() const { return mobile; }
    string getEmail() const { return email; }
    string getRole() const { return role; }
    int getFailedAttempts() const { return failedAttempts; }

    AccountStatus getStatus() const { return status; }

    bool isBlocked() const {
        return status == AccountStatus::BLOCKED;
    }

    bool verifyPin(const string& enteredPin) {
        if (isBlocked()) {
            return false;
        }

        if (enteredPin == pin) {
            failedAttempts = 0;
            return true;
        }

        failedAttempts++;

        if (failedAttempts >= 3) {
            status = AccountStatus::BLOCKED;
        }

        return false;
    }

    bool changePin(const string& oldPin,
                   const string& newPin) {
        if (isBlocked() || oldPin != pin ||
            !isValidPin(newPin) || oldPin == newPin) {
            return false;
        }

        pin = newPin;
        failedAttempts = 0;
        return true;
    }

    void blockAccount() {
        status = AccountStatus::BLOCKED;
    }

    void unblockAccount() {
        status = AccountStatus::ACTIVE;
        failedAttempts = 0;
    }

    // Data format used by the file manager.
    string serialize() const {
        return userId + "|" + name + "|" + mobile + "|" +
               email + "|" + pin + "|" + role + "|" +
               to_string(failedAttempts) + "|" +
               (isBlocked() ? "BLOCKED" : "ACTIVE");
    }

    static bool isValidPin(const string& value) {
        if (value.length() != 4) {
            return false;
        }

        for (unsigned char ch : value) {
            if (!isdigit(ch)) {
                return false;
            }
        }

        return true;
    }

    void restoreState(int attempts, const string& state) {
        failedAttempts = attempts;
        status = (state == "BLOCKED")
                     ? AccountStatus::BLOCKED
                     : AccountStatus::ACTIVE;
    }

    virtual string accountDescription() const = 0;
};

// -------------------- CUSTOMER CLASS --------------------

class Customer : public User {
public:
    Customer(const string& id, const string& name,
             const string& mobile, const string& email,
             const string& pin)
        : User(id, name, mobile, email, pin, "Customer") {}

    string accountDescription() const override {
        return "Customer Account";
    }
};

// -------------------- MERCHANT CLASS --------------------

class Merchant : public User {
public:
    Merchant(const string& id, const string& name,
             const string& mobile, const string& email,
             const string& pin)
        : User(id, name, mobile, email, pin, "Merchant") {}

    string accountDescription() const override {
        return "Merchant Account";
    }
};

// -------------------- AUTHENTICATION SERVICE --------------------

class AuthService {
private:
    vector<unique_ptr<User>> users;
    int nextCustomerId = 1001;
    int nextMerchantId = 1001;
    const string filename = "users.txt";

    // Keep the file format safe by rejecting its separator.
    bool validField(const string& value) const {
        return !value.empty() &&
               value.find('|') == string::npos &&
               value.find('\n') == string::npos &&
               value.find('\r') == string::npos;
    }

    bool validMobile(const string& mobile) const {
        if (mobile.length() != 10) {
            return false;
        }

        return all_of(mobile.begin(), mobile.end(),
            [](unsigned char ch) { return isdigit(ch); });
    }

    bool validEmail(const string& email) const {
        size_t at = email.find('@');

        return at != string::npos &&
               at > 0 &&
               at < email.length() - 1 &&
               email.find('.', at) != string::npos &&
               email.find(' ') == string::npos;
    }

    bool detailsAlreadyExist(const string& mobile,
                             const string& email) const {
        for (const auto& user : users) {
            if (user->getMobile() == mobile ||
                user->getEmail() == email) {
                return true;
            }
        }

        return false;
    }

    User* findById(const string& id) const {
        for (const auto& user : users) {
            if (user->getUserId() == id) {
                return user.get();
            }
        }

        return nullptr;
    }

public:
    AuthService() {
        loadUsers();
    }

    string registerUser(const string& name,
                        const string& mobile,
                        const string& email,
                        const string& pin,
                        const string& role) {
        if (!validField(name) ||
            !validField(mobile) ||
            !validField(email)) {
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
            do {
                id = "U" + to_string(nextCustomerId++);
            } while (findById(id) != nullptr);

            users.push_back(make_unique<Customer>(
                id, name, mobile, email, pin));
        }
        else if (role == "Merchant") {
            do {
                id = "M" + to_string(nextMerchantId++);
            } while (findById(id) != nullptr);

            users.push_back(make_unique<Merchant>(
                id, name, mobile, email, pin));
        }
        else {
            cout << "Error: Invalid account type.\n";
            return "";
        }

        saveUsers();
        return id;
    }

    User* login(const string& id, const string& enteredPin) {
        User* user = findById(id);

        if (user == nullptr) {
            cout << "Error: Account ID not found.\n";
            return nullptr;
        }

        if (user->isBlocked()) {
            cout << "Account is BLOCKED after failed login attempts.\n";
            return nullptr;
        }

        if (!user->verifyPin(enteredPin)) {
            if (user->isBlocked()) {
                cout << "Three incorrect attempts. Account blocked.\n";
            } else {
                cout << "Incorrect PIN. Attempts used: "
                     << user->getFailedAttempts() << "/3\n";
            }

            saveUsers();
            return nullptr;
        }

        saveUsers();
        return user;
    }

    bool changePin(User* user, const string& oldPin,
                   const string& newPin) {
        if (user == nullptr) {
            return false;
        }

        if (!user->changePin(oldPin, newPin)) {
            return false;
        }

        saveUsers();
        return true;
    }

    void listAccounts() const {
        cout << "\n----------- REGISTERED ACCOUNTS -----------\n";

        if (users.empty()) {
            cout << "No accounts registered yet.\n";
            return;
        }

        for (const auto& user : users) {
            cout << "ID: " << user->getUserId()
                 << " | Name: " << user->getName()
                 << " | Role: " << user->getRole()
                 << " | Status: "
                 << (user->isBlocked() ? "BLOCKED" : "ACTIVE")
                 << '\n';
        }
    }

    // For testing and future admin integration.
    bool setAccountBlocked(const string& id, bool blocked) {
        User* user = findById(id);

        if (user == nullptr) {
            return false;
        }

        if (blocked) {
            user->blockAccount();
        } else {
            user->unblockAccount();
        }

        saveUsers();
        return true;
    }

    void saveUsers() const {
        ofstream file(filename);

        if (!file) {
            cerr << "Warning: Could not save users to file.\n";
            return;
        }

        for (const auto& user : users) {
            file << user->serialize() << '\n';
        }
    }

    void loadUsers() {
        ifstream file(filename);

        if (!file) {
            return; // First run: the file may not exist yet.
        }

        string line;

        while (getline(file, line)) {
            stringstream ss(line);
            vector<string> fields;
            string field;

            while (getline(ss, field, '|')) {
                fields.push_back(field);
            }

            if (fields.size() != 8) {
                continue;
            }

            const string& id = fields[0];
            const string& name = fields[1];
            const string& mobile = fields[2];
            const string& email = fields[3];
            const string& pin = fields[4];
            const string& role = fields[5];

            int attempts = 0;

            try {
                attempts = stoi(fields[6]);
            } catch (...) {
                continue;
            }

            if (attempts < 0) {
                attempts = 0;
            }

            if (!validField(name) ||
                !validMobile(mobile) ||
                !validEmail(email) ||
                !User::isValidPin(pin)) {
                continue;
            }

            if (findById(id) != nullptr ||
                detailsAlreadyExist(mobile, email)) {
                continue;
            }

            unique_ptr<User> user;

            if (role == "Customer" && id.rfind("U", 0) == 0) {
                user = make_unique<Customer>(
                    id, name, mobile, email, pin);

                try {
                    nextCustomerId = max(
                        nextCustomerId,
                        stoi(id.substr(1)) + 1);
                } catch (...) {
                    continue;
                }
            }
            else if (role == "Merchant" &&
                     id.rfind("M", 0) == 0) {
                user = make_unique<Merchant>(
                    id, name, mobile, email, pin);

                try {
                    nextMerchantId = max(
                        nextMerchantId,
                        stoi(id.substr(1)) + 1);
                } catch (...) {
                    continue;
                }
            }
            else {
                continue;
            }

            user->restoreState(attempts, fields[7]);
            users.push_back(move(user));
        }
    }
};

// -------------------- CLI HELPER FUNCTIONS --------------------

string readLine(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

void registerMenu(AuthService& auth, const string& role) {
    cout << "\n===== " << role << " REGISTRATION =====\n";

    string name = readLine("Enter full name: ");
    string mobile = readLine("Enter 10-digit mobile: ");
    string email = readLine("Enter email: ");
    string pin = readLine("Create a 4-digit PIN: ");

    string id = auth.registerUser(
        name, mobile, email, pin, role);

    if (!id.empty()) {
        cout << "\nRegistration successful!\n";
        cout << "Your " << role << " ID is: " << id << '\n';
        cout << "Please remember this ID for login.\n";
    }
}

void accountMenu(AuthService& auth, User* user) {
    if (user == nullptr) {
        return;
    }

    int choice = 0;

    do {
        cout << "\n===== " << user->getRole() << " MENU =====\n";
        cout << "Logged in as: " << user->getName()
             << " (" << user->getUserId() << ")\n";
        cout << "1. View Profile\n";
        cout << "2. Change PIN\n";
        cout << "3. Logout\n";
        cout << "Enter choice: ";

        string input;
        getline(cin, input);
        stringstream ss(input);

        if (!(ss >> choice) || !(ss >> ws).eof()) {
            cout << "Please enter a valid number.\n";
            choice = 0;
            continue;
        }

        switch (choice) {
            case 1:
                cout << "\n----- PROFILE -----\n";
                cout << "ID: " << user->getUserId() << '\n';
                cout << "Name: " << user->getName() << '\n';
                cout << "Mobile: " << user->getMobile() << '\n';
                cout << "Email: " << user->getEmail() << '\n';
                cout << "Role: " << user->getRole() << '\n';
                cout << "Status: "
                     << (user->isBlocked() ? "BLOCKED" : "ACTIVE")
                     << '\n';
                break;

            case 2: {
                string oldPin = readLine("Enter current PIN: ");
                string newPin = readLine("Enter new 4-digit PIN: ");

                if (auth.changePin(user, oldPin, newPin)) {
                    cout << "PIN changed successfully.\n";
                } else {
                    cout << "PIN change failed. Check your current PIN; "
                         << "the new PIN must be four digits and different.\n";
                }
                break;
            }

            case 3:
                cout << "Logged out successfully.\n";
                break;

            default:
                cout << "Invalid choice. Try again.\n";
        }

    } while (choice != 3);
}

// -------------------- MAIN FUNCTION --------------------

int main() {
    AuthService auth;
    int choice = 0;

    cout << "============================================\n";
    cout << "   DIGITAL WALLET AND PAYMENT SIMULATION\n";
    cout << "============================================\n";

    while (choice != 5) {
        cout << "\n=============== MAIN MENU ===============\n";
        cout << "1. Register Customer\n";
        cout << "2. Register Merchant\n";
        cout << "3. Login\n";
        cout << "4. View Accounts (demo/testing)\n";
        cout << "5. Exit\n";
        cout << "Enter choice: ";

        string input;
        getline(cin, input);
        stringstream ss(input);

        if (!(ss >> choice) || !(ss >> ws).eof()) {
            cout << "Please enter a valid number.\n";
            choice = 0;
            continue;
        }

        switch (choice) {
            case 1:
                registerMenu(auth, "Customer");
                break;

            case 2:
                registerMenu(auth, "Merchant");
                break;

            case 3: {
                cout << "\n=============== LOGIN ===============\n";
                string id = readLine("Enter account ID: ");
                string pin = readLine("Enter PIN: ");

                User* user = auth.login(id, pin);

                if (user != nullptr) {
                    cout << "\nLogin successful. Welcome, "
                         << user->getName() << "!\n";
                    accountMenu(auth, user);
                }
                break;
            }

            case 4:
                // Demonstration option; hide/remove in final submission
                // if account listing should be restricted to admins.
                auth.listAccounts();
                break;

            case 5:
                cout << "Saving data and exiting. Goodbye!\n";
                break;

            default:
                cout << "Invalid choice. Please choose 1-5.\n";
        }
    }

    return 0;
}
