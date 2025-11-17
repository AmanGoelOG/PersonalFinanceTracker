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
        if (exists(username)) {
            cout << "Username already exists!\n";
            return false;
        }
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
        if (!fin) {
            cout << "No users. Please sign up.\n";
            return false;
        }
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
    string date, description, category, accountType;
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
        if (!cin || amount < 0) {
            cin.clear();
            cin.ignore(1000, '\n');
            throw invalid_argument("Amount must be non-negative numeric");
        }
        cout << "Enter account name: ";
        cin.ignore();
        getline(cin, accountType);
        char type;
        cout << "Income (i) or Expense (e)? ";
        cin >> type;
        if (type != 'i' && type != 'e')
            throw invalid_argument("Type must be 'i' or 'e'");
        isIncome = (type == 'i');
    }
    void display() const {
        cout << date << " | " << description << " | " << category << " | "
             << (isIncome ? "Income" : "Expense") << " | "
             << amount << " | " << accountType << "\n";
    }
    string toFileString() const {
        return date + "," + description + "," + category + "," +
               to_string(amount) + "," + (isIncome ? "1" : "0") + "," + accountType;
    }
    void fromFileString(const string &line) {
        string copy = line;
        string data[6];
        int idx = 0;
        size_t pos = 0;
        while ((pos = copy.find(',')) != string::npos && idx < 5) {
            data[idx++] = copy.substr(0, pos);
            copy.erase(0, pos + 1);
        }
        data[idx] = copy;
        if (idx < 5) throw runtime_error("Corrupt transaction line");
        date = data[0];
        description = data[1];
        category = data[2];
        try { amount = stod(data[3]); } 
        catch (...) { amount = 0; }
        isIncome = (data[4] == "1");
        accountType = data[5];
    }
    bool getIsIncome() const { return isIncome; }
    double getAmount() const { return amount; }
    string getAccountType() const { return accountType; }
};
void saveTransactions(Transaction t[], int count, const string &user) {
    string fname = "transactions_" + user + ".txt";
    ofstream fout(fname);
    if (!fout) throw runtime_error("Failed to open " + fname);
    for (int i = 0; i < count; i++)
        fout << t[i].toFileString() << "\n";
    fout.close();
}
int loadTransactions(Transaction t[], int maxT, const string &user) {
    string fname = "transactions_" + user + ".txt";
    ifstream fin(fname);
    if (!fin) return 0;
    string line;
    int count = 0;
    while (getline(fin, line) && count < maxT) {
        if (line.empty()) continue;
        try {
            t[count].fromFileString(line);
            count++;
        } catch (...) { continue; }
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
        if (!cin || balance < 0) throw invalid_argument("Balance must be >= 0");
    }
    void displayAccount() {
        cout << "Account: " << accountName << " | Balance: " << balance << "\n";
    }
    void updateBalance(double amt, bool income) {
        if (!income && balance < amt)
            throw runtime_error("Insufficient funds");
        balance += (income ? amt : -amt);
    }
    string getName() const { return accountName; }
    double getBalance() const { return balance; }
    void setName(const string &n) { accountName = n; }
    void setBalance(double b) { balance = b; }
};
class LoanMath {
public:
    static double pow_n(double base, int n) {
        double result = 1;
        for (int i = 0; i < n; i++) result *= base;
        return result;
    }
    static double calculateEMI(double principal, double annualRate, int months) {
        if (principal <= 0) throw invalid_argument("Principal must be > 0");
        if (months <= 0) throw invalid_argument("Months must be > 0");
        if (annualRate < 0) throw invalid_argument("Rate cannot be negative");
        double r = annualRate / (12.0 * 100.0);
        if (r == 0) return principal / months;
        double powr = pow_n(1 + r, months);
        double numerator = principal * r * powr;
        double denominator = powr - 1;
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
        cout << "Enter loan amount: ";
        cin >> balance;
        cout << "Enter tenure (months): ";
        int months;
        cin >> months;
        cout << "Enter annual interest rate (%): ";
        cin >> interestRate;
        emi = LoanMath::calculateEMI(balance, interestRate, months);
        cout << "Calculated EMI: " << emi << "\n";
    }
    void displayAccount() {
        cout << "Loan: " << accountName 
             << " | Outstanding: " << balance
             << " | EMI: " << emi
             << " | Interest: " << interestRate << "%\n";
    }
    void payEMI() {
        if (emi <= 0) {
            cout << "No EMI set.\n";
            return;
        }
        if (balance <= 0) {
            cout << "Loan already cleared.\n";
            return;
        }
        double pay = min(emi, balance);
        balance -= pay;
        cout << "EMI Paid: " << pay << ". Remaining: " << balance << "\n";
    }

