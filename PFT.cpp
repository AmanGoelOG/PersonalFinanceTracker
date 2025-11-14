#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
using namespace std;
const int MAX_TRANSACTIONS = 500;
const int MAX_ACCOUNTS = 15;

class User {
string username;
string password;
bool exists(const string &uname) {
ifstream fin("users.txt");
if (!fin) return false;
string fileUsername, filePassword;
while (fin >> fileUsername >> filePassword) {
if (fileUsername == uname) {
fin.close();
return true;
}
}
fin.close();
return false;
}
public:
string getUsername() const { return username; }
bool signup() {
cout << "\nSIGN UP\n";
cout << "Enter new username: ";
cin >> username;
if (exists(username)) { cout << "Username already exists!\n"; return false; }
cout << "Enter new password: ";
cin >> password;
ofstream fout("users.txt", ios::app);
if (!fout) throw runtime_error("Unable to open users.txt!");
fout << username << " " << password << "\n";
fout.close();
cout << "Account created\n";
return true;
}
bool login() {
cout << "\nLOGIN\n";
cout << "Username: ";
cin >> username;
cout << "Password: ";
cin >> password;
ifstream fin("users.txt");
if (!fin) { cout << "No users. Please sign up.\n"; return false; }
string fileUsername, filePassword;
while (fin >> fileUsername >> filePassword) {
if (fileUsername == username && filePassword == password) {
cout << "Login successful! Welcome " << username << "\n\n";
fin.close();
return true;
}
}
fin.close();
cout << "Invalid credentials.\n";
return false;
}
};

class Transaction {
string date;
string description;
string category;
string accountType;
double amount;
bool isIncome;
public:
Transaction() : amount(0), isIncome(false) {}
void input() {
cout << "Enter date (DD-MM-YYYY): ";
cin >> date;
cout << "Enter description: ";
cin.ignore();
getline(cin, description);
cout << "Enter category: ";
getline(cin, category);
cout << "Enter amount: ";
cin >> amount;
if (!cin || amount < 0) { cin.clear(); cin.ignore(1000, '\n'); throw invalid_argument("Amount must be non-negative numeric"); }
cout << "Enter account name: ";
cin.ignore();
getline(cin, accountType);
char type;
cout << "Income (i) or Expense (e)? ";
cin >> type;
if (type != 'i' && type != 'e') throw invalid_argument("Type must be 'i' or 'e'");
isIncome = (type == 'i');
}
void display() const {
cout << date << " | " << description << " | " << category << " | " << (isIncome ? "Income" : "Expense") << " | " << amount << " | " << accountType << "\n";
}
string toFileString() const {
string s = date + "," + description + "," + category + "," + to_string(amount) + "," + (isIncome ? "1" : "0") + "," + accountType;
return s;
}
void fromFileString(const string &line) {
string lineCopy = line;
string dataParts[6];
int partIndex = 0;
size_t pos = 0;
while ((pos = lineCopy.find(',')) != string::npos && partIndex < 5) {
dataParts[partIndex++] = lineCopy.substr(0, pos);
lineCopy.erase(0, pos + 1);
}
dataParts[partIndex] = lineCopy;
if (partIndex < 5) throw runtime_error("Corrupt transaction line");
date = dataParts[0];
description = dataParts[1];
category = dataParts[2];
try { amount = stod(dataParts[3]); } catch (...) { amount = 0.0; }
isIncome = (dataParts[4] == "1");
accountType = dataParts[5];
}
bool getIsIncome() const { return isIncome; }
double getAmount() const { return amount; }
string getAccountType() const { return accountType; }
};

void saveTransactions(Transaction transactions[], int transactionCount, const string &username) {
string fname = "transactions_" + username + ".txt";
ofstream fout(fname);
if (!fout) throw runtime_error("Failed to open " + fname + " for writing.");
for (int i = 0; i < transactionCount; ++i) fout << transactions[i].toFileString() << "\n";
fout.close();
}
int loadTransactions(Transaction transactions[], int maxTransactions, const string &username) {
string fname = "transactions_" + username + ".txt";
ifstream fin(fname);
if (!fin) return 0;
string line;
int count = 0;
while (getline(fin, line) && count < maxTransactions) {
if (line.empty()) continue;
try { transactions[count].fromFileString(line); ++count; }
catch (exception &e) { cerr << "Warning: skipping corrupt line: " << e.what() << "\n"; }
}
fin.close();
return count;
}

