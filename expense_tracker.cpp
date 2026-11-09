// ============================================================================
//  Personal Expense Tracker for College Students
//  Language : C++17 (console application)
//  Build    : g++ -std=c++17 -Wall -Wextra -o expense_tracker expense_tracker.cpp
//
//  OOP concepts demonstrated
//  -------------------------
//  Encapsulation  : Date, Budget, Tracker keep their data private.
//  Abstraction    : Transaction is an abstract base class (pure virtual methods).
//  Inheritance    : Expense and Income derive from Transaction.
//  Polymorphism   : Tracker stores unique_ptr<Transaction> and calls virtual
//                   methods without knowing the concrete type.
//  Operator overloading : Date::operator< and operator<<.
//  Factory        : Tracker::fromCsv() builds the right subclass from a file row.
//  RAII / STL     : smart pointers, vector, map, algorithms, fstream.
// ============================================================================

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const string CURRENCY  = "Rs. ";               // change here for another currency
const string DATA_FILE = "tracker_data.csv";   // saved in the folder you run from

// Spending categories that suit a hostel / college routine.
const vector<string> EXPENSE_CATEGORIES = {
    "Food", "Travel", "Books & Stationery", "Mobile & Internet",
    "Entertainment", "Shopping", "Rent & Hostel", "Health", "Other"};

// ----------------------------------------------------------------------------
//  Date : small value class with validation
// ----------------------------------------------------------------------------
class Date {
    int day_, month_, year_;

public:
    Date() {                                     // defaults to today
        time_t now = time(nullptr);
        tm *t = localtime(&now);
        day_ = t->tm_mday;
        month_ = t->tm_mon + 1;
        year_ = t->tm_year + 1900;
    }
    Date(int d, int m, int y) : day_(d), month_(m), year_(y) {}

    static bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

    static int daysIn(int m, int y) {
        static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        return (m == 2 && isLeap(y)) ? 29 : days[m - 1];
    }

    static bool isValid(int d, int m, int y) {
        return y >= 2000 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= daysIn(m, y);
    }

    // Accepts "dd-mm-yyyy". Returns false if malformed or not a real date.
    static bool parse(const string &s, Date &out) {
        int d, m, y;
        char a, b;
        istringstream in(s);
        if (!(in >> d >> a >> m >> b >> y) || a != '-' || b != '-') return false;
        if (in >> ws && !in.eof()) return false;
        if (!isValid(d, m, y)) return false;
        out = Date(d, m, y);
        return true;
    }

    int day() const { return day_; }
    int month() const { return month_; }
    int year() const { return year_; }

    string toString() const {
        ostringstream o;
        o << setfill('0') << setw(2) << day_ << '-' << setw(2) << month_ << '-' << year_;
        return o.str();
    }

    bool operator<(const Date &o) const {
        if (year_ != o.year_) return year_ < o.year_;
        if (month_ != o.month_) return month_ < o.month_;
        return day_ < o.day_;
    }

    friend ostream &operator<<(ostream &os, const Date &d) { return os << d.toString(); }
};

// ----------------------------------------------------------------------------
//  Transaction : abstract base class
// ----------------------------------------------------------------------------
class Transaction {
protected:
    int id_;
    Date date_;
    double amount_;
    string note_;

public:
    Transaction(int id, const Date &date, double amount, string note)
        : id_(id), date_(date), amount_(amount), note_(move(note)) {
        replace(note_.begin(), note_.end(), ',', ';');   // keep the CSV safe
    }
    virtual ~Transaction() = default;

    virtual string kind() const = 0;          // "EXPENSE" or "INCOME"
    virtual string tag() const = 0;           // category or source
    virtual double signedAmount() const = 0;  // -ve for spending, +ve for money in

    int id() const { return id_; }
    const Date &date() const { return date_; }
    double amount() const { return amount_; }
    const string &note() const { return note_; }

    string toCsv() const {
        ostringstream o;
        o << kind() << ',' << id_ << ',' << date_ << ',' << fixed << setprecision(2)
          << amount_ << ',' << tag() << ',' << note_;
        return o.str();
    }
};