    double getEMI() const { return emi; }
    double getInterestRate() const { return interestRate; }
    void setEMI(double e) { emi = e; }
    void setInterestRate(double r) { interestRate = r; }
};
void saveAccounts(Account accounts[], int count, Loan &loan, const string &user) {
    string fname = "accounts_" + user + ".txt";
    ofstream fout(fname);
    if (!fout) throw runtime_error("Cannot save accounts");
    for (int i = 0; i < count; i++) {
        fout << "Account," << accounts[i].getName()
             << "," << accounts[i].getBalance() << "\n";
    }
    fout << "Loan," 
         << loan.getName() << ","
         << loan.getBalance() << ","
         << loan.getEMI() << ","
         << loan.getInterestRate() << "\n";

    fout.close();
}
bool loadAccounts(Account accounts[], int &count, Loan &loan, const string &user) {
    string fname = "accounts_" + user + ".txt";
    ifstream fin(fname);
    if (!fin) return false;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        string parts[5];
        int idx = 0;
        size_t pos = 0, start = 0;
        while ((pos = line.find(',', start)) != string::npos && idx < 4) {
            parts[idx++] = line.substr(start, pos - start);
            start = pos + 1;
        }
        parts[idx] = line.substr(start);
        if (parts[0] == "Account") {
            accounts[count].setName(parts[1]);
            accounts[count].setBalance(stod(parts[2]));
            count++;
        } else if (parts[0] == "Loan") {
            loan.setName(parts[1]);
            loan.setBalance(stod(parts[2]));
            loan.setEMI(stod(parts[3]));
            loan.setInterestRate(stod(parts[4]));
        }
    }
    fin.close();
    return true;
}
class Budget {
    double monthlyLimit;
public:
    Budget() : monthlyLimit(0) {}
    void setLimit() {
        cout << "Enter monthly limit: ";
        cin >> monthlyLimit;
    }
    bool checkLimit(double totalExpense) {
        if (monthlyLimit > 0 && totalExpense > monthlyLimit) {
            cout << "Warning! You exceeded the monthly limit.\n";
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
        if (income <= 1000000) return 12500 + (income - 500000) * 0.20;
        return 112500 + (income - 1000000) * 0.30;
    }
};
class SavingGoal {
    double goalAmount;
public:
    SavingGoal() : goalAmount(0) {}
    void setGoal() {
        cout << "Enter saving goal: ";
        cin >> goalAmount;
    }
    void checkGoal(double savings) {
        if (goalAmount > 0) {
            if (savings >= goalAmount)
                cout << "Goal achieved!\n";
            else
                cout << "Need " << (goalAmount - savings) << " more.\n";
        }
    }
};
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
    FinanceManager(const string &user)
        : transactionCount(0), accountCount(0),
          totalIncome(0), totalExpense(0), username(user)
    {
        bool loaded = false;
        try {
            loaded = loadAccounts(accounts, accountCount, loan, username);
        } catch (...) {}
        if (!loaded) {
            cout << "\n--- First-time setup ---\n";
            double b;
            cout << "Initial balance for Cash: ";
            cin >> b;
            accounts[accountCount++] = Account("Cash", b);
            cout << "Initial balance for Bank: ";
            cin >> b;
            accounts[accountCount++] = Account("Bank", b);
            cout << "Initial balance for Savings: ";
            cin >> b;
            accounts[accountCount++] = Account("Savings", b);
            loan.createLoan();
        }
        transactionCount = loadTransactions(transactions, MAX_TRANSACTIONS, username);
        for (int i = 0; i < transactionCount; i++) {
            if (transactions[i].getIsIncome())
                totalIncome += transactions[i].getAmount();
            else
                totalExpense += transactions[i].getAmount();
        }
    }
    int findAccountIndex(const string &name) {
        for (int i = 0; i < accountCount; i++)
            if (accounts[i].getName() == name)
                return i;
        return -1;
    }
    void addTransaction() {
        if (transactionCount >= MAX_TRANSACTIONS) {
            cout << "Transaction limit reached.\n";
            return;
        }
        Transaction t;
        try {
            t.input();
        } catch (exception &e) {
            cout << "Error: " << e.what() << "\n";
            return;
        }
        int idx = findAccountIndex(t.getAccountType());
        if (idx >= 0) {
            try {
                accounts[idx].updateBalance(t.getAmount(), t.getIsIncome());
            } catch (exception &e) {
                cout << "Balance update failed: " << e.what() << "\n";
            }
        } else if (loan.getName() == t.getAccountType()) {
            cout << "Note: Loan balance must be updated using EMI.\n";
        } else {
            cout << "Account not found. Transaction recorded only.\n";
        }
        transactions[transactionCount++] = t;
        if (t.getIsIncome())
            totalIncome += t.getAmount();
        else
            totalExpense += t.getAmount();

        cout << "Transaction added.\n";
    }

