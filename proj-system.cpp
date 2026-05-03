#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <cmath>

// ─────────────────────────────────────────────
//  CONSTANTS
// ─────────────────────────────────────────────

const std::vector<std::string> DAYS_ORDER = {
    "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday", "Sunday"
};

// ─────────────────────────────────────────────
//  STRUCTS
// ─────────────────────────────────────────────

struct DailyExpense {
    double percentage;
    double total_budget;
    double daily_budget;
};

struct OneTimeExpense {
    double amount;
    std::string day;
    double percentage_equivalence;
};

struct BudgetSystem {
    double weekly_budget = 0;

    // VECTOR — supports addition and deletion of specific days
    std::vector<std::string> days_included = {
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
    };

    // UNORDERED MAP — fast key-value access for expense name -> expense details
    std::unordered_map<std::string, DailyExpense>   daily_expenses;
    std::unordered_map<std::string, OneTimeExpense> one_time_expenses;

    // derived — updated by recalculate()
    double daily_percent_total = 0;
    double one_time_total      = 0;
    double remaining           = 0;    // weekly_budget - one_time_total
    double percent_remaining   = 100;  // 100 - daily_percent_total
    double budget_remaining    = 0;    // remaining * (percent_remaining / 100)
};

// ─────────────────────────────────────────────
//  HELPERS
// ─────────────────────────────────────────────

double round2(double v) {
    return std::round(v * 100.0) / 100.0;
}

std::string capitalize(const std::string& s) {
    if (s.empty()) return s;
    std::string out = s;
    out[0] = toupper(out[0]);
    for (size_t i = 1; i < out.size(); i++)
        out[i] = tolower(out[i]);
    return out;
}

bool is_valid_day(const std::string& day) {
    return std::find(DAYS_ORDER.begin(), DAYS_ORDER.end(), day) != DAYS_ORDER.end();
}

// ─────────────────────────────────────────────
//  CORE LOGIC
// ─────────────────────────────────────────────

void recalculate(BudgetSystem& bs) {
    bs.daily_percent_total = 0;
    for (auto& [name, e] : bs.daily_expenses)
        bs.daily_percent_total += e.percentage;

    bs.one_time_total = 0;
    for (auto& [name, e] : bs.one_time_expenses)
        bs.one_time_total += e.amount;

    bs.remaining         = bs.weekly_budget - bs.one_time_total;
    bs.percent_remaining = 100.0 - bs.daily_percent_total;
    bs.budget_remaining  = bs.remaining * (bs.percent_remaining / 100.0);

    int day_count = (int)bs.days_included.size();

    for (auto& [name, e] : bs.daily_expenses) {
        double new_total = bs.remaining * (e.percentage / 100.0);
        e.total_budget   = round2(new_total);
        e.daily_budget   = (day_count > 0) ? round2(new_total / day_count) : 0.0;
    }
}

void toggle_day(BudgetSystem& bs, const std::string& day) {
    if (!is_valid_day(day))
        throw std::invalid_argument("Invalid day: " + day);

    // VECTOR — find and erase, or push_back, then re-sort by DAYS_ORDER
    auto it = std::find(bs.days_included.begin(), bs.days_included.end(), day);
    if (it != bs.days_included.end()) {
        bs.days_included.erase(it);
    } else {
        bs.days_included.push_back(day);
        std::sort(bs.days_included.begin(), bs.days_included.end(),
            [](const std::string& a, const std::string& b) {
                auto ia = std::find(DAYS_ORDER.begin(), DAYS_ORDER.end(), a);
                auto ib = std::find(DAYS_ORDER.begin(), DAYS_ORDER.end(), b);
                return ia < ib;
            });
    }

    recalculate(bs);
}

void remove_all_days(BudgetSystem& bs) {
    bs.days_included.clear();   // VECTOR — clear()
    recalculate(bs);
}

void add_daily_expense(BudgetSystem& bs, const std::string& name, double percentage) {
    if (name.empty())
        throw std::invalid_argument("Expense name cannot be empty.");
    if (bs.daily_expenses.count(name))
        throw std::invalid_argument("Daily expense already exists: " + name);
    if (percentage <= 0)
        throw std::invalid_argument("Percentage must be greater than 0.");
    if (percentage > bs.percent_remaining)
        throw std::invalid_argument(
            "Only " + std::to_string(bs.percent_remaining) + "% available.");

    // UNORDERED MAP — insert by key
    bs.daily_expenses[name] = { round2(percentage), 0.0, 0.0 };
    recalculate(bs);
}