class Expense : public Transaction {
    string category_;

public:
    Expense(int id, const Date &d, double amt, string category, string note)
        : Transaction(id, d, amt, move(note)), category_(move(category)) {}
    string kind() const override { return "EXPENSE"; }
    string tag() const override { return category_; }
    double signedAmount() const override { return -amount_; }
};

class Income : public Transaction {
    string source_;

public:
    Income(int id, const Date &d, double amt, string source, string note)
        : Transaction(id, d, amt, move(note)), source_(move(source)) {}
    string kind() const override { return "INCOME"; }
    string tag() const override { return source_; }
    double signedAmount() const override { return amount_; }
};

// ----------------------------------------------------------------------------
//  Budget : monthly spending limit
// ----------------------------------------------------------------------------
class Budget {
    double limit_ = 0;   // 0 means "no budget set"

public:
    void set(double limit) { limit_ = limit; }
    double limit() const { return limit_; }
    bool isSet() const { return limit_ > 0; }
    double usedPercent(double spent) const { return isSet() ? spent / limit_ * 100.0 : 0.0; }
};

// ----------------------------------------------------------------------------
//  Console input helpers (every read is validated, so bad input never crashes)
// ----------------------------------------------------------------------------
namespace io {

string readLine(const string &prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) {            // Ctrl+D / end of input
        cout << "\nInput closed. Exiting.\n";
        exit(0);
    }
    size_t a = s.find_first_not_of(" \t\r");
    size_t b = s.find_last_not_of(" \t\r");
    return a == string::npos ? "" : s.substr(a, b - a + 1);
}

int readInt(const string &prompt, int lo, int hi) {
    while (true) {
        string s = readLine(prompt);
        try {
            size_t pos;
            int v = stoi(s, &pos);
            if (pos == s.size() && v >= lo && v <= hi) return v;
        } catch (...) {}
        cout << "  Enter a whole number between " << lo << " and " << hi << ".\n";
    }
}

double readAmount(const string &prompt) {
    while (true) {
        string s = readLine(prompt);
        try {
            size_t pos;
            double v = stod(s, &pos);
            if (pos == s.size() && v > 0 && v < 10000000 && isfinite(v)) return round(v * 100) / 100;
        } catch (...) {}
        cout << "  Enter a positive amount, e.g. 120 or 49.50.\n";
    }
}

Date readDate(const string &prompt) {
    while (true) {
        string s = readLine(prompt + " (dd-mm-yyyy, Enter = today): ");
        if (s.empty()) return Date();
        Date d;
        if (Date::parse(s, d)) return d;
        cout << "  Not a valid date. Example: 05-11-2026\n";
    }
}

bool confirm(const string &prompt) {
    string s = readLine(prompt + " (y/n): ");
    return !s.empty() && (s[0] == 'y' || s[0] == 'Y');
}

}  // namespace io

// ----------------------------------------------------------------------------
//  Tracker : owns all transactions, does reports and file I/O
// ----------------------------------------------------------------------------
class Tracker {
    vector<unique_ptr<Transaction>> items_;
    Budget budget_;
    int nextId_ = 1;

    static string money(double v) {
        ostringstream o;
        o << CURRENCY << fixed << setprecision(2) << v;
        return o.str();
    }

    static string clip(const string &s, size_t n) {
        return s.size() <= n ? s : s.substr(0, n - 2) + "..";
    }

    // Factory: one CSV row -> correct subclass (polymorphism in action)
    static unique_ptr<Transaction> fromCsv(const string &line) {
        vector<string> f;
        string part;
        istringstream in(line);
        for (int i = 0; i < 5 && getline(in, part, ','); ++i) f.push_back(part);
        string note;
        getline(in, note);
        if (f.size() < 5) return nullptr;
        try {
            int id = stoi(f[1]);
            double amt = stod(f[3]);
            Date d;
            if (!Date::parse(f[2], d) || amt <= 0) return nullptr;
            if (f[0] == "EXPENSE") return make_unique<Expense>(id, d, amt, f[4], note);
            if (f[0] == "INCOME") return make_unique<Income>(id, d, amt, f[4], note);
        } catch (...) {}
        return nullptr;
    }

