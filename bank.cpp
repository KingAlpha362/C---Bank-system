/*
    Standard Bank - Multi‑Branch Banking Management System
    Beginner‑style implementation with robust error handling
    Features: Interest calculation, search, reports, backup, CSV export
    Enhanced with: Luhn check, overdraft fees, fixed deposit maturity,
                   teller password change, account closure, profile editing,
                   transaction filtering, branch management, and more.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace std;

// ------------------------------------------------------------
// Helper functions for robust input (prevent crashes)
// ------------------------------------------------------------
void clear_cin() {
    cin.clear();
    cin.ignore(10000, '\n');
}

int get_int_choice(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            clear_cin();
            return value;
        } else {
            cout << "Invalid input. Please enter a number.\n";
            clear_cin();
        }
    }
}

double get_double_amount(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value > 0) {
            clear_cin();
            return value;
        } else {
            cout << "Invalid amount. Please enter a positive number.\n";
            clear_cin();
        }
    }
}

string get_string_input(const string& prompt) {
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

// ------------------------------------------------------------
// Data structures (fixed‑size char arrays for binary files)
// ------------------------------------------------------------
struct TellerRecord {
    char id[10];
    char name[50];
    size_t password_hash;      // simple hash of password + salt
    char branch_code[10];
    bool is_active;            // new: can deactivate teller
};

struct BranchRecord {
    char code[10];
    char name[50];
    char address[100];
    double total_balance;
    int customer_count;
    bool is_active;            // new: branch active flag
};

struct CustomerRecord {
    char account_number[30];
    char full_name[100];
    char sa_id[14];
    char contact[11];
    char email[100];
    char address[200];
    char dob[11];
    int account_type;          // 1=Savings, 2=Cheque, 3=Fixed, 4=Student
    double balance;
    char branch_code[10];
    char encrypted_pin[10];    // XOR‑encrypted PIN
    int failed_attempts;
    bool locked;
    // New fields for enhanced features
    char last_interest_date[11];   // DD/MM/YYYY
    char fixed_maturity_date[11];  // for fixed deposits
    double overdraft_limit;        // for cheque accounts
    double monthly_fee;            // for cheque accounts
    double transaction_limit;      // for student accounts
};

struct TransactionRecord {
    char account_number[30];
    char type[20];
    double amount;
    double new_balance;
    char date[30];
    char branch_code[10];      // where transaction occurred
};

struct SystemLogRecord {
    char timestamp[30];
    char actor[50];            // teller ID or "SYSTEM"
    char action[100];
};

// ------------------------------------------------------------
// Simple encryption (XOR)
// ------------------------------------------------------------
string simple_encrypt(string input) {
    for (char &c : input) c = c ^ 0x55;
    return input;
}

// ------------------------------------------------------------
// Validation functions (enhanced)
// ------------------------------------------------------------
// Luhn algorithm for SA ID check (first 12 digits, 13th is checksum)
bool luhn_check(const string& id) {
    if (id.length() != 13) return false;
    int sum = 0;
    bool alternate = false;
    for (int i = 11; i >= 0; i--) {
        int digit = id[i] - '0';
        if (alternate) {
            digit *= 2;
            if (digit > 9) digit = (digit % 10) + 1;
        }
        sum += digit;
        alternate = !alternate;
    }
    int checksum = (10 - (sum % 10)) % 10;
    return checksum == (id[12] - '0');
}

bool validate_said(const string& id) {
    return id.length() == 13 && all_of(id.begin(), id.end(), ::isdigit) && luhn_check(id);
}

bool validate_phone(const string& phone) {
    return phone.length() == 10 && all_of(phone.begin(), phone.end(), ::isdigit);
}

bool validate_email(const string& email) {
    return email.find('@') != string::npos && email.find('.') != string::npos;
}

bool validate_pin(const string& pin) {
    return pin.length() == 5 && all_of(pin.begin(), pin.end(), ::isdigit);
}

bool validate_date(const string& date) {
    if (date.length() != 10) return false;
    if (date[2] != '/' || date[5] != '/') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit(date[i])) return false;
    }
    return true;
}

// ------------------------------------------------------------
// Generation helpers
// ------------------------------------------------------------
string gen_acc_number(const string& branch_code) {
    static int counter = 1000;
    counter++;
    return "ACC-" + branch_code + "-" + to_string(counter);
}

string gen_pin() {
    return to_string(rand() % 90000 + 10000);
}

string get_current_date_str() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[11];
    sprintf(buf, "%02d/%02d/%04d", ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
    return string(buf);
}

// Add months to a date (simple, assumes 30-day months for demo)
string add_months(const string& date, int months) {
    int d, m, y;
    sscanf(date.c_str(), "%d/%d/%d", &d, &m, &y);
    m += months;
    while (m > 12) { m -= 12; y++; }
    char buf[11];
    sprintf(buf, "%02d/%02d/%04d", d, m, y);
    return string(buf);
}

// ------------------------------------------------------------
// System logging
// ------------------------------------------------------------
void log_system_event(const string& actor, const string& action) {
    ofstream file("system_log.dat", ios::binary | ios::app);
    if (!file) return;
    SystemLogRecord log;
    memset(&log, 0, sizeof(log));
    time_t now = time(0);
    strcpy(log.timestamp, ctime(&now));
    log.timestamp[strlen(log.timestamp)-1] = '\0';
    strcpy(log.actor, actor.c_str());
    strcpy(log.action, action.c_str());
    file.write((char*)&log, sizeof(log));
    file.close();
}

// ------------------------------------------------------------
// Branch file handling (vector used for simplicity)
// ------------------------------------------------------------
vector<BranchRecord> load_branches() {
    vector<BranchRecord> branches;
    ifstream file("branches.dat", ios::binary);
    if (file.is_open()) {
        BranchRecord b;
        while (file.read((char*)&b, sizeof(b)))
            branches.push_back(b);
        file.close();
    }
    return branches;
}

bool save_branches(vector<BranchRecord> &branches) {
    ofstream file("branches.dat", ios::binary);
    if (!file) {
        cout << "ERROR: Could not open branches.dat for writing.\n";
        return false;
    }
    for (size_t i = 0; i < branches.size(); i++)
        file.write((char*)&branches[i], sizeof(branches[i]));
    if (!file.good()) {
        cout << "WARNING: Branch data may be incomplete.\n";
        return false;
    }
    file.close();
    return true;
}

void update_branch_stats(const string& branch_code, double amount, bool new_customer) {
    vector<BranchRecord> branches = load_branches();
    for (auto &b : branches) {
        if (branch_code == b.code && b.is_active) {
            b.total_balance += amount;
            if (new_customer) b.customer_count++;
            break;
        }
    }
    save_branches(branches);
}

// New: Add a new branch
void add_branch() {
    BranchRecord b;
    memset(&b, 0, sizeof(b));
    cout << "Enter Branch Code (e.g., B004): ";
    cin.getline(b.code, 10);
    cout << "Enter Branch Name: ";
    cin.getline(b.name, 50);
    cout << "Enter Address: ";
    cin.getline(b.address, 100);
    b.total_balance = 0.0;
    b.customer_count = 0;
    b.is_active = true;
    vector<BranchRecord> branches = load_branches();
    branches.push_back(b);
    if (save_branches(branches))
        cout << "Branch added successfully.\n";
    else
        cout << "Failed to add branch.\n";
    log_system_event("ADMIN", "Added branch " + string(b.code));
}

// New: Remove (deactivate) a branch
void remove_branch() {
    string code = get_string_input("Enter branch code to remove: ");
    vector<BranchRecord> branches = load_branches();
    bool found = false;
    for (auto &b : branches) {
        if (b.code == code) {
            b.is_active = false;
            found = true;
            break;
        }
    }
    if (found) {
        save_branches(branches);
        cout << "Branch deactivated.\n";
        log_system_event("ADMIN", "Deactivated branch " + code);
    } else {
        cout << "Branch not found.\n";
    }
}

// ------------------------------------------------------------
// System initialisation
// ------------------------------------------------------------
void init_system() {
    srand((unsigned)time(0));
    // Create config file if missing
    ifstream config_in("system_config.txt");
    if (!config_in.is_open()) {
        ofstream config("system_config.txt");
        config << "MAX_LOGIN_ATTEMPTS=3\n";
        config << "SAVINGS_INTEREST=2.5\n";
        config << "FIXED_INTEREST=5.0\n";
        config << "OVERDRAFT_FEE=25.0\n";
        config << "STUDENT_TRANSACTION_LIMIT=1000.0\n";
        config.close();
    }
    config_in.close();

    // Create default branches if file missing
    ifstream branch_in("branches.dat", ios::binary);
    if (!branch_in.is_open()) {
        ofstream bfile("branches.dat", ios::binary);
        BranchRecord branches[3] = {
            {"B001", "Sandton City", "1 Sandton Drive, Johannesburg", 0.0, 0, true},
            {"B002", "Cape Town V&A", "22 Long Street, Cape Town", 0.0, 0, true},
            {"B003", "Durban Beachfront", "5 Marine Parade, Durban", 0.0, 0, true}
        };
        bfile.write((char*)branches, sizeof(branches));
        bfile.close();
        cout << "Default Standard Bank branches created.\n";
    }
    branch_in.close();
}

// ------------------------------------------------------------
// Teller functions (enhanced)
// ------------------------------------------------------------
void create_teller() {
    ofstream file("tellers.dat", ios::binary | ios::app);
    if (!file) {
        cout << "ERROR: Could not open tellers.dat for writing.\n";
        return;
    }

    TellerRecord t;
    memset(&t, 0, sizeof(t));

    cout << "Enter Teller ID: ";
    cin.getline(t.id, 10);
    cout << "Enter Full Name: ";
    cin.getline(t.name, 50);
    string pass = get_string_input("Enter Password: ");
    cout << "Enter Branch Code: ";
    cin.getline(t.branch_code, 10);
    t.is_active = true;

    // Simple hashing with salt
    string salt = "StandardBank#2024";
    hash<string> hasher;
    t.password_hash = hasher(pass + salt);

    file.write((char*)&t, sizeof(t));
    if (!file.good()) {
        cout << "ERROR: Failed to write teller record.\n";
    } else {
        cout << "Teller created successfully.\n";
        log_system_event("ADMIN", "Created teller " + string(t.id));
    }
    file.close();
}

bool teller_login(string &branch_code, string &teller_id) {
    string id, pass;
    cout << "Teller ID: ";
    cin >> id;
    pass = get_string_input("Password: ");

    string salt = "StandardBank#2024";
    hash<string> hasher;
    size_t hashed = hasher(pass + salt);

    ifstream file("tellers.dat", ios::binary);
    if (!file) {
        cout << "No tellers found. Please create a teller first.\n";
        return false;
    }

    TellerRecord t;
    while (file.read((char*)&t, sizeof(t))) {
        if (id == t.id && hashed == t.password_hash && t.is_active) {
            branch_code = t.branch_code;
            teller_id = t.id;
            cout << "Login successful! Welcome to Standard Bank, " << t.name << "\n";
            cout << "Branch: " << branch_code << "\n";
            log_system_event(teller_id, "Logged in");
            file.close();
            return true;
        }
    }
    file.close();
    cout << "Invalid ID or password, or account inactive.\n";
    return false;
}

// New: Change teller password
void change_teller_password(const string& teller_id) {
    ifstream infile("tellers.dat", ios::binary);
    ofstream temp("temp_tellers.dat", ios::binary);
    if (!infile || !temp) {
        cout << "ERROR: Could not update teller file.\n";
        return;
    }
    string old_pass = get_string_input("Enter current password: ");
    string new_pass = get_string_input("Enter new password: ");
    string salt = "StandardBank#2024";
    hash<string> hasher;
    size_t old_hash = hasher(old_pass + salt);
    size_t new_hash = hasher(new_pass + salt);

    TellerRecord t;
    bool updated = false;
    while (infile.read((char*)&t, sizeof(t))) {
        if (string(t.id) == teller_id) {
            if (t.password_hash == old_hash) {
                t.password_hash = new_hash;
                updated = true;
                cout << "Password changed successfully.\n";
                log_system_event(teller_id, "Changed password");
            } else {
                cout << "Current password incorrect.\n";
            }
        }
        temp.write((char*)&t, sizeof(t));
    }
    infile.close();
    temp.close();
    remove("tellers.dat");
    rename("temp_tellers.dat", "tellers.dat");
    if (!updated && old_hash != 0) cout << "Password not changed.\n";
}

// ------------------------------------------------------------
// Customer file helpers (enhanced)
// ------------------------------------------------------------
bool find_customer(const string& acc_num, CustomerRecord &record) {
    ifstream file("customers.dat", ios::binary);
    if (!file) return false;
    while (file.read((char*)&record, sizeof(record))) {
        if (acc_num == record.account_number) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool update_customer(CustomerRecord &record) {
    ifstream infile("customers.dat", ios::binary);
    ofstream temp("temp.dat", ios::binary);
    if (!infile || !temp) {
        cout << "ERROR: Could not update customer file.\n";
        return false;
    }

    CustomerRecord r;
    while (infile.read((char*)&r, sizeof(r))) {
        if (string(r.account_number) == string(record.account_number))
            temp.write((char*)&record, sizeof(record));
        else
            temp.write((char*)&r, sizeof(r));
    }
    infile.close();
    temp.close();
    remove("customers.dat");
    rename("temp.dat", "customers.dat");
    return true;
}

bool verify_customer_pin(const string& acc_num, const string& entered_pin) {
    CustomerRecord rec;
    if (!find_customer(acc_num, rec)) return false;
    if (rec.locked) {
        cout << "Account is LOCKED. Contact branch manager.\n";
        return false;
    }
    string decrypted = simple_encrypt(rec.encrypted_pin);
    if (entered_pin == decrypted) {
        rec.failed_attempts = 0;
        update_customer(rec);
        return true;
    } else {
        rec.failed_attempts++;
        if (rec.failed_attempts >= 3) {
            rec.locked = true;
            cout << "Too many failed attempts. Account locked.\n";
            log_system_event("SYSTEM", "Account locked: " + acc_num);
        }
        update_customer(rec);
        return false;
    }
}

void register_customer(const string& branch_code, const string& teller_id) {
    CustomerRecord c;
    memset(&c, 0, sizeof(c));

    int type = get_int_choice("Select Account Type:\n1. Savings\n2. Cheque\n3. Fixed Deposit\n4. Student\nChoice: ");
    if (type < 1 || type > 4) {
        cout << "Invalid account type. Registration aborted.\n";
        return;
    }
    c.account_type = type;

    cin.ignore(); // clear newline before getline
    string name = get_string_input("Full Name: ");
    strcpy(c.full_name, name.c_str());

    string said;
    do {
        said = get_string_input("SA ID (13 digits, checksum validated): ");
    } while (!validate_said(said));
    strcpy(c.sa_id, said.c_str());

    string phone;
    do {
        phone = get_string_input("Contact Number (10 digits): ");
    } while (!validate_phone(phone));
    strcpy(c.contact, phone.c_str());

    string email;
    do {
        email = get_string_input("Email: ");
    } while (!validate_email(email));
    strcpy(c.email, email.c_str());

    string addr = get_string_input("Physical Address: ");
    strcpy(c.address, addr.c_str());

    string dob;
    do {
        dob = get_string_input("Date of Birth (DD/MM/YYYY): ");
    } while (!validate_date(dob));
    strcpy(c.dob, dob.c_str());

    strcpy(c.branch_code, branch_code.c_str());
    string acc_num = gen_acc_number(branch_code);
    strcpy(c.account_number, acc_num.c_str());

    // Set account-specific fields
    double min_dep = 0.0;
    if (type == 1) { // Savings
        min_dep = 100.0;
        strcpy(c.last_interest_date, get_current_date_str().c_str());
    } else if (type == 2) { // Cheque
        min_dep = 500.0;
        // Load overdraft fee from config
        ifstream config("system_config.txt");
        string line;
        while (getline(config, line)) {
            if (line.find("OVERDRAFT_FEE=") == 0)
                c.monthly_fee = stod(line.substr(14));
        }
        config.close();
        c.overdraft_limit = 500.0; // default
    } else if (type == 3) { // Fixed Deposit
        min_dep = 1000.0;
        string maturity = add_months(get_current_date_str(), 12);
        strcpy(c.fixed_maturity_date, maturity.c_str());
        strcpy(c.last_interest_date, get_current_date_str().c_str());
    } else if (type == 4) { // Student
        min_dep = 50.0;
        ifstream config("system_config.txt");
        string line;
        while (getline(config, line)) {
            if (line.find("STUDENT_TRANSACTION_LIMIT=") == 0)
                c.transaction_limit = stod(line.substr(25));
        }
        config.close();
    }

    double init_dep;
    do {
        init_dep = get_double_amount("Initial deposit (min R" + to_string(min_dep) + "): R");
    } while (init_dep < min_dep);
    c.balance = init_dep;

    string pin = gen_pin();
    string enc_pin = simple_encrypt(pin);
    strcpy(c.encrypted_pin, enc_pin.c_str());
    c.failed_attempts = 0;
    c.locked = false;

    ofstream file("customers.dat", ios::binary | ios::app);
    if (!file) {
        cout << "ERROR: Could not open customers.dat. Registration failed.\n";
        return;
    }
    file.write((char*)&c, sizeof(c));
    file.close();

    update_branch_stats(branch_code, c.balance, true);
    log_system_event(teller_id, "Registered customer " + acc_num);

    cout << "\n=== STANDARD BANK ACCOUNT CREATED ===\n";
    cout << "Account Number: " << c.account_number << "\n";
    cout << "Generated PIN: " << pin << " (keep this safe!)\n";
    if (type == 3)
        cout << "Fixed Deposit matures on: " << c.fixed_maturity_date << "\n";
}

// New: Edit customer profile (teller only)
void edit_customer_profile(const string& acc_num, const string& teller_id) {
    CustomerRecord c;
    if (!find_customer(acc_num, c)) {
        cout << "Customer not found.\n";
        return;
    }
    cout << "Editing profile for " << c.full_name << " (" << acc_num << ")\n";
    cout << "Leave blank to keep current value.\n";
    cin.ignore();
    string input = get_string_input("New Full Name [" + string(c.full_name) + "]: ");
    if (!input.empty()) strcpy(c.full_name, input.c_str());
    input = get_string_input("New Contact [" + string(c.contact) + "]: ");
    if (!input.empty() && validate_phone(input)) strcpy(c.contact, input.c_str());
    input = get_string_input("New Email [" + string(c.email) + "]: ");
    if (!input.empty() && validate_email(input)) strcpy(c.email, input.c_str());
    input = get_string_input("New Address [" + string(c.address) + "]: ");
    if (!input.empty()) strcpy(c.address, input.c_str());
    if (update_customer(c)) {
        cout << "Profile updated.\n";
        log_system_event(teller_id, "Edited profile of " + acc_num);
    } else {
        cout << "Update failed.\n";
    }
}

// New: Close account (teller/admin)
void close_account(const string& acc_num, const string& teller_id) {
    CustomerRecord c;
    if (!find_customer(acc_num, c)) {
        cout << "Account not found.\n";
        return;
    }
    if (c.balance > 0) {
        cout << "Account has a balance of R" << c.balance << ". Withdraw or transfer funds first.\n";
        return;
    }
    // Simple deactivation: set balance to -1 to mark closed (or remove from file)
    // Here we'll physically remove by rewriting file without this record
    ifstream infile("customers.dat", ios::binary);
    ofstream temp("temp.dat", ios::binary);
    if (!infile || !temp) {
        cout << "ERROR: Could not close account.\n";
        return;
    }
    CustomerRecord r;
    bool removed = false;
    while (infile.read((char*)&r, sizeof(r))) {
        if (string(r.account_number) == acc_num) {
            removed = true;
            // update branch stats (reverse)
            update_branch_stats(r.branch_code, -r.balance, true); // decrease customer count
        } else {
            temp.write((char*)&r, sizeof(r));
        }
    }
    infile.close();
    temp.close();
    remove("customers.dat");
    rename("temp.dat", "customers.dat");
    if (removed) {
        cout << "Account closed successfully.\n";
        log_system_event(teller_id, "Closed account " + acc_num);
    } else {
        cout << "Account not found.\n";
    }
}

// ------------------------------------------------------------
// Transaction logging
// ------------------------------------------------------------
void log_transaction(const string& acc_num, const string& type, double amount, double balance, const string& branch_code) {
    ofstream file("transactions.dat", ios::binary | ios::app);
    if (!file) {
        cout << "WARNING: Could not log transaction.\n";
        return;
    }
    TransactionRecord t;
    memset(&t, 0, sizeof(t));
    strcpy(t.account_number, acc_num.c_str());
    strcpy(t.type, type.c_str());
    t.amount = amount;
    t.new_balance = balance;
    strcpy(t.branch_code, branch_code.c_str());
    time_t now = time(0);
    strcpy(t.date, ctime(&now));
    t.date[strlen(t.date) - 1] = '\0';  // remove newline
    file.write((char*)&t, sizeof(t));
    file.close();
}

// ------------------------------------------------------------
// Transaction functions (enhanced with account-specific rules)
// ------------------------------------------------------------
void deposit(CustomerRecord &c, const string& branch_code) {
    double amt = get_double_amount("Deposit amount: R");
    c.balance += amt;
    if (update_customer(c)) {
        log_transaction(c.account_number, "DEPOSIT", amt, c.balance, branch_code);
        update_branch_stats(c.branch_code, amt, false);
        cout << "New balance: R" << c.balance << "\n";
    } else {
        cout << "Deposit failed due to system error.\n";
    }
}

void withdraw(CustomerRecord &c, const string& branch_code) {
    double amt = get_double_amount("Withdrawal amount: R");
    // Check account-specific restrictions
    if (c.account_type == 3) { // Fixed deposit
        string today = get_current_date_str();
        if (today < string(c.fixed_maturity_date)) {
            cout << "Early withdrawal penalty: 1% fee applies. Proceed? (y/n): ";
            char ch; cin >> ch; clear_cin();
            if (tolower(ch) != 'y') return;
            double penalty = amt * 0.01;
            amt += penalty;
            cout << "Total debit including penalty: R" << amt << "\n";
        }
    }
    if (c.account_type == 4) { // Student
        if (amt > c.transaction_limit) {
            cout << "Exceeds daily transaction limit of R" << c.transaction_limit << "\n";
            return;
        }
    }
    // Overdraft for cheque
    double available = c.balance;
    if (c.account_type == 2) available += c.overdraft_limit;
    if (amt > available) {
        cout << "Insufficient funds (including overdraft).\n";
        return;
    }
    c.balance -= amt;
    if (update_customer(c)) {
        log_transaction(c.account_number, "WITHDRAWAL", amt, c.balance, branch_code);
        update_branch_stats(c.branch_code, -amt, false);
        cout << "New balance: R" << c.balance << "\n";
        if (c.account_type == 2 && c.balance < 0) {
            cout << "Overdraft used. Monthly fee of R" << c.monthly_fee << " will apply.\n";
        }
    } else {
        cout << "Withdrawal failed.\n";
    }
}

void transfer(CustomerRecord &from, const string& branch_code) {
    string to_acc = get_string_input("Recipient account number: ");
    CustomerRecord to;
    if (!find_customer(to_acc, to)) {
        cout << "Recipient account not found.\n";
        return;
    }
    double amt = get_double_amount("Amount to transfer: R");
    // Similar restrictions as withdrawal
    double available = from.balance;
    if (from.account_type == 2) available += from.overdraft_limit;
    if (amt > available) {
        cout << "Insufficient funds.\n";
        return;
    }
    if (from.account_type == 4 && amt > from.transaction_limit) {
        cout << "Exceeds student transaction limit.\n";
        return;
    }
    from.balance -= amt;
    to.balance += amt;
    if (update_customer(from) && update_customer(to)) {
        log_transaction(from.account_number, "TRANSFER OUT", amt, from.balance, branch_code);
        log_transaction(to.account_number, "TRANSFER IN", amt, to.balance, to.branch_code);
        update_branch_stats(from.branch_code, -amt, false);
        update_branch_stats(to.branch_code, amt, false);
        cout << "Transfer successful.\n";
    } else {
        cout << "Transfer failed.\n";
    }
}

void view_statement(CustomerRecord &c) {
    cout << "\n=== STANDARD BANK STATEMENT: " << c.account_number << " ===\n";
    cout << "Name: " << c.full_name << "  Balance: R" << c.balance << "\n";
    if (c.account_type == 2) cout << "Overdraft Limit: R" << c.overdraft_limit << "\n";
    if (c.account_type == 3) cout << "Maturity Date: " << c.fixed_maturity_date << "\n";
    cout << "\nRecent Transactions:\n";
    ifstream file("transactions.dat", ios::binary);
    if (!file) {
        cout << "No transaction history available.\n";
        return;
    }
    TransactionRecord t;
    int count = 0;
    while (file.read((char*)&t, sizeof(t)) && count < 10) {
        if (string(t.account_number) == string(c.account_number)) {
            cout << t.date << " | " << t.type << " | R" << t.amount
                 << " | Balance: R" << t.new_balance << "\n";
            count++;
        }
    }
    file.close();
    if (count == 0) cout << "No transactions yet.\n";
}

void change_pin(CustomerRecord &c) {
    string old_pin = get_string_input("Current PIN: ");
    string decrypted = simple_encrypt(c.encrypted_pin);
    if (old_pin != decrypted) {
        cout << "Incorrect PIN.\n";
        return;
    }
    string new_pin;
    do {
        new_pin = get_string_input("New PIN (5 digits): ");
    } while (!validate_pin(new_pin));
    string enc_new = simple_encrypt(new_pin);
    strcpy(c.encrypted_pin, enc_new.c_str());
    if (update_customer(c))
        cout << "PIN changed successfully.\n";
    else
        cout << "Failed to update PIN.\n";
}

// ------------------------------------------------------------
// Advanced Features (Enhanced)
// ------------------------------------------------------------
void apply_interest() {
    ifstream infile("customers.dat", ios::binary);
    if (!infile) {
        cout << "No customer data found.\n";
        return;
    }
    ofstream temp("temp.dat", ios::binary);
    if (!temp) {
        cout << "ERROR: Could not create temporary file.\n";
        return;
    }

    double sav_rate = 2.5, fix_rate = 5.0;
    ifstream config("system_config.txt");
    string line;
    while (getline(config, line)) {
        if (line.find("SAVINGS_INTEREST=") == 0)
            sav_rate = stod(line.substr(17));
        else if (line.find("FIXED_INTEREST=") == 0)
            fix_rate = stod(line.substr(15));
    }
    config.close();

    CustomerRecord c;
    int count = 0;
    string today = get_current_date_str();
    while (infile.read((char*)&c, sizeof(c))) {
        bool applied = false;
        if (c.account_type == 1 && string(c.last_interest_date) < today) {
            double interest = c.balance * (sav_rate / 100.0);
            c.balance += interest;
            strcpy(c.last_interest_date, today.c_str());
            applied = true;
            count++;
            cout << "Added R" << interest << " interest to Savings " << c.account_number << "\n";
        } else if (c.account_type == 3 && string(c.last_interest_date) < today) {
            double interest = c.balance * (fix_rate / 100.0);
            c.balance += interest;
            strcpy(c.last_interest_date, today.c_str());
            applied = true;
            count++;
            cout << "Added R" << interest << " interest to Fixed Deposit " << c.account_number << "\n";
        }
        temp.write((char*)&c, sizeof(c));
    }
    infile.close();
    temp.close();

    if (remove("customers.dat") != 0 || rename("temp.dat", "customers.dat") != 0) {
        cout << "ERROR: Failed to update customer file.\n";
    } else {
        cout << "Interest applied to " << count << " accounts.\n";
        log_system_event("SYSTEM", "Applied interest to " + to_string(count) + " accounts");
    }
}

void search_customer() {
    string term = get_string_input("Search (account number, name, ID, or phone): ");
    ifstream file("customers.dat", ios::binary);
    if (!file) {
        cout << "No customer data found.\n";
        return;
    }
    CustomerRecord c;

    bool found = false;

    while (file.read((char*)&c, sizeof(c))) {
        
        string acc(c.account_number), name(c.full_name), id(c.sa_id), phone(c.contact);
        string term_lower = term;
        transform(term_lower.begin(), term_lower.end(), term_lower.begin(), ::tolower);
        string acc_lower = acc, name_lower = name, id_lower = id, phone_lower = phone;
        transform(acc_lower.begin(), acc_lower.end(), acc_lower.begin(), ::tolower);
        transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
        transform(id_lower.begin(), id_lower.end(), id_lower.begin(), ::tolower);
        transform(phone_lower.begin(), phone_lower.end(), phone_lower.begin(), ::tolower);
        if (acc_lower.find(term_lower) != string::npos ||
            name_lower.find(term_lower) != string::npos ||
            id_lower.find(term_lower) != string::npos ||
            phone_lower.find(term_lower) != string::npos) {
            cout << "Account: " << c.account_number << " | " << c.full_name
                 << " | R" << c.balance << " | Branch " << c.branch_code << "\n";
            found = true;
        }
    }
    file.close();
    if (!found) cout << "No matching customers.\n";
}

void daily_transaction_report() {
    ifstream file("transactions.dat", ios::binary);
    if (!file) {
        cout << "No transactions found.\n";
        return;
    }
    TransactionRecord t;
    time_t now = time(0);
    string today = ctime(&now);
    today = today.substr(0, 10);  // "Mmm dd yyyy"
    int count = 0;
    double total = 0.0;
    cout << "\n=== Standard Bank Daily Transactions (" << today << ") ===\n";
    while (file.read((char*)&t, sizeof(t))) {
        string tdate(t.date);
        if (tdate.find(today) != string::npos) {
            cout << t.date << " | " << t.account_number << " | " << t.type
                 << " | R" << t.amount << " | Branch " << t.branch_code << "\n";
            count++;
            total += t.amount;
        }
    }
    file.close();
    cout << "Total: " << count << " transactions, Volume: R" << total << "\n";
}

void customer_summary_report() {
    ifstream file("customers.dat", ios::binary);
    if (!file) {
        cout << "No customer data found.\n";
        return;
    }
    CustomerRecord c;
    int count = 0;
    double total = 0.0;
    cout << "\n=== Standard Bank Customer Summary ===\n";
    while (file.read((char*)&c, sizeof(c))) {
        string type = (c.account_type == 1) ? "Savings" : (c.account_type == 2) ? "Cheque" :
                      (c.account_type == 3) ? "FixedDep" : "Student";
        cout << c.account_number << " | " << c.full_name << " | " << type
             << " | R" << c.balance << " | Branch " << c.branch_code << "\n";
        count++;
        total += c.balance;
    }
    file.close();
    cout << "\nTotal customers: " << count << "\nTotal bank balance: R" << total << "\n";
}

void branch_performance_report() {
    vector<BranchRecord> branches = load_branches();
    cout << "\n=== Standard Bank Branch Performance ===\n";
    for (auto &b : branches) {
        if (!b.is_active) continue;
        cout << b.name << " (" << b.code << ")\n";
        cout << "  Total deposits: R" << b.total_balance << "\n";
        cout << "  Customers: " << b.customer_count << "\n";
        if (b.customer_count > 0)
            cout << "  Avg per customer: R" << b.total_balance / b.customer_count << "\n";
        cout << "-----\n";
    }
}

// New: View system logs
void view_system_logs() {
    ifstream file("system_log.dat", ios::binary);
    if (!file) {
        cout << "No logs available.\n";
        return;
    }
    SystemLogRecord log;
    cout << "\n=== System Activity Log ===\n";
    while (file.read((char*)&log, sizeof(log))) {
        cout << log.timestamp << " | " << log.actor << " | " << log.action << "\n";
    }
    file.close();
}

// Portable backup
bool copy_file(const string& src, const string& dst) {
    ifstream in(src, ios::binary);
    if (!in) return false;
    ofstream out(dst, ios::binary);
    if (!out) return false;
    out << in.rdbuf();
    return out.good();
}

void backup_data() {
    bool ok = true;
    ok &= copy_file("customers.dat", "customers_backup.dat");
    ok &= copy_file("transactions.dat", "transactions_backup.dat");
    ok &= copy_file("branches.dat", "branches_backup.dat");
    ok &= copy_file("tellers.dat", "tellers_backup.dat");
    ok &= copy_file("system_log.dat", "system_log_backup.dat");
    if (ok)
        cout << "Backup successful.\n";
    else
        cout << "Backup failed (some files may be missing).\n";
    log_system_event("SYSTEM", "Backup performed");
}

void recover_data() {
    bool ok = true;
    ok &= copy_file("customers_backup.dat", "customers.dat");
    ok &= copy_file("transactions_backup.dat", "transactions.dat");
    ok &= copy_file("branches_backup.dat", "branches.dat");
    ok &= copy_file("tellers_backup.dat", "tellers.dat");
    ok &= copy_file("system_log_backup.dat", "system_log.dat");
    if (ok)
        cout << "Recovery successful.\n";
    else
        cout << "Recovery failed (backup files missing).\n";
    log_system_event("SYSTEM", "Data recovery performed");
}

void export_to_csv() {
    ofstream csv("standard_bank_export.csv");
    if (!csv) {
        cout << "ERROR: Could not create CSV file.\n";
        return;
    }
    csv << "Account Number,Full Name,SA ID,Contact,Email,Address,DOB,Type,Balance,Branch,Maturity\n";
    ifstream file("customers.dat", ios::binary);
    if (!file) {
        cout << "No customer data to export.\n";
        return;
    }
    CustomerRecord c;
    while (file.read((char*)&c, sizeof(c))) {
        string type = (c.account_type == 1) ? "Savings" :
                      (c.account_type == 2) ? "Cheque" :
                      (c.account_type == 3) ? "Fixed Deposit" : "Student";
        csv << c.account_number << "," << c.full_name << "," << c.sa_id << ","
            << c.contact << "," << c.email << "," << c.address << ","
            << c.dob << "," << type << "," << c.balance << "," << c.branch_code << ","
            << (c.account_type == 3 ? c.fixed_maturity_date : "") << "\n";
    }
    file.close();
    csv.close();
    cout << "Data exported to standard_bank_export.csv\n";
    log_system_event("SYSTEM", "CSV export performed");
}

// ------------------------------------------------------------
// Menus (Enhanced)
// ------------------------------------------------------------
void customer_menu(CustomerRecord &c) {
    int choice;
    do {
        cout << "\n=== STANDARD BANK CUSTOMER PORTAL ===\n";
        cout << "Welcome, " << c.full_name << "\n";
        cout << "1. View Balance & Details\n2. Deposit\n3. Withdraw\n4. Transfer\n";
        cout << "5. Statement\n6. Change PIN\n7. Logout\n";
        choice = get_int_choice("Choice: ");
        if (choice == 1) {
            cout << "Balance: R" << c.balance << "\n";
            if (c.account_type == 2) cout << "Overdraft Limit: R" << c.overdraft_limit << "\n";
            if (c.account_type == 3) cout << "Matures: " << c.fixed_maturity_date << "\n";
        } else if (choice == 2) {
            deposit(c, c.branch_code);
        } else if (choice == 3) {
            withdraw(c, c.branch_code);
        } else if (choice == 4) {
            transfer(c, c.branch_code);
        } else if (choice == 5) {
            view_statement(c);
        } else if (choice == 6) {
            change_pin(c);
        }
    } while (choice != 7);
}

void teller_menu(const string& branch_code, const string& teller_id) {
    int choice;
    do {
        cout << "\n=== STANDARD BANK TELLER MENU (Branch " << branch_code << ") ===\n";
        cout << "1. Register Customer\n2. Assisted Transaction\n3. Search Customer\n";
        cout << "4. Edit Customer Profile\n5. Close Account\n";
        cout << "6. View All Branches\n7. Reports\n8. Change Password\n9. Logout\n";

        choice = get_int_choice("Choice: ");

        if (choice == 1) {

            register_customer(branch_code, teller_id);

        } else if (choice == 2) {

            string acc = get_string_input("Customer account: ");

            string pin = get_string_input("Customer PIN: ");

            if (verify_customer_pin(acc, pin)) {

                CustomerRecord c;

                if (find_customer(acc, c)) {

                    cout << "Customer: " << c.full_name << "\nBalance: R" << c.balance << "\n";
                    int t = get_int_choice("1. Deposit  2. Withdraw\nChoice: ");
                    if (t == 1) deposit(c, branch_code);
                    else if (t == 2) withdraw(c, branch_code);

                }
            } else {
                cout << "Verification failed.\n";
            }
        } else if (choice == 3) {

            search_customer();

        } else if (choice == 4) {

            string acc = get_string_input("Enter account number: ");

            edit_customer_profile(acc, teller_id);
            
        } else if (choice == 5) {

            string acc = get_string_input("Enter account number to close: ");

            close_account(acc, teller_id);

        } else if (choice == 6) {

            vector<BranchRecord> br = load_branches();

            for (auto &b : br) {

                if (b.is_active)

                    cout << b.code << " " << b.name << " " << b.address << "\n";
            }
        } else if (choice == 7) {
            int r = get_int_choice("1. Daily Transactions  2. Customer Summary  3. Branch Performance\nChoice: ");
            if (r == 1) daily_transaction_report();
            else if (r == 2) customer_summary_report();
            else if (r == 3) branch_performance_report();
        } else if (choice == 8) {
            change_teller_password(teller_id);
        }
    } while (choice != 9);
    log_system_event(teller_id, "Logged out");
}

// Admin menu (for branch management, etc.)
void admin_menu() {
    int choice;
    do {
        cout << "\n=== STANDARD BANK ADMINISTRATOR MENU ===\n";
        cout << "1. Add Branch\n2. Remove Branch\n3. View System Logs\n";
        cout << "4. Apply Interest\n5. Backup Data\n6. Recover Data\n7. Export CSV\n8. Exit Admin\n";
        choice = get_int_choice("Choice: ");
        if (choice == 1) {
            add_branch();
        } else if (choice == 2) {
            remove_branch();
        } else if (choice == 3) {
            view_system_logs();
        } else if (choice == 4) {
            apply_interest();
        } else if (choice == 5) {
            backup_data();
        } else if (choice == 6) {
            recover_data();
        } else if (choice == 7) {
            export_to_csv();
        }
    } while (choice != 8);
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main() {
    init_system();
    cout << "\n*** WELCOME TO STANDARD BANK MULTI-BRANCH SYSTEM ***\n";
    int choice;
    do {
        cout << "\n=== MAIN MENU ===\n";
        cout << "1. Create Teller\n2. Teller Login\n3. Customer Login\n";
        cout << "4. View All Branches\n5. Inter‑Branch Comparison\n";
        cout << "6. Apply Interest\n7. Search Customer\n8. Daily Transaction Report\n";
        cout << "9. Customer Summary\n10. Branch Performance\n";
        cout << "11. Backup Data\n12. Recover Data\n13. Export CSV\n";
        cout << "14. Admin Menu\n15. Exit\n";
        choice = get_int_choice("Choice: ");

        if (choice == 1) {
            create_teller();
        } else if (choice == 2) {
            string br, tid;
            if (teller_login(br, tid)) teller_menu(br, tid);
        } else if (choice == 3) {
            string acc = get_string_input("Account Number: ");
            string pin = get_string_input("PIN: ");
            if (verify_customer_pin(acc, pin)) {
                CustomerRecord c;
                if (find_customer(acc, c)) {
                    cout << "Welcome, " << c.full_name << "!\n";
                    customer_menu(c);
                }
            } else {
                cout << "Login failed.\n";
            }
        } else if (choice == 4) {
            vector<BranchRecord> br = load_branches();
            for (auto &b : br) if (b.is_active) cout << b.code << " " << b.name << "\n";
        } else if (choice == 5) {
            vector<BranchRecord> br = load_branches();
            if (br.empty()) { cout << "No branches.\n"; continue; }
            int best = 0;
            for (size_t i = 1; i < br.size(); i++)
                if (br[i].is_active && br[i].total_balance > br[best].total_balance) best = i;
            cout << "Highest deposits: " << br[best].name << " (R" << br[best].total_balance << ")\n";
        } else if (choice == 6) {
            apply_interest();
        } else if (choice == 7) {
            search_customer();
        } else if (choice == 8) {
            daily_transaction_report();
        } else if (choice == 9) {
            customer_summary_report();
        } else if (choice == 10) {
            branch_performance_report();
        } else if (choice == 11) {
            backup_data();
        } else if (choice == 12) {
            recover_data();
        } else if (choice == 13) {
            export_to_csv();
        } else if (choice == 14) {
            admin_menu();
        }
    } while (choice != 15);
    cout << "Thank you for banking with Standard Bank. Goodbye!\n";
    return 0;
}