void add_one_time_expense(BudgetSystem& bs,
                          const std::string& name,
                          double amount,
                          const std::string& raw_day) {
    if (name.empty())
        throw std::invalid_argument("Expense name cannot be empty.");
    if (bs.one_time_expenses.count(name))
        throw std::invalid_argument("One-time expense already exists: " + name);
    if (amount <= 0)
        throw std::invalid_argument("Amount must be greater than 0.");
    if (amount > bs.budget_remaining)
        throw std::invalid_argument(
            "Only " + std::to_string(bs.budget_remaining) + " PHP available.");

    std::string day = capitalize(raw_day);
    if (!is_valid_day(day))
        throw std::invalid_argument("Not a valid day: " + day);

    double pct_equiv = (bs.weekly_budget > 0)
                     ? (amount / bs.weekly_budget) * 100.0
                     : 0.0;

    // UNORDERED MAP — insert by key
    bs.one_time_expenses[name] = { round2(amount), day, round2(pct_equiv) };
    recalculate(bs);
}

void remove_expense(BudgetSystem& bs,
                    const std::string& type,
                    const std::string& name) {
    if (type == "daily") {
        if (!bs.daily_expenses.count(name))
            throw std::invalid_argument("Daily expense not found: " + name);
        bs.daily_expenses.erase(name);     // UNORDERED MAP — erase by key
    } else if (type == "one_time") {
        if (!bs.one_time_expenses.count(name))
            throw std::invalid_argument("One-time expense not found: " + name);
        bs.one_time_expenses.erase(name);  // UNORDERED MAP — erase by key
    } else {
        throw std::invalid_argument("Unknown expense type: " + type);
    }
    recalculate(bs);
}

void remove_all_expenses(BudgetSystem& bs) {
    bs.daily_expenses.clear();    // UNORDERED MAP — clear()
    bs.one_time_expenses.clear(); // UNORDERED MAP — clear()
    recalculate(bs);
}

// ─────────────────────────────────────────────
//  DISPLAY
// ─────────────────────────────────────────────

void print_summary(const BudgetSystem& bs) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n====== BUDGET SUMMARY ======\n";
    std::cout << "Weekly Budget   : P" << bs.weekly_budget     << "\n";
    std::cout << "One-Time Total  : P" << bs.one_time_total    << "\n";
    std::cout << "Remaining Pool  : P" << bs.remaining         << "\n";
    std::cout << "% Still Free    : "  << bs.percent_remaining << "%\n";
    std::cout << "Budget Remaining: P" << bs.budget_remaining  << "\n";

    std::cout << "\n-- Days Included (vector) --\n";
    if (bs.days_included.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto& d : bs.days_included)
            std::cout << "  " << d << "\n";
    }

    std::cout << "\n-- Daily Expenses (unordered_map) --\n";
    if (bs.daily_expenses.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto& [name, e] : bs.daily_expenses) {
            std::cout << "  " << name
                      << "  |  " << e.percentage  << "%"
                      << "  |  P" << e.daily_budget << "/day"
                      << "  |  P" << e.total_budget << "/week\n";
        }
    }

    std::cout << "\n-- One-Time Expenses (unordered_map) --\n";
    if (bs.one_time_expenses.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto& [name, e] : bs.one_time_expenses) {
            std::cout << "  " << name
                      << "  |  P" << e.amount
                      << "  |  due " << e.day << "\n";
        }
    }
    std::cout << "============================\n\n";
}

// ─────────────────────────────────────────────
//  MAIN — demo
// ─────────────────────────────────────────────

int main() {
    BudgetSystem bs;
    bs.weekly_budget = 5000.0;
    recalculate(bs);

    std::cout << "=== Let's Go Budget (core system demo) ===\n";

    add_daily_expense(bs, "Food",      40.0);
    add_daily_expense(bs, "Transport", 20.0);

    add_one_time_expense(bs, "Electric Bill", 800.0, "monday");
    add_one_time_expense(bs, "Internet",      600.0, "friday");

    toggle_day(bs, "Saturday");
    toggle_day(bs, "Sunday");

    print_summary(bs);

    remove_expense(bs, "daily", "Transport");
    std::cout << ">> Removed Transport.\n";
    print_summary(bs);

    remove_all_days(bs);
    std::cout << ">> Removed all days.\n";
    print_summary(bs);

    return 0;
}