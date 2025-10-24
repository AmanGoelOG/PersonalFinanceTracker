#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
using namespace std;

// -------------------- USER CLASS --------------------
class User {
    string username, password;

    bool userExists(const string& uname) {
        ifstream fin("users.txt");
        if (!fin) return false;
        string u, p;
        while (fin >> u >> p) {
            if (u == uname) return true;
        }
        return false;
    }

public:
    string getUsername() const { return username; }

    bool signup() {
        cout << "\n=== SIGN UP ===\n";
        cout << "Enter new username: ";
        cin >> username;

        if (userExists(username)) {
            cout << "❌ Username already exists! Try another.\n";
            return false;
        }

        cout << "Enter new password: ";
        cin >> password;

        ofstream fout("users.txt", ios::app);
        if (!fout) throw runtime_error("Unable to open users.txt!");
        fout << username << " " << password << "\n";
        fout.close();

        cout << "✅ Account created successfully!\n";
        return true;
    }

    bool login() {
        cout << "\n=== LOGIN ===\n";
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;

        ifstream fin("users.txt");
        if (!fin) {
            cout << "No user data found. Please sign up first.\n";
            return false;
        }

        string u, p;
        while (fin >> u >> p) {
            if (u == username && p == password) {
                cout << "✅ Login successful! Welcome " << username << "\n\n";
                return true;
            }
        }

        cout << "❌ Invalid credentials!\n";
        return false;
    }
};

// -------------------- TRANSACTION STRUCT --------------------
struct Transaction {
    string date, description, category, accountType;
    double amount;
    bool isIncome;
};

// -------------------- FILE HANDLER CLASS --------------------
class FileHandler {
public:
    static void saveTransactions(const Transaction* list, int count, const string& username) {
        string filename = "transactions_" + username + ".txt";
        ofstream fout(filename, ios::out);
        if (!fout) throw runtime_error("Failed to save file!");
        for (int i = 0; i < count; i++) {
            fout << list[i].date << "," << list[i].description << "," << list[i].category << ","
                 << list[i].amount << "," << list[i].isIncome << "," << list[i].accountType << "\n";
        }
        fout.close();
    }

    static int loadTransactions(Transaction* list, const string& username) {
        string filename = "transactions_" + username + ".txt";
        ifstream fin(filename);
        if (!fin) return 0;
        string d, desc, cat, acc;
        double amt; int income, count = 0;
        while (getline(fin, d, ',')) {
            getline(fin, desc, ',');
            getline(fin, cat, ',');
            fin >> amt; fin.ignore();
            fin >> income; fin.ignore();
            getline(fin, acc);
            list[count++] = {d, desc, cat, acc, amt, (bool)income};
            if (count >= 100) break; // limit
        }
        fin.close();
        return count;
    }
};

// -------------------- ACCOUNT CLASS --------------------
class Account {
protected:
    string accountName;
    double balance;
public:
    Account() {}
    void createAccount() {
        cout << "Enter account name: ";
        cin >> accountName;
        cout << "Enter initial balance: ";
        cin >> balance;
    }
    virtual void displayAccount() {
        cout << "Account: " << accountName << " | Balance: " << balance << endl;
    }
    void updateBalance(double amt, bool income) {
        balance += (income ? amt : -amt);
    }
    string getName() const { return accountName; }
    double getBalance() const { return balance; }
};

// -------------------- LOAN CLASS --------------------
class Loan : public Account {
    double emi, interestRate;
public:
    Loan() {}
    void createLoan() {
        cout << "Enter loan name: ";
        cin >> accountName;
        cout << "Enter loan amount: ";
        cin >> balance;
        cout << "Enter EMI amount: ";
        cin >> emi;
        cout << "Enter interest rate (%): ";
        cin >> interestRate;
    }
    void displayAccount() override {
        cout << "Loan: " << accountName << " | Outstanding: " << balance
             << " | EMI: " << emi << " | Interest: " << interestRate << "%\n";
    }
    void payEMI() {
        balance -= emi;
        if (balance < 0) balance = 0;
        cout << "EMI of " << emi << " paid. Remaining balance: " << balance << endl;
    }
};

// -------------------- BUDGET CLASS --------------------
class Budget {
    double monthlyLimit;
public:
    void setLimit() {
        cout << "Enter monthly limit: ";
        cin >> monthlyLimit;
    }
    bool checkLimit(double totalExpense) {
        if (totalExpense > monthlyLimit && monthlyLimit > 0) {
            cout << "⚠️ Warning! You exceeded your monthly budget of " << monthlyLimit << endl;
            return false;
        }
        return true;
    }
};

// -------------------- TAX CALCULATOR --------------------
class TaxCalculator {
public:
    static double calculateTax(double income) {
        double tax = 0;
        if (income <= 250000) tax = 0;
        else if (income <= 500000) tax = (income - 250000) * 0.05;
        else if (income <= 1000000) tax = 12500 + (income - 500000) * 0.2;
        else tax = 112500 + (income - 1000000) * 0.3;
        return tax;
    }
};

