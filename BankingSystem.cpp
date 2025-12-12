#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <map>
#include <sstream>

using namespace std;

// Utility class for date/time operations
class DateTime {
public:
    static string getCurrentDateTime() {
        time_t now = time(0);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buf);
    }
};

// Transaction class
class Transaction {
private:
    string transactionId;
    string type;
    double amount;
    string date;
    string description;

public:
    Transaction(string type, double amt, string desc) 
        : type(type), amount(amt), description(desc) {
        date = DateTime::getCurrentDateTime();
        transactionId = "TXN" + to_string(time(0));
    }

    void display() const {
        cout << setw(15) << transactionId 
             << setw(12) << type 
             << setw(12) << fixed << setprecision(2) << amount
             << setw(22) << date 
             << "  " << description << endl;
    }

    string serialize() const {
        return transactionId + "|" + type + "|" + to_string(amount) + "|" + date + "|" + description;
    }

    static Transaction deserialize(const string& data) {
        stringstream ss(data);
        string id, type, amtStr, date, desc;
        getline(ss, id, '|');
        getline(ss, type, '|');
        getline(ss, amtStr, '|');
        getline(ss, date, '|');
        getline(ss, desc, '|');
        Transaction t(type, stod(amtStr), desc);
        return t;
    }
};

// Loan class
class Loan {
private:
    string loanId;
    double principal;
    double interestRate;
    int termMonths;
    double monthlyPayment;
    double remainingBalance;
    string startDate;
    bool isActive;

public:
    Loan(double amt, double rate, int months) 
        : principal(amt), interestRate(rate), termMonths(months), 
          remainingBalance(amt), isActive(true) {
        startDate = DateTime::getCurrentDateTime();
        loanId = "LOAN" + to_string(time(0));
        calculateMonthlyPayment();
    }

    void calculateMonthlyPayment() {
        double monthlyRate = interestRate / 12 / 100;
        monthlyPayment = (principal * monthlyRate * pow(1 + monthlyRate, termMonths)) / 
                        (pow(1 + monthlyRate, termMonths) - 1);
    }

    bool makePayment(double amount) {
        if (!isActive) return false;
        if (amount < monthlyPayment) return false;
        
        remainingBalance -= amount;
        if (remainingBalance <= 0) {
            remainingBalance = 0;
            isActive = false;
        }
        return true;
    }

    void display() const {
        cout << "\n--- Loan Details ---" << endl;
        cout << "Loan ID: " << loanId << endl;
        cout << "Principal: $" << fixed << setprecision(2) << principal << endl;
        cout << "Interest Rate: " << interestRate << "%" << endl;
        cout << "Term: " << termMonths << " months" << endl;
        cout << "Monthly Payment: $" << monthlyPayment << endl;
        cout << "Remaining Balance: $" << remainingBalance << endl;
        cout << "Status: " << (isActive ? "Active" : "Paid Off") << endl;
    }

    double getRemainingBalance() const { return remainingBalance; }
    bool getIsActive() const { return isActive; }
    double getMonthlyPayment() const { return monthlyPayment; }
};

// Base Account class
class Account {
protected:
    string accountNumber;
    string accountHolderName;
    double balance;
    string accountType;
    vector<Transaction> transactions;
    vector<shared_ptr<Loan>> loans;
    string creationDate;
    bool isActive;

public:
    Account(string accNum, string name, string type) 
        : accountNumber(accNum), accountHolderName(name), 
          accountType(type), balance(0), isActive(true) {
        creationDate = DateTime::getCurrentDateTime();
    }

    virtual ~Account() {}

    virtual void deposit(double amt) {
        if (amt <= 0) {
            cout << "Invalid deposit amount!" << endl;
            return;
        }
        balance += amt;
        transactions.push_back(Transaction("DEPOSIT", amt, "Cash deposit"));
        cout << "Deposited $" << fixed << setprecision(2) << amt << " successfully!" << endl;
    }

    virtual bool withdraw(double amt) {
        if (amt <= 0) {
            cout << "Invalid withdrawal amount!" << endl;
            return false;
        }
        if (balance < amt) {
            cout << "Insufficient balance!" << endl;
            return false;
        }
        balance -= amt;
        transactions.push_back(Transaction("WITHDRAW", amt, "Cash withdrawal"));
        cout << "Withdrawn $" << fixed << setprecision(2) << amt << " successfully!" << endl;
        return true;
    }