    void printHeader() const {
        cout << left << setw(5) << "ID" << setw(12) << "Date" << setw(9) << "Type"
             << setw(20) << "Category/Source" << right << setw(14) << "Amount" << "  "
             << left << "Note\n"
             << string(78, '-') << '\n';
    }

    void printRow(const Transaction &t) const {
        ostringstream amt;
        amt << (t.signedAmount() < 0 ? "-" : "+") << fixed << setprecision(2) << t.amount();
        cout << left << setw(5) << t.id() << setw(12) << t.date().toString() << setw(9)
             << t.kind() << setw(20) << clip(t.tag(), 19) << right << setw(14) << amt.str()
             << "  " << left << clip(t.note(), 22) << '\n';
    }

public:
    // ---- data changes ------------------------------------------------------
    void addExpense(const Date &d, double amt, const string &cat, const string &note) {
        items_.push_back(make_unique<Expense>(nextId_++, d, amt, cat, note));
    }

    void addIncome(const Date &d, double amt, const string &src, const string &note) {
        items_.push_back(make_unique<Income>(nextId_++, d, amt, src, note));
    }

    bool remove(int id) {
        auto it = find_if(items_.begin(), items_.end(),
                          [id](const unique_ptr<Transaction> &t) { return t->id() == id; });
        if (it == items_.end()) return false;
        items_.erase(it);
        return true;
    }

    const Transaction *find(int id) const {
        for (const auto &t : items_) if (t->id() == id) return t.get();
        return nullptr;
    }

    Budget &budget() { return budget_; }
    bool empty() const { return items_.empty(); }

    // ---- totals ------------------------------------------------------------
    double totalIncome() const {
        double s = 0;
        for (const auto &t : items_) if (t->signedAmount() > 0) s += t->amount();
        return s;
    }
    double totalExpense() const {
        double s = 0;
        for (const auto &t : items_) if (t->signedAmount() < 0) s += t->amount();
        return s;
    }
    double balance() const { return totalIncome() - totalExpense(); }

    double spentInMonth(int month, int year) const {
        double s = 0;
        for (const auto &t : items_)
            if (t->signedAmount() < 0 && t->date().month() == month && t->date().year() == year)
                s += t->amount();
        return s;
    }

    // ---- views -------------------------------------------------------------
    void showAll(bool newestFirst = true) const {
        if (items_.empty()) { cout << "\nNo transactions yet.\n"; return; }
        vector<const Transaction *> v;
        for (const auto &t : items_) v.push_back(t.get());
        sort(v.begin(), v.end(), [&](const Transaction *a, const Transaction *b) {
            if (a->date() < b->date()) return !newestFirst;
            if (b->date() < a->date()) return newestFirst;
            return newestFirst ? a->id() > b->id() : a->id() < b->id();
        });
        cout << '\n';
        printHeader();
        for (auto *t : v) printRow(*t);
        cout << string(78, '-') << '\n' << v.size() << " transaction(s)\n";
    }

    void showByCategory(const string &cat) const {
        vector<const Transaction *> v;
        double total = 0;
        for (const auto &t : items_)
            if (t->signedAmount() < 0 && t->tag() == cat) { v.push_back(t.get()); total += t->amount(); }
        if (v.empty()) { cout << "\nNo expenses found in \"" << cat << "\".\n"; return; }
        cout << '\n';
        printHeader();
        for (auto *t : v) printRow(*t);
        cout << string(78, '-') << "\nTotal in " << cat << ": " << money(total) << '\n';
    }

