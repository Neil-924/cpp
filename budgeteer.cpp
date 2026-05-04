#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>

using namespace std;

struct DailyExpense {
    double percentage;
    double totalBudget;
    double dailyBudget;
};

struct OneTimeExpense {
    double amount;
    string day;
};

const vector<string> DAYS_ORDER = {
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
};

double weeklyBudget = 0.0;

vector<string> daysIncluded;

unordered_map<string, DailyExpense>   dailyExpenses;
unordered_map<string, OneTimeExpense> oneTimeExpenses;

void clearInput() {
    cin.clear();
    cin.ignore(1000, '\n');
}

string capitalize(string s) {
    if (!s.empty()) {
        s[0] = toupper(s[0]);
        for (size_t i = 1; i < s.size(); ++i)
            s[i] = tolower(s[i]);
    }
    return s;
}

bool isValidDay(const string& day) {
    for (const auto& d : DAYS_ORDER)
        if (d == day) return true;
    return false;
}

int getIntChoice(const string& prompt, int lo, int hi) {
    int choice;
    while (true) {
        cout << prompt;
        if (cin >> choice && choice >= lo && choice <= hi) {
            clearInput();
            return choice;
        }
        clearInput();
        cout << "  Error: Enter a number from " << lo << " to " << hi << ".\n";
    }
}

double getPositiveDouble(const string& prompt) {
    double val = 0.0;
    while (true) {
        cout << prompt;
        if (cin >> val && val > 0) {
            clearInput();
            return val;
        }
        clearInput();
        cout << "  Error: Enter a valid positive number.\n";
    }
}

void recalculate() {
    double oneTimeTotal = 0.0;
    for (const auto& [name, e] : oneTimeExpenses)
        oneTimeTotal += e.amount;

    double remaining = weeklyBudget - oneTimeTotal;

    size_t numDays = daysIncluded.size();

    for (auto& [name, e] : dailyExpenses) {
        e.totalBudget = remaining * (e.percentage / 100.0);
        e.dailyBudget = (numDays > 0) ? e.totalBudget / numDays : 0.0;
    }

}

void printDivider(char c = '=', int n = 52) {
    cout << string(n, c) << "\n";
}

//days
void printDays() {
    cout << "  Days included: ";
    if (daysIncluded.empty()) {
        cout << "(none)\n";
    } else {
        for (size_t i = 0; i < daysIncluded.size(); ++i) {
            cout << daysIncluded[i];
            if (i + 1 < daysIncluded.size()) cout << ", ";
        }
        cout << "\n";
    }
}

void addDay() {
    printDivider();
    cout << "  ADD DAY\n";
    printDivider();
    printDays();

    cout << "  Enter day of the week: ";
    string day;
    cin >> day;
    day = capitalize(day);

    if (!isValidDay(day)) {
        cout << "  \"" << day << "\" is not a valid day. Try again.\n";
        return;
    }

    auto it = find(daysIncluded.begin(), daysIncluded.end(), day);
    if (it != daysIncluded.end()) {
        cout << "  " << day << " is already included.\n";
        return;
    }

    daysIncluded.push_back(day);

    for (size_t i = 0; i < daysIncluded.size() - 1; ++i) {
        for (size_t j = 0; j < daysIncluded.size() - 1 - i; ++j) {
            auto idxA = find(DAYS_ORDER.begin(), DAYS_ORDER.end(), daysIncluded[j]) - DAYS_ORDER.begin();
            auto idxB = find(DAYS_ORDER.begin(), DAYS_ORDER.end(), daysIncluded[j + 1]) - DAYS_ORDER.begin();
            if (idxA > idxB)
                swap(daysIncluded[j], daysIncluded[j + 1]);
        }
    }

    recalculate();
    cout << "  " << day << " added!\n";
}