    void viewReport() {
        cout << "\n--- Report ---\n";
        for (int i = 0; i < transactionCount; i++)
            transactions[i].display();
        cout << "\nTotal Income: " << totalIncome;
        cout << "\nTotal Expense: " << totalExpense;
        cout << "\nNet Balance: " << totalIncome - totalExpense << "\n";
    }
    void viewAccounts() {
        cout << "\n--- Accounts ---\n";
        for (int i = 0; i < accountCount; i++)
            accounts[i].displayAccount();

        loan.displayAccount();
    }
    void payLoanEMI() { loan.payEMI(); }
    void setBudget() { budget.setLimit(); }
    void checkBudget() { budget.checkLimit(totalExpense); }
    void setSavingGoal() { goal.setGoal(); }
    void checkGoal() { goal.checkGoal(totalIncome - totalExpense); }
    void calculateTax() {
        cout << "Estimated Tax: " 
             << TaxCalculator::calculateTax(totalIncome) << "\n";
    }
    void saveAndExit() {
        saveTransactions(transactions, transactionCount, username);
        saveAccounts(accounts, accountCount, loan, username);
        cout << "Data saved. Goodbye.\n";
    }
};
int main() {
    User currentUser;
    int choice;
    bool loggedIn = false;
    cout << "=== Personal Finance Tracker ===\n";
    while (!loggedIn) {
        cout << "\n1. Sign Up\n2. Login\n3. Exit\nEnter choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input\n";
            continue;
        }
        if (choice == 1) {
            currentUser.signup();
        } 
        else if (choice == 2) {
            if (currentUser.login())
                loggedIn = true;
        }
        else if (choice == 3) {
            cout << "Exiting...\n";
            return 0;
        }
        else {
            cout << "Invalid choice.\n";
        }
    }
    FinanceManager manager(currentUser.getUsername());
    int option;
    do {
        cout << "\n=== Menu ===\n"
             << "1. Add Transaction\n"
             << "2. View Report\n"
             << "3. View Accounts\n"
             << "4. Set Budget\n"
             << "5. Check Budget\n"
             << "6. Set Saving Goal\n"
             << "7. Check Goal\n"
             << "8. Pay Loan EMI\n"
             << "9. Calculate Tax\n"
             << "10. Save & Exit\n"
             << "Enter choice: ";
        if (!(cin >> option)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input\n";
            continue;
        }
        switch (option) {
            case 1: manager.addTransaction(); break;
            case 2: manager.viewReport(); break;
            case 3: manager.viewAccounts(); break;
            case 4: manager.setBudget(); break;
            case 5: manager.checkBudget(); break;
            case 6: manager.setSavingGoal(); break;
            case 7: manager.checkGoal(); break;
            case 8: manager.payLoanEMI(); break;
            case 9: manager.calculateTax(); break;
            case 10: manager.saveAndExit(); break;
            default: cout << "Invalid option.\n";
        }
    } while (option != 10);
    return 0;
}