    void showSummary(int month, int year) const {
        double inc = 0, exp = 0;
        map<string, double> byCat;
        for (const auto &t : items_) {
            if (t->date().month() != month || t->date().year() != year) continue;
            if (t->signedAmount() > 0) inc += t->amount();
            else { exp += t->amount(); byCat[t->tag()] += t->amount(); }
        }

        cout << "\n===== Summary for " << setfill('0') << setw(2) << month << '-' << year
             << setfill(' ') << " =====\n";
        cout << "Money in    : " << money(inc) << '\n';
        cout << "Money spent : " << money(exp) << '\n';
        cout << "Saved       : " << money(inc - exp) << (inc - exp < 0 ? "  (overspent!)" : "") << '\n';

        if (budget_.isSet()) {
            double pct = budget_.usedPercent(exp);
            cout << "Budget      : " << money(budget_.limit()) << "  (" << fixed << setprecision(1)
                 << pct << "% used)\n";
            if (pct > 100) cout << "  !! You are over budget by " << money(exp - budget_.limit()) << '\n';
            else if (pct >= 80) cout << "  ! Careful: 80% of your budget is gone. "
                                     << money(budget_.limit() - exp) << " left.\n";
        }

        if (byCat.empty()) { cout << "\nNo expenses this month.\n"; return; }

        vector<pair<string, double>> sorted(byCat.begin(), byCat.end());
        sort(sorted.begin(), sorted.end(),
             [](const auto &a, const auto &b) { return a.second > b.second; });

        cout << "\nWhere the money went:\n";
        for (const auto &[cat, amt] : sorted) {
            double pct = amt / exp * 100.0;
            int bar = (int)round(pct / 100.0 * 30);
            cout << left << setw(20) << cat << right << setw(12) << fixed << setprecision(2) << amt
                 << setw(7) << setprecision(1) << pct << "%  " << string(bar, '#') << '\n';
        }
        cout << "\nTop spend: " << sorted.front().first << '\n';
    }

    void showOverview() const {
        cout << "\n===== Overall Balance =====\n";
        cout << "Total money in : " << money(totalIncome()) << '\n';
        cout << "Total spent    : " << money(totalExpense()) << '\n';
        cout << "Balance left   : " << money(balance()) << (balance() < 0 ? "  (negative!)" : "") << '\n';
        Date today;
        double m = spentInMonth(today.month(), today.year());
        cout << "Spent this month: " << money(m);
        if (budget_.isSet())
            cout << "  of " << money(budget_.limit()) << " (" << fixed << setprecision(1)
                 << budget_.usedPercent(m) << "%)";
        cout << '\n';
    }

    // ---- file I/O ----------------------------------------------------------
    bool save(const string &path) const {
        ofstream out(path);
        if (!out) return false;
        out << "BUDGET," << fixed << setprecision(2) << budget_.limit() << '\n';
        for (const auto &t : items_) out << t->toCsv() << '\n';
        return out.good();
    }

    int load(const string &path) {
        ifstream in(path);
        if (!in) return 0;               // first run: no file yet
        items_.clear();
        string line;
        int bad = 0;
        while (getline(in, line)) {
            if (line.empty()) continue;
            if (line.rfind("BUDGET,", 0) == 0) {
                try { budget_.set(stod(line.substr(7))); } catch (...) {}
                continue;
            }
            auto t = fromCsv(line);
            if (!t) { ++bad; continue; }
            nextId_ = max(nextId_, t->id() + 1);
            items_.push_back(move(t));
        }
        return bad;
    }
};

// ----------------------------------------------------------------------------
//  App : menu and user interaction
// ----------------------------------------------------------------------------
class App {
    Tracker tracker_;

    void autosave() {
        if (!tracker_.save(DATA_FILE)) cout << "  Warning: could not save to " << DATA_FILE << '\n';
    }

    string pickCategory() const {
        cout << "\nCategories:\n";
        for (size_t i = 0; i < EXPENSE_CATEGORIES.size(); ++i)
            cout << "  " << i + 1 << ". " << EXPENSE_CATEGORIES[i] << '\n';
        int c = io::readInt("Choose category: ", 1, (int)EXPENSE_CATEGORIES.size());
        return EXPENSE_CATEGORIES[c - 1];
    }

