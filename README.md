# Digital Wallet and Payment Simulation System

A console-based Digital Wallet and Payment Simulation System developed in C++ using Object-Oriented Programming (OOP), file handling, and STL containers.

This project simulates customer and merchant accounts, wallet operations, payments, transaction records, and administrative controls. It is an academic demonstration and does not process real money.

## Features

### User and Account Management

* Register customer and merchant accounts.
* Generate unique customer and merchant IDs.
* Login using a four-digit PIN.
* Block accounts after repeated incorrect PIN attempts.
* View account profiles and change PINs.

### Wallet Management

* Automatically create a wallet for a registered account.
* Add simulated money to a wallet.
* View wallet balances and wallet status.
* Apply a daily spending limit of ₹10,000.

### Payments and Transactions

* Transfer money between user wallets.
* Pay registered merchants.
* Validate transaction amounts and balances.
* Enforce a maximum transaction amount of ₹50,000.
* Maintain transaction history with transaction IDs, amounts, types, statuses, and timestamps.
* Support administrator-processed merchant refunds.

### Admin Features

* View the admin dashboard and transaction statistics.
* View all transactions or search by transaction ID.
* View transactions associated with a user.
* Identify transactions of ₹40,000 or more for review.
* Block and unblock user accounts.
* Freeze and unfreeze wallets.
* Process refunds and view all wallets.

## Object-Oriented Programming Concepts

* **Encapsulation:** Account, wallet, and transaction data are managed through class methods.
* **Inheritance:** `Customer` and `Merchant` derive from the abstract `User` class.
* **Polymorphism:** Derived account classes override the virtual `accountDescription()` method.
* **Abstraction:** Service and manager classes provide operations without exposing all their internal implementation details.
* **Class relationships:** `PaymentManager` works with wallet and transaction managers.
* **STL:** Uses containers such as `vector` and `unordered_map`, along with `unique_ptr` for user objects.

## File Handling

The program saves application data in these files:

| File               | Purpose                               |
| ------------------ | ------------------------------------- |
| `users.txt`        | User accounts and account state       |
| `wallets.txt`      | Wallet balances, limits, and statuses |
| `transactions.txt` | Transaction history                   |

These files are generated in the program's current working directory. The data can persist between program runs when the files remain available.

## Requirements

* A C++ compiler supporting C++17, such as GCC.
* A terminal or command prompt.

## Compile and Run

Run these commands from the repository root.

### Windows (PowerShell)

Compile:

```powershell
g++ -std=c++17 Final_Project/main.cpp -o Final_Project/main.exe
```

Run:

```powershell
.\Final_Project\main.exe
```

### Linux

Compile:

```bash
g++ -std=c++17 Final_Project/main.cpp -o Final_Project/main
```

Run:

```bash
./Final_Project/main
```

## Demo Admin Login

The console simulation uses these hard-coded demonstration credentials:

* **Admin ID:** `A1001`
* **Admin PIN:** `9999`

These credentials are for demonstration purposes only and should not be used for a real payment system.

## Project Structure

```text
Digital-wallet-payment-simulation/
├── Final_Project/
│   └── main.cpp
├── Person1_User/
├── Person2_Wallet/
├── Person3_Payment/
├── Person4_Transaction/
├── .gitignore
└── README.md
```

`Final_Project/main.cpp` contains the integrated, single-file console application. The other folders preserve the team's modular development work.

## Limitations

This is an educational simulation, not a production payment platform. It does not connect to banks, UPI, payment gateways, or real financial accounts. Authentication and data storage are simplified for demonstrating C++ concepts and file handling.

Do not use real personal information or actual financial credentials when testing the application.