void removeDay() {
    printDivider();
    cout << "  REMOVE DAY\n";
    printDivider();
    printDays();

    cout << "  1. Remove ALL days\n";
    cout << "  2. Remove a specific day\n";
    cout << "  3. Go back\n";
    int choice = getIntChoice("  Enter choice (1-3): ", 1, 3);

    if (choice == 1) {
        daysIncluded.clear();
        recalculate();
        cout << "  All days removed.\n";
    } else if (choice == 2) {
        cout << "  Enter day to remove: ";
        string day;
        cin >> day;
        day = capitalize(day);

        auto it = find(daysIncluded.begin(), daysIncluded.end(), day);
        if (it == daysIncluded.end()) {
            cout << "  " << day << " is not in the list.\n";
            return;
        }

        daysIncluded.erase(it);
        recalculate();
        cout << "  " << day << " removed!\n";
    }
}

//daily
void addDailyExpense() {
    printDivider();
    cout << "  ADD DAILY EXPENSE\n";
    printDivider();

    double usedPercent = 0.0;
    for (const auto& [n, e] : dailyExpenses)
        usedPercent += e.percentage;
    double availPercent = 100.0 - usedPercent;

    double oneTimeTotal = 0.0;
    for (const auto& [n, e] : oneTimeExpenses)
        oneTimeTotal += e.amount;
    double remaining = weeklyBudget - oneTimeTotal;

    cout << fixed << setprecision(2);
    cout << "  Available %  : " << availPercent << "%\n";
    cout << "  Applies on   : PHP " << remaining << " (budget minus one-time)\n";

    if (availPercent <= 0) {
        cout << "  Daily expenses already at 100%. Remove one to add more.\n";
        return;
    }

    cout << "  Expense name : ";
    string name;
    getline(cin, name);

    if (name.empty()) {
        cout << "  Name cannot be empty.\n";
        return;
    }
    if (dailyExpenses.count(name)) {
        cout << "  \"" << name << "\" already exists.\n";
        return;
    }

    double pct = getPositiveDouble("  Percentage (1-" + to_string((int)availPercent) + "): ");
    if (pct > availPercent) {
        cout << "  Too much! Only " << availPercent << "% available.\n";
        return;
    }

    dailyExpenses[name] = { pct, 0.0, 0.0 };
    recalculate();
    cout << "  \"" << name << "\" added at " << pct << "%!\n";
}

void removeDailyExpense() {
    if (dailyExpenses.empty()) {
        cout << "  No daily expenses to remove.\n";
        return;
    }

    printDivider();
    cout << "  REMOVE DAILY EXPENSE\n";
    printDivider();
    cout << "  1. Remove ALL daily expenses\n";
    cout << "  2. Remove a specific expense\n";
    cout << "  3. Go back\n";
    int choice = getIntChoice("  Enter choice (1-3): ", 1, 3);

    if (choice == 1) {
        dailyExpenses.clear();
        recalculate();
        cout << "  All daily expenses removed.\n";
    } else if (choice == 2) {
        cout << "  Current daily expenses:\n";
        for (const auto& [n, e] : dailyExpenses)
            cout << "    - " << n << " (" << e.percentage << "%)\n";

        cout << "  Enter expense name to remove: ";
        string name;
        getline(cin, name);

        if (!dailyExpenses.count(name)) {
            cout << "  \"" << name << "\" not found.\n";
            return;
        }

        dailyExpenses.erase(name);
        recalculate();
        cout << "  \"" << name << "\" removed!\n";
    }
}