class Account {
protected:
string accountName;
double balance;
public:
Account() : accountName(""), balance(0) {}
Account(const string &n, double b) : accountName(n), balance(b) {}
void createAccount() {
cout << "Enter account name: ";
cin.ignore();
getline(cin, accountName);
cout << "Enter initial balance: ";
cin >> balance;
if (!cin || balance < 0) { cin.clear(); cin.ignore(1000, '\n'); throw invalid_argument("Initial balance must be non-negative numeric"); }
}
void displayAccount() {
cout << "Account: " << accountName << " | Balance: " << balance << "\n";
}
void updateBalance(double amt, bool income) {
if (!income && balance - amt < 0) throw runtime_error("Insufficient funds in account " + accountName);
balance += (income ? amt : -amt);
}
string getName() const { return accountName; }
double getBalance() const { return balance; }
void setName(const string &n) { accountName = n; }
void setBalance(double b) { balance = b; }
};

template <typename T>
class LoanMath {
public:
static T pow_n(T base, int n) {
T result = 1;
for (int i = 0; i < n; ++i) result *= base;
return result;
}
static T calculateEMI(T principal, T annualRatePercent, int months) {
if (principal <= 0) throw invalid_argument("Principal must be > 0");
if (months <= 0) throw invalid_argument("Months must be > 0");
if (annualRatePercent < 0) throw invalid_argument("Interest rate cannot be negative");
T monthlyRate = annualRatePercent / (12.0 * 100.0);
if (monthlyRate == 0) return principal / months;
T r = monthlyRate;
T powr = pow_n((T)(1 + r), months);
T numerator = principal * r * powr;
T denominator = powr - 1;
if (denominator == 0) throw runtime_error("Invalid EMI calculation");
return numerator / denominator;
}
};

class Loan : public Account {
double emi;
double interestRate;
public:
Loan() : emi(0), interestRate(0) {}
void createLoan() {
cout << "Enter loan name: ";
cin.ignore();
getline(cin, accountName);
cout << "Enter loan amount (principal): ";
cin >> balance;
if (!cin || balance <= 0) { cin.clear(); cin.ignore(1000, '\n'); throw invalid_argument("Loan amount must be positive"); }
cout << "Enter tenure in months: ";
int months; cin >> months;
if (!cin || months <= 0) { cin.clear(); cin.ignore(1000, '\n'); throw invalid_argument("Months must be positive integer"); }
cout << "Enter annual interest rate (%): ";
cin >> interestRate;
if (!cin || interestRate < 0) { cin.clear(); cin.ignore(1000, '\n'); throw invalid_argument("Interest rate must be non-negative"); }
try {
emi = LoanMath<double>::calculateEMI(balance, interestRate, months);
cout << "Calculated EMI: " << emi << " for " << months << " months\n";
}
catch (exception &e) { throw runtime_error(string("Loan calculation failed: ") + e.what()); }
}
void displayAccount() {
cout << "Loan: " << accountName << " | Outstanding: " << balance << " | EMI: " << emi << " | Interest: " << interestRate << "%\n";
}
void payEMI() {
if (emi <= 0) throw runtime_error("No EMI set for this loan");
if (balance <= 0) { cout << "Loan already cleared.\n"; return; }
double pay = emi;
if (pay > balance) pay = balance;
balance -= pay;
if (balance < 0) balance = 0;
cout << "EMI of " << pay << " paid. Remaining balance: " << balance << "\n";
}
double getEMI() const { return emi; }
double getInterestRate() const { return interestRate; }
void setEMI(double e) { emi = e; }
void setInterestRate(double r) { interestRate = r; }
};

void saveAccounts(Account accounts[], int accountCount, Loan &loan, const string &username) {
string fname = "accounts_" + username + ".txt";
ofstream fout(fname);
if (!fout) throw runtime_error("Failed to open " + fname + " for writing.");
for (int i = 0; i < accountCount; ++i) {
fout << "Account," << accounts[i].getName() << "," << accounts[i].getBalance() << "\n";
}
fout << "Loan," << loan.getName() << "," << loan.getBalance() << "," << loan.getEMI() << "," << loan.getInterestRate() << "\n";
fout.close();
}

