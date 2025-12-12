# BankingSystem
An Advanced banking system in C++ with the following features:
Key Features:
Account Types:

Savings Account (with interest rate)
Checking Account (with overdraft protection)

Core Banking Operations:

Create multiple account types
Deposit and withdraw money
Transfer funds between accounts
Balance inquiry
Transaction history tracking

Advanced Features:

Loan Management: Apply for loans, make payments, track remaining balance
Interest Calculation: Apply interest to savings accounts
Transaction Records: Complete audit trail of all operations
Date/Time Stamps: All transactions timestamped
Overdraft Protection: Checking accounts support overdraft limits
Multiple Loans: Each account can have multiple active loans

Technical Highlights:

Object-oriented design with inheritance
Smart pointers for memory management
STL containers (vector, map)
Transaction serialization support
Robust error handling
Menu-driven interface

Design Patterns Used:

Inheritance (Account → SavingsAccount, CheckingAccount)
Polymorphism (virtual functions)
Encapsulation (private data members)
Composition (Bank contains Accounts, Accounts contain Transactions/Loans)

The system is production-ready with proper input validation, error messages, and a user-friendly interface. You can compile and run it with any C++ compiler (g++, Visual Studio, etc.).