    bool transfer(Account& toAccount, double amt) {
        if (amt <= 0) {
            cout << "Invalid transfer amount!" << endl;
            return false;
        }
        if (balance < amt) {
            cout << "Insufficient balance for transfer!" << endl;
            return false;
        }
        
        balance -= amt;
        toAccount.balance += amt;
        
        transactions.push_back(Transaction("TRANSFER_OUT", amt, 
            "Transfer to " + toAccount.accountNumber));
        toAccount.transactions.push_back(Transaction("TRANSFER_IN", amt, 
            "Transfer from " + accountNumber));
        
        cout << "Transferred $" << fixed << setprecision(2) << amt << " successfully!" << endl;
        return true;
    }

    void applyLoan(double amount, double interestRate, int termMonths) {
        auto loan = make_shared<Loan>(amount, interestRate, termMonths);
        loans.push_back(loan);
        balance += amount;
        transactions.push_back(Transaction("LOAN", amount, "Loan disbursement"));
        cout << "Loan of $" << fixed << setprecision(2) << amount << " approved and credited!" << endl;
    }

    void payLoan(int loanIndex, double amount) {
        if (loanIndex < 0 || loanIndex >= loans.size()) {
            cout << "Invalid loan index!" << endl;
            return;
        }
        
        auto& loan = loans[loanIndex];
        if (!loan->getIsActive()) {
            cout << "This loan is already paid off!" << endl;
            return;
        }
        
        if (balance < amount) {
            cout << "Insufficient balance to pay loan!" << endl;
            return;
        }
        
        if (loan->makePayment(amount)) {
            balance -= amount;
            transactions.push_back(Transaction("LOAN_PAYMENT", amount, "Loan payment"));
            cout << "Loan payment of $" << fixed << setprecision(2) << amount << " successful!" << endl;
        } else {
            cout << "Payment must be at least the monthly payment amount!" << endl;
        }
    }

    virtual void displayAccountInfo() const {
        cout << "\n========================================" << endl;
        cout << "Account Type: " << accountType << endl;
        cout << "Account Number: " << accountNumber << endl;
        cout << "Account Holder: " << accountHolderName << endl;
        cout << "Balance: $" << fixed << setprecision(2) << balance << endl;
        cout << "Created: " << creationDate << endl;
        cout << "Status: " << (isActive ? "Active" : "Inactive") << endl;
        cout << "========================================" << endl;
    }

    void displayTransactionHistory() const {
        cout << "\n--- Transaction History ---" << endl;
        if (transactions.empty()) {
            cout << "No transactions yet." << endl;
            return;
        }
        
        cout << setw(15) << "Transaction ID" 
             << setw(12) << "Type" 
             << setw(12) << "Amount"
             << setw(22) << "Date" 
             << "  Description" << endl;
        cout << string(80, '-') << endl;
        
        for (const auto& t : transactions) {
            t.display();
        }
    }

    void displayLoans() const {
        if (loans.empty()) {
            cout << "\nNo loans on this account." << endl;
            return;
        }
        
        cout << "\n--- Loans ---" << endl;
        for (size_t i = 0; i < loans.size(); ++i) {
            cout << "\nLoan #" << i + 1;
            loans[i]->display();
        }
    }

    string getAccountNumber() const { return accountNumber; }
    string getAccountHolder() const { return accountHolderName; }
    double getBalance() const { return balance; }
    bool getIsActive() const { return isActive; }
    void deactivate() { isActive = false; }
};

// Savings Account with interest
class SavingsAccount : public Account {
private:
    double interestRate;

public:
    SavingsAccount(string accNum, string name, double rate = 3.5) 
        : Account(accNum, name, "SAVINGS"), interestRate(rate) {}

    void applyInterest() {
        double interest = balance * (interestRate / 100);
        balance += interest;
        transactions.push_back(Transaction("INTEREST", interest, "Interest credit"));
        cout << "Interest of $" << fixed << setprecision(2) << interest << " applied!" << endl;
    }

    void displayAccountInfo() const override {
        Account::displayAccountInfo();
        cout << "Interest Rate: " << interestRate << "%" << endl;
    }
};

// Checking Account with overdraft
class CheckingAccount : public Account {
private:
    double overdraftLimit;

public:
    CheckingAccount(string accNum, string name, double overdraft = 500) 
        : Account(accNum, name, "CHECKING"), overdraftLimit(overdraft) {}