bool loadAccounts(Account accounts[], int &accountCount, Loan &loan, const string &username) {
string fname = "accounts_" + username + ".txt";
ifstream fin(fname);
if (!fin) return false;
string line;
bool accountsFound = false;
while (getline(fin, line)) {
if (line.empty()) continue;
accountsFound = true;
string lineCopy = line;
string parts[5];
int partIndex = 0;
size_t pos = 0;
while ((pos = lineCopy.find(',')) != string::npos && partIndex < 4) {
parts[partIndex++] = lineCopy.substr(0, pos);
lineCopy.erase(0, pos + 1);
}
parts[partIndex] = lineCopy;
try {
string type = parts[0];
if (type == "Account" && accountCount < MAX_ACCOUNTS) {
accounts[accountCount].setName(parts[1]);
accounts[accountCount].setBalance(stod(parts[2]));
accountCount++;
} else if (type == "Loan") {
loan.setName(parts[1]);
loan.setBalance(stod(parts[2]));
loan.setEMI(stod(parts[3]));
loan.setInterestRate(stod(parts[4]));
}
} catch (exception &e) {
cerr << "Warning: skipping corrupt account line: " << e.what() << "\n";
}
}
fin.close();
return accountsFound;
}

class Budget {
double monthlyLimit;
public:
Budget() : monthlyLimit(0) {}
void setLimit() {
cout << "Enter monthly limit: ";
cin >> monthlyLimit;
if (!cin || monthlyLimit < 0) { cin.clear(); cin.ignore(1000, '\n'); throw invalid_argument("Monthly limit must be non-negative numeric"); }
}
bool checkLimit(double totalExpense) {
if (monthlyLimit > 0 && totalExpense > monthlyLimit) {
cout << "Warning! You exceeded your monthly budget of " << monthlyLimit << "\n";
return false;
}
return true;
}
};

class TaxCalculator {
public:
static double calculateTax(double income) {
if (income <= 250000) return 0;
if (income <= 500000) return (income - 250000) * 0.05;
if (income <= 1000000) return 12500 + (income - 500000) * 0.2;
return 112500 + (income - 1000000) * 0.3;
}
};

class SavingGoal {
double goalAmount;
public:
SavingGoal() : goalAmount(0) {}
void setGoal() {
cout << "Enter saving goal amount: ";
cin >> goalAmount;
if (!cin || goalAmount < 0) { cin.clear(); cin.ignore(1000, '\n'); throw invalid_argument("Goal must be non-negative numeric"); }
}
void checkGoal(double currentSavings) {
if (goalAmount > 0) {
if (currentSavings >= goalAmount) cout << "Congratulations! You reached your saving goal of " << goalAmount << "\n";
else cout << "Keep saving! You need " << (goalAmount - currentSavings) << " more.\n";
}
}
};

template <typename T>
void printReport(T array[], int count) {
if (count <= 0) throw runtime_error("No data available to generate report");
cout << "\n--- Report ---\n";
for (int i = 0; i < count; ++i) array[i].display();
}