//onetime
void addOneTimeExpense() {
    printDivider();
    cout << "  ADD ONE-TIME EXPENSE\n";
    printDivider();

    double oneTimeTotal = 0.0;
    for (const auto& [n, e] : oneTimeExpenses)
        oneTimeTotal += e.amount;

    double availableForOneTime = weeklyBudget - oneTimeTotal;
    cout << fixed << setprecision(2);
    cout << "  Available amount : PHP " << availableForOneTime << "\n";
    cout << "  (One-time expenses reduce the base for daily % calculations)\n\n";

    cout << "  Expense name : ";
    string name;
    getline(cin, name);

    if (name.empty()) {
        cout << "  Name cannot be empty.\n";
        return;
    }
    if (oneTimeExpenses.count(name)) {
        cout << "  \"" << name << "\" already exists.\n";
        return;
    }

    double amount = getPositiveDouble("  Amount (PHP): ");
    if (amount > availableForOneTime) {
        cout << "  Too much! Only PHP " << availableForOneTime << " available.\n";
        return;
    }

    cout << "  Day to pay   : ";
    string day;
    cin >> day;
    day = capitalize(day);

    if (!isValidDay(day)) {
        cout << "  \"" << day << "\" is not a valid day.\n";
        return;
    }

    oneTimeExpenses[name] = { amount, day };
    recalculate();
    cout << "  \"" << name << "\" added! Due on " << day << ".\n";
}

void removeOneTimeExpense() {
    if (oneTimeExpenses.empty()) {
        cout << "  No one-time expenses to remove.\n";
        return;
    }

    printDivider();
    cout << "  REMOVE ONE-TIME EXPENSE\n";
    printDivider();
    cout << "  1. Remove ALL one-time expenses\n";
    cout << "  2. Remove a specific expense\n";
    cout << "  3. Go back\n";
    int choice = getIntChoice("  Enter choice (1-3): ", 1, 3);

    if (choice == 1) {
        oneTimeExpenses.clear();
        recalculate();
        cout << "  All one-time expenses removed.\n";
    } else if (choice == 2) {
        cout << "  Current one-time expenses:\n";
        for (const auto& [n, e] : oneTimeExpenses)
            cout << "    - " << n << " (PHP " << e.amount << " on " << e.day << ")\n";

        cout << "  Enter expense name to remove: ";
        string name;
        getline(cin, name);

        if (!oneTimeExpenses.count(name)) {
            cout << "  \"" << name << "\" not found.\n";
            return;
        }

        oneTimeExpenses.erase(name);
        recalculate();
        cout << "  \"" << name << "\" removed!\n";
    }
}

void showSummary() {
    recalculate(); 

    double oneTimeTotal = 0.0;
    for (const auto& [n, e] : oneTimeExpenses)
        oneTimeTotal += e.amount;

    double dailyPercentUsed = 0.0;
    for (const auto& [n, e] : dailyExpenses)
        dailyPercentUsed += e.percentage;

    double remaining = weeklyBudget - oneTimeTotal;
    double unallocated = remaining * ((100.0 - dailyPercentUsed) / 100.0);

    cout << "\n";
    printDivider();
    cout << "  BUDGET SUMMARY\n";
    printDivider();
    cout << fixed << setprecision(2);
    cout << "  Weekly Budget  : PHP " << weeklyBudget << "\n";
    cout << "  One-time Total : PHP " << oneTimeTotal << "\n";
    cout << "  Remaining Base : PHP " << remaining << "\n";
    cout << "  Unallocated    : PHP " << unallocated
              << " (" << (100.0 - dailyPercentUsed) << "% of remaining)\n";
    printDivider('-', 52);

    cout << "  DAILY BREAKDOWN (per included day)\n";
    printDivider('-', 52);
    if (daysIncluded.empty()) {
        cout << "  (No days included - daily budgets show as 0)\n";
    }

    for (const auto& day : DAYS_ORDER) {
        bool included = find(daysIncluded.begin(), daysIncluded.end(), day)
                        != daysIncluded.end();

        cout << "\n  [ " << day << (included ? " [INCLUDED]" : " [not included]") << " ]\n";

        if (included && !dailyExpenses.empty()) {
            cout << "    DAILY EXPENSES:\n";
            for (const auto& [name, e] : dailyExpenses) {
                cout << "      " << left << setw(20) << name
                          << " PHP " << right << setw(8) << e.dailyBudget
                          << "/day   (" << e.percentage << "% = PHP "
                          << e.totalBudget << "/week)\n";
            }
        } else if (included) {
            cout << "    DAILY EXPENSES: (none)\n";
        }

        bool hasOneTime = false;
        for (const auto& [name, e] : oneTimeExpenses)
            if (e.day == day) hasOneTime = true;

        if (hasOneTime) {
            cout << "    ONE-TIME:\n";
            for (const auto& [name, e] : oneTimeExpenses) {
                if (e.day == day)
                    cout << "      " << left << setw(20) << name
                              << " PHP " << right << setw(8) << e.amount << "\n";
            }
        }
    }

    printDivider();
}