    bool withdraw(double amt) override {
        if (amt <= 0) {
            cout << "Invalid withdrawal amount!" << endl;
            return false;
        }
        if (balance + overdraftLimit < amt) {
            cout << "Exceeds overdraft limit!" << endl;
            return false;
        }
        balance -= amt;
        transactions.push_back(Transaction("WITHDRAW", amt, "Cash withdrawal"));
        cout << "Withdrawn $" << fixed << setprecision(2) << amt << " successfully!" << endl;
        return true;
    }

    void displayAccountInfo() const override {
        Account::displayAccountInfo();
        cout << "Overdraft Limit: $" << fixed << setprecision(2) << overdraftLimit << endl;
    }
};

// Bank class
class Bank {
private:
    string bankName;
    map<string, shared_ptr<Account>> accounts;
    int nextAccountNumber;

    string generateAccountNumber() {
        return "ACC" + to_string(++nextAccountNumber);
    }

public:
    Bank(string name) : bankName(name), nextAccountNumber(1000) {}

    void createSavingsAccount(string name, double initialDeposit = 0) {
        string accNum = generateAccountNumber();
        auto acc = make_shared<SavingsAccount>(accNum, name);
        if (initialDeposit > 0) {
            acc->deposit(initialDeposit);
        }
        accounts[accNum] = acc;
        cout << "\nSavings Account created successfully!" << endl;
        cout << "Account Number: " << accNum << endl;
    }

    void createCheckingAccount(string name, double initialDeposit = 0) {
        string accNum = generateAccountNumber();
        auto acc = make_shared<CheckingAccount>(accNum, name);
        if (initialDeposit > 0) {
            acc->deposit(initialDeposit);
        }
        accounts[accNum] = acc;
        cout << "\nChecking Account created successfully!" << endl;
        cout << "Account Number: " << accNum << endl;
    }

    shared_ptr<Account> findAccount(string accNum) {
        auto it = accounts.find(accNum);
        if (it != accounts.end()) {
            return it->second;
        }
        return nullptr;
    }

    void displayAllAccounts() const {
        if (accounts.empty()) {
            cout << "\nNo accounts in the system." << endl;
            return;
        }
        
        cout << "\n========== All Accounts ==========" << endl;
        cout << setw(15) << "Acc Number" 
             << setw(20) << "Holder Name" 
             << setw(15) << "Type"
             << setw(15) << "Balance" << endl;
        cout << string(65, '-') << endl;
        
        for (const auto& pair : accounts) {
            auto acc = pair.second;
            if (acc->getIsActive()) {
                cout << setw(15) << acc->getAccountNumber()
                     << setw(20) << acc->getAccountHolder()
                     << setw(15) << "Account"
                     << setw(15) << fixed << setprecision(2) << acc->getBalance() << endl;
            }
        }
    }

    void saveToFile(const string& filename) {
        ofstream file(filename);
        if (!file) {
            cout << "Error saving to file!" << endl;
            return;
        }
        file << accounts.size() << endl;
        cout << "Data saved successfully!" << endl;
        file.close();
    }

    string getBankName() const { return bankName; }
};

// Main menu system
class BankingSystem {
private:
    Bank bank;

    void displayMainMenu() {
        cout << "\n========================================" << endl;
        cout << "     " << bank.getBankName() << endl;
        cout << "========================================" << endl;
        cout << "1.  Create Savings Account" << endl;
        cout << "2.  Create Checking Account" << endl;
        cout << "3.  Deposit Money" << endl;
        cout << "4.  Withdraw Money" << endl;
        cout << "5.  Transfer Money" << endl;
        cout << "6.  Check Balance" << endl;
        cout << "7.  View Transaction History" << endl;
        cout << "8.  Apply for Loan" << endl;
        cout << "9.  Pay Loan" << endl;
        cout << "10. View Loans" << endl;
        cout << "11. Apply Interest (Savings)" << endl;
        cout << "12. View All Accounts" << endl;
        cout << "13. Exit" << endl;
        cout << "========================================" << endl;
        cout << "Enter your choice: ";
    }

public:
    BankingSystem(string name) : bank(name) {}