class FinanceManager {
Transaction transactions[MAX_TRANSACTIONS];
int transactionCount;
Account accounts[MAX_ACCOUNTS];
int accountCount;
Loan loan;
Budget budget;
SavingGoal goal;
double totalIncome;
double totalExpense;
string username;
public:
FinanceManager(const string &user) : transactionCount(0), accountCount(0), totalIncome(0), totalExpense(0), username(user) {
bool loaded = false;
try {
loaded = loadAccounts(accounts, accountCount, loan, username);
} catch (exception &e) {
cerr << "Load accounts error: " << e.what() << "\n";
}

if (!loaded) {
cout << "\n--- First-time setup: Set initial balances ---\n";
double initialBalance;

if (accountCount < MAX_ACCOUNTS) {
cout << "Enter initial balance for 'Cash': ";
cin >> initialBalance;
if (!cin || initialBalance < 0) { cin.clear(); cin.ignore(1000, '\n'); initialBalance = 0; }
accounts[accountCount++] = Account("Cash", initialBalance);
}
if (accountCount < MAX_ACCOUNTS) {
cout << "Enter initial balance for 'Bank': ";
cin >> initialBalance;
if (!cin || initialBalance < 0) { cin.clear(); cin.ignore(1000, '\n'); initialBalance = 0; }
accounts[accountCount++] = Account("Bank", initialBalance);
}
if (accountCount < MAX_ACCOUNTS) {
cout << "Enter initial balance for 'Savings': ";
cin >> initialBalance;
if (!cin || initialBalance < 0) { cin.clear(); cin.ignore(1000, '\n'); initialBalance = 0; }
accounts[accountCount++] = Account("Savings", initialBalance);
}
cout << "Default accounts created.\n";

cout << "\n--- First-time setup: Create your loan ---\n";
try { loan.createLoan(); }
catch (exception &e) { cerr << "Loan setup error: " << e.what() << "\n"; }
} else {
cout << "Accounts loaded successfully.\n";
}

try { transactionCount = loadTransactions(transactions, MAX_TRANSACTIONS, username); }
catch (exception &e) { cerr << "Load error: " << e.what() << "\n"; transactionCount = 0; }
for (int i = 0; i < transactionCount; ++i) {
if (transactions[i].getIsIncome()) totalIncome += transactions[i].getAmount();
else totalExpense += transactions[i].getAmount();
}
}
int findAccountIndex(const string &name) {
for (int i = 0; i < accountCount; ++i) if (accounts[i].getName() == name) return i;
return -1;
}
void addTransaction() {
try {
if (transactionCount >= MAX_TRANSACTIONS) throw overflow_error("Transaction limit reached");
Transaction newTransaction;
newTransaction.input();
int accountIndex = findAccountIndex(newTransaction.getAccountType());
if (accountIndex >= 0) {
try { accounts[accountIndex].updateBalance(newTransaction.getAmount(), newTransaction.getIsIncome()); }
catch (exception &e) { cout << "Balance update failed: " << e.what() << "\n"; }
}
else {
if (loan.getName() == newTransaction.getAccountType()) {
cout << "Note: Transaction recorded. Use 'Pay Loan EMI' to update loan balance.\n";
} else {
cout << "Account not found! Transaction recorded but balance not updated.\n";
}
}
transactions[transactionCount] = newTransaction;
++transactionCount;
if (newTransaction.getIsIncome()) totalIncome += newTransaction.getAmount(); else totalExpense += newTransaction.getAmount();
cout << "Transaction added successfully.\n";
}
catch (exception &e) { cout << "Failed to add transaction: " << e.what() << "\n"; }
}
void viewReport() {
try {
printReport(transactions, transactionCount);
cout << "\nTotal Income: " << totalIncome << "\nTotal Expense: " << totalExpense << "\nNet Balance: " << (totalIncome - totalExpense) << "\n";
}
catch (exception &e) { cout << "Report error: " << e.what() << "\n"; }
}
void viewAccounts() {
cout << "\n--- Accounts ---\n";
if (accountCount == 0) cout << "No accounts.\n";
for (int i = 0; i < accountCount; ++i) accounts[i].displayAccount();
loan.displayAccount();
}
void payLoanEMI() { 
loan.payEMI(); 
}
void setBudget() { 
budget.setLimit(); 
}
void checkBudget() { 
budget.checkLimit(totalExpense); 
}
void setSavingGoal() { 
goal.setGoal(); 
}
void checkGoal() { 
goal.checkGoal(totalIncome - totalExpense); 
}
void calculateTax() { 
double tax = TaxCalculator::calculateTax(totalIncome); 
cout << "Estimated Tax: " << tax << "\n"; 
}
void saveAndExit() {
try {
saveTransactions(transactions, transactionCount, username);
saveAccounts(accounts, accountCount, loan, username);
cout << "Data saved successfully. Goodbye.\n";
}
catch (exception &e) { cout << "Save error: " << e.what() << "\n"; }
}
};

int main() {
User currentUser;
int choice;
bool loggedIn = false;
cout << "=== Personal Finance Tracker ===\n";
while (!loggedIn) {
cout << "\n1. Sign Up\n2. Login\n3. Exit\nEnter choice: ";
if (!(cin >> choice)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input\n"; continue; }
if (choice == 1) {
try { currentUser.signup(); }
catch (exception &e) { cout << "Signup failed: " << e.what() << "\n"; }
}
else if (choice == 2) {
try { if (currentUser.login()) loggedIn = true; }
catch (exception &e) { cout << "Login failed: " << e.what() << "\n"; }
}
else if (choice == 3) { cout << "Exiting...\n"; return 0; }
else cout << "Invalid choice.\n";
}

FinanceManager financeManager(currentUser.getUsername());
int option;
do {
cout << "\n=== Menu ===\n1. Add Transaction\n2. View Report\n3. View Accounts\n4. Set Budget\n5. Check Budget\n6. Set Saving Goal\n7. Check Goal\n8. Pay Loan EMI\n9. Calculate Tax\n10. Save & Exit\nEnter choice: ";
if (!(cin >> option)) {
cin.clear();
cin.ignore(1000, '\n');
cout << "Invalid input\n";
continue;
}
switch (option) {
case 1: financeManager.addTransaction(); break;
case 2: financeManager.viewReport(); break;
case 3: financeManager.viewAccounts(); break;
case 4: financeManager.setBudget(); break;
case 5: financeManager.checkBudget(); break;
case 6: financeManager.setSavingGoal(); break;
case 7: financeManager.checkGoal(); break;
case 8: financeManager.payLoanEMI(); break;
case 9: financeManager.calculateTax(); break;
case 10: financeManager.saveAndExit(); break;
default: cout << "Invalid option.\n";
}
} while (option != 10);
return 0;
}