void resetBudget() {
    cout << "  WARNING: This will clear all expenses and reset the budget.\n";
    cout << "  Confirm? (y/n): ";
    char c;
    cin >> c;
    if (tolower(c) != 'y') {
        cout << "  Cancelled.\n";
        return;
    }

    weeklyBudget = 0.0;
    daysIncluded.clear();
    dailyExpenses.clear();
    oneTimeExpenses.clear();

    weeklyBudget = getPositiveDouble("  Enter new weekly budget (PHP): ");
    cout << "  Budget reset to PHP " << fixed << setprecision(2) << weeklyBudget << ".\n";
}

void expensesMenu() {
    while (true) {
        printDivider();
        cout << "  EXPENSES\n";
        printDivider();
        cout << "  1. Add daily expense\n";
        cout << "  2. Remove daily expense\n";
        cout << "  3. Add one-time expense\n";
        cout << "  4. Remove one-time expense\n";
        cout << "  5. Go back\n";

        int choice = getIntChoice("  Enter choice (1-5): ", 1, 5);
        switch (choice) {
            case 1: addDailyExpense();    break;
            case 2: removeDailyExpense(); break;
            case 3: addOneTimeExpense();  break;
            case 4: removeOneTimeExpense(); break;
            case 5: return;
        }
    }
}

void daysMenu() {
    while (true) {
        printDivider();
        cout << "  DAYS\n";
        printDivider();
        printDays();
        cout << "  1. Add a day\n";
        cout << "  2. Remove a day\n";
        cout << "  3. Go back\n";

        int choice = getIntChoice("  Enter choice (1-3): ", 1, 3);
        switch (choice) {
            case 1: addDay();    break;
            case 2: removeDay(); break;
            case 3: return;
        }
    }
}

void mainMenu() {
    while (true) {
        cout << "\n";
        printDivider();
        cout << "  LET'S GO BUDGET!\n";
        printDivider();
        cout << fixed << setprecision(2);
        cout << "  Budget: PHP " << weeklyBudget << "\n";
        printDays();
        printDivider('-', 52);
        cout << "  1. Manage Days\n";
        cout << "  2. Manage Expenses\n";
        cout << "  3. View Summary\n";
        cout << "  4. Reset Budget\n";
        cout << "  5. Exit\n";
        printDivider();

        int choice = getIntChoice("  Enter choice (1-5): ", 1, 5);
        switch (choice) {
            case 1: daysMenu();     break;
            case 2: expensesMenu(); break;
            case 3: showSummary();  break;
            case 4: resetBudget();  break;
            case 5:
                cout << "  Goodbye!\n";
                return;
        }
    }
}

int main() {
    cout << "\n";
    printDivider();
    cout << "  WELCOME TO LET'S GO BUDGET!\n";
    cout << "  Budgeted Money For a Wiser Week.\n";
    printDivider();

    weeklyBudget = getPositiveDouble("  Enter your weekly budget (PHP): ");

    daysIncluded = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    cout << "  Default days set: Mon, Tue, Wed, Thu, Fri.\n";
    cout << "  You can change these in the Days menu.\n";

    mainMenu();
    return 0;
}