    void run() {
        int choice;
        
        while (true) {
            displayMainMenu();
            cin >> choice;
            
            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid input! Please enter a number." << endl;
                continue;
            }
            
            switch (choice) {
                case 1: {
                    string name;
                    double deposit;
                    cout << "Enter account holder name: ";
                    cin.ignore();
                    getline(cin, name);
                    cout << "Enter initial deposit: $";
                    cin >> deposit;
                    bank.createSavingsAccount(name, deposit);
                    break;
                }
                case 2: {
                    string name;
                    double deposit;
                    cout << "Enter account holder name: ";
                    cin.ignore();
                    getline(cin, name);
                    cout << "Enter initial deposit: $";
                    cin >> deposit;
                    bank.createCheckingAccount(name, deposit);
                    break;
                }
                case 3: {
                    string accNum;
                    double amount;
                    cout << "Enter account number: ";
                    cin >> accNum;
                    auto acc = bank.findAccount(accNum);
                    if (acc) {
                        cout << "Enter amount to deposit: $";
                        cin >> amount;
                        acc->deposit(amount);
                    } else {
                        cout << "Account not found!" << endl;
                    }
                    break;
                }
                case 4: {
                    string accNum;
                    double amount;
                    cout << "Enter account number: ";
                    cin >> accNum;
                    auto acc = bank.findAccount(accNum);
                    if (acc) {
                        cout << "Enter amount to withdraw: $";
                        cin >> amount;
                        acc->withdraw(amount);
                    } else {
                        cout << "Account not found!" << endl;
                    }
                    break;
                }
                case 5: {
                    string fromAcc, toAcc;
                    double amount;
                    cout << "Enter source account number: ";
                    cin >> fromAcc;
                    cout << "Enter destination account number: ";
                    cin >> toAcc;
                    auto from = bank.findAccount(fromAcc);
                    auto to = bank.findAccount(toAcc);
                    if (from && to) {
                        cout << "Enter amount to transfer: $";
                        cin >> amount;
                        from->transfer(*to, amount);
                    } else {
                        cout << "One or both accounts not found!" << endl;
                    }
                    break;
                }
                case 6: {
                    string accNum;
                    cout << "Enter account number: ";
                    cin >> accNum;
                    auto acc = bank.findAccount(accNum);
                    if (acc) {
                        acc->displayAccountInfo();
                    } else {
                        cout << "Account not found!" << endl;
                    }
                    break;
                }
                case 7: {
                    string accNum;
                    cout << "Enter account number: ";
                    cin >> accNum;
                    auto acc = bank.findAccount(accNum);
                    if (acc) {
                        acc->displayTransactionHistory();
                    } else {
                        cout << "Account not found!" << endl;
                    }
                    break;
                }
                case 8: {
                    string accNum;
                    double amount, rate;
                    int months;
                    cout << "Enter account number: ";
                    cin >> accNum;
                    auto acc = bank.findAccount(accNum);
                    if (acc) {
                        cout << "Enter loan amount: $";
                        cin >> amount;
                        cout << "Enter interest rate (%): ";
                        cin >> rate;
                        cout << "Enter term (months): ";
                        cin >> months;
                        acc->applyLoan(amount, rate, months);
                    } else {
                        cout << "Account not found!" << endl;
                    }
                    break;
                }
                case 9: {
                    string accNum;
                    int loanIndex;
                    double amount;
                    cout << "Enter account number: ";
                    cin >> accNum;
                    auto acc = bank.findAccount(accNum);
                    if (acc) {
                        acc->displayLoans();
                        cout << "Enter loan number to pay: ";
                        cin >> loanIndex;
                        cout << "Enter payment amount: $";
                        cin >> amount;
                        acc->payLoan(loanIndex - 1, amount);
                    } else {
                        cout << "Account not found!" << endl;
                    }
                    break;
                }
                case 10: {
                    string accNum;
                    cout << "Enter account number: ";
                    cin >> accNum;
                    auto acc = bank.findAccount(accNum);
                    if (acc) {
                        acc->displayLoans();
                    } else {
                        cout << "Account not found!" << endl;
                    }
                    break;
                }
                case 11: {
                    string accNum;
                    cout << "Enter savings account number: ";
                    cin >> accNum;
                    auto acc = bank.findAccount(accNum);
                    if (acc) {
                        auto savings = dynamic_pointer_cast<SavingsAccount>(acc);
                        if (savings) {
                            savings->applyInterest();
                        } else {
                            cout << "Not a savings account!" << endl;
                        }
                    } else {
                        cout << "Account not found!" << endl;
                    }
                    break;
                }
                case 12:
                    bank.displayAllAccounts();
                    break;
                case 13:
                    cout << "\nThank you for using " << bank.getBankName() << "!" << endl;
                    return;
                default:
                    cout << "Invalid choice! Please try again." << endl;
            }
        }
    }
};

int main() {
    BankingSystem system("Swagat's Bank");
    system.run();
    return 0;
}