// -------------------- SAVING GOAL --------------------
class SavingGoal {
    double goalAmount;
public:
    void setGoal() {
        cout << "Enter goal amount: ";
        cin >> goalAmount;
    }
    void checkGoal(double currentSavings) {
        if (goalAmount > 0) {
            if (currentSavings >= goalAmount)
                cout << "🎉 Congratulations! You reached your saving goal of " << goalAmount << endl;
            else
                cout << "Keep saving! You need " << (goalAmount - currentSavings) << " more.\n";
        }
    }
};

// -------------------- FINANCE MANAGER --------------------
class FinanceManager {
    Transaction transactions[100];
    int count;
    Account accounts[3];
    Loan loan;
    Budget budget;
    SavingGoal goal;
    double totalIncome = 0, totalExpense = 0;
    string username;

public:
    FinanceManager(const string& user) : username(user) {
        cout << "\n--- Create 3 accounts ---\n";
        for (int i = 0; i < 3; i++) accounts[i].createAccount();

        cout << "\n--- Create your loan ---\n";
        loan.createLoan();

        count = FileHandler::loadTransactions(transactions, username);
    }

    void addTransaction() {
        try {
            if (count >= 100) throw overflow_error("Transaction limit reached!");
            Transaction t;
            cout << "Enter date (DD-MM-YYYY): "; cin >> t.date;
            cout << "Enter description: "; cin >> t.description;
            cout << "Enter category: "; cin >> t.category;
            cout << "Enter amount: "; cin >> t.amount;
            cout << "Enter account (exact name as created): "; cin >> t.accountType;
            char type; cout << "Income (i) or Expense (e)? "; cin >> type;
            t.isIncome = (type == 'i');
            transactions[count++] = t;

            for (int i = 0; i < 3; i++)
                if (accounts[i].getName() == t.accountType)
                    accounts[i].updateBalance(t.amount, t.isIncome);

            if (t.isIncome) totalIncome += t.amount; else totalExpense += t.amount;

            cout << "Transaction added successfully!\n";
        }
        catch (exception& e) {
            cout << "⚠️ Error: " << e.what() << endl;
        }
    }

    void viewReport() {
        cout << "\n--- Transaction Report ---\n";
        for (int i = 0; i < count; i++) {
            cout << transactions[i].date << " | " << transactions[i].description << " | "
                 << transactions[i].category << " | "
                 << (transactions[i].isIncome ? "Income" : "Expense") << " | "
                 << transactions[i].amount << " | " << transactions[i].accountType << endl;
        }
        cout << "\nTotal Income: " << totalIncome;
        cout << "\nTotal Expense: " << totalExpense;
        cout << "\nBalance: " << (totalIncome - totalExpense) << "\n";
    }

    void viewAccounts() {
        cout << "\n--- Accounts ---\n";
        for (int i = 0; i < 3; i++) accounts[i].displayAccount();
        loan.displayAccount();
    }

    void payLoanEMI() { loan.payEMI(); }
    void setBudget() { budget.setLimit(); }
    void checkBudget() { budget.checkLimit(totalExpense); }
    void setSavingGoal() { goal.setGoal(); }
    void checkGoal() { goal.checkGoal(totalIncome - totalExpense); }
    void calculateTax() {
        double tax = TaxCalculator::calculateTax(totalIncome);
        cout << "\nEstimated Tax: " << tax << endl;
    }

    void saveAndExit() {
        FileHandler::saveTransactions(transactions, count, username);
        cout << "Data saved successfully. Goodbye!\n";
    }
};

// -------------------- MAIN FUNCTION --------------------
int main() {
    User user;
    int choice;
    bool loggedIn = false;

    try {
        cout << "=== Personal Finance Tracker ===\n";

        do {
            cout << "\n1. Sign Up\n2. Login\n3. Exit\nEnter choice: ";
            cin >> choice;

            if (cin.fail()) throw invalid_argument("Invalid input type!");

            if (choice == 1) user.signup();
            else if (choice == 2) {
                if (user.login()) {
                    loggedIn = true;
                    break;
                }
            }
            else if (choice == 3) {
                cout << "Exiting...\n";
                return 0;
            }
            else cout << "Invalid choice.\n";
        } while (!loggedIn);

        if (loggedIn) {
            FinanceManager manager(user.getUsername());
            int option;
            do {
                cout << "\n=== Menu ===\n";
                cout << "1. Add Transaction\n2. View Report\n3. View Accounts\n4. Set Budget\n5. Check Budget\n";
                cout << "6. Set Saving Goal\n7. Check Goal\n8. Pay Loan EMI\n9. Calculate Tax\n10. Save & Exit\n";
                cout << "Enter choice: ";
                cin >> option;

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
        }
    }
    catch (exception& e) {
        cout << "⚠️ Exception: " << e.what() << endl;
    }

    return 0;
}