    void addExpense() {
        cout << "\n--- Add Expense ---\n";
        double amt = io::readAmount("Amount: " + CURRENCY);
        string cat = pickCategory();
        string note = io::readLine("Note (optional): ");
        Date d = io::readDate("Date");
        tracker_.addExpense(d, amt, cat, note);
        autosave();
        cout << "Saved.\n";

        Budget &b = tracker_.budget();
        if (b.isSet()) {
            double spent = tracker_.spentInMonth(d.month(), d.year());
            if (spent > b.limit()) cout << "!! Over your monthly budget by " << fixed << setprecision(2)
                                        << CURRENCY << spent - b.limit() << '\n';
            else if (spent >= 0.8 * b.limit()) cout << "! You have used " << fixed << setprecision(0)
                                                     << b.usedPercent(spent) << "% of this month's budget.\n";
        }
    }

    void addIncome() {
        cout << "\n--- Add Money Received ---\n";
        double amt = io::readAmount("Amount: " + CURRENCY);
        string src = io::readLine("Source (e.g. Pocket money, Scholarship, Part-time): ");
        if (src.empty()) src = "Pocket money";
        replace(src.begin(), src.end(), ',', ';');
        string note = io::readLine("Note (optional): ");
        Date d = io::readDate("Date");
        tracker_.addIncome(d, amt, src, note);
        autosave();
        cout << "Saved.\n";
    }

    void deleteTransaction() {
        if (tracker_.empty()) { cout << "\nNothing to delete.\n"; return; }
        tracker_.showAll();
        int id = io::readInt("\nID to delete (0 to cancel): ", 0, 1000000);
        if (id == 0) return;
        if (!tracker_.find(id)) { cout << "No transaction with ID " << id << ".\n"; return; }
        if (io::confirm("Delete transaction " + to_string(id) + "?")) {
            tracker_.remove(id);
            autosave();
            cout << "Deleted.\n";
        }
    }

    void monthlySummary() {
        Date today;
        while (true) {
            string s = io::readLine("Month to view (mm-yyyy, Enter = this month): ");
            if (s.empty()) { tracker_.showSummary(today.month(), today.year()); return; }
            Date d;
            if (Date::parse("01-" + s, d)) { tracker_.showSummary(d.month(), d.year()); return; }
            cout << "  Example: 11-2026\n";
        }
    }

    void setBudget() {
        cout << "\nCurrent budget: ";
        if (tracker_.budget().isSet()) cout << CURRENCY << fixed << setprecision(2) << tracker_.budget().limit();
        else cout << "not set";
        cout << '\n';
        tracker_.budget().set(io::readAmount("New monthly budget: " + CURRENCY));
        autosave();
        cout << "Budget updated.\n";
    }

    void printMenu() const {
        cout << "\n=========== EXPENSE TRACKER ===========\n"
             << " 1. Add expense\n"
             << " 2. Add money received\n"
             << " 3. View all transactions\n"
             << " 4. View expenses by category\n"
             << " 5. Monthly summary\n"
             << " 6. Set monthly budget\n"
             << " 7. Overall balance\n"
             << " 8. Delete a transaction\n"
             << " 0. Exit\n"
             << "=======================================\n";
    }

public:
    void run() {
        int bad = tracker_.load(DATA_FILE);
        cout << "Welcome to your Expense Tracker!\n";
        if (bad) cout << "(Skipped " << bad << " unreadable line(s) in " << DATA_FILE << ")\n";

        while (true) {
            printMenu();
            int choice = io::readInt("Choose an option: ", 0, 8);
            switch (choice) {
                case 1: addExpense(); break;
                case 2: addIncome(); break;
                case 3: tracker_.showAll(); break;
                case 4: tracker_.showByCategory(pickCategory()); break;
                case 5: monthlySummary(); break;
                case 6: setBudget(); break;
                case 7: tracker_.showOverview(); break;
                case 8: deleteTransaction(); break;
                case 0:
                    autosave();
                    cout << "Data saved to " << DATA_FILE << ". Goodbye!\n";
                    return;
            }
        }
    }
};

int main() {
    App app;
    app.run();
    return 0;
}
