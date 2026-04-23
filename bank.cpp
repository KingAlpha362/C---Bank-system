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
#include <limits>

using namespace std;

struct TellerRecord {
    char id[10];
    char name[50];
    size_t password_hash;
    char branch_code[10];
    bool is_active;
};

struct BranchRecord {
    char code[10];
    char name[50];
    char address[100];
    double total_balance;
    int customer_count;
    bool is_active;
};

struct CustomerRecord {
    char account_number[30];
    char full_name[100];
    char sa_id[14];
    char contact[11];
    char email[100];
    char address[200];
    char dob[11];
    int account_type;
    double balance;
    char branch_code[10];
    char encrypted_pin[10];
    int failed_attempts;
    bool locked;
    time_t lock_until;
    char last_interest_date[11];
    char fixed_maturity_date[11];
    double overdraft_limit;
    double monthly_fee;
    double transaction_limit;
};

struct TransactionRecord {
    char account_number[30];
    char type[20];
    double amount;
    double new_balance;
    char date[30];
    char branch_code[10];
};

struct SystemLogRecord {
    char timestamp[30];
    char actor[50];
    char action[100];
};

void clear_cin() {
    cin.clear();
    cin.ignore(10000, '\n');
}

string get_line(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

int get_int(const string& prompt) {
    int x;
    while (true) {
        cout << prompt;
        if (cin >> x) {
            clear_cin();
            return x;
        }
        cout << "Invalid number, try again.\n";
        clear_cin();
    }
}

double get_double(const string& prompt) {
    double x;
    while (true) {
        cout << prompt;
        if (cin >> x && x > 0) {
            clear_cin();
            return x;
        }
        cout << "Invalid amount, enter a positive number.\n";
        clear_cin();
    }
}

string lower_str(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)tolower(c); });
    return s;
}

bool all_digits(const string& s) {
    return !s.empty() && all_of(s.begin(), s.end(), [](unsigned char c){ return isdigit(c); });
}

string simple_encrypt(string input) {
    for (char &c : input) c = char(c ^ 0x55);
    return input;
}

bool validate_phone(const string& phone) {
    return phone.size() == 10 && all_digits(phone);
}

bool validate_pin(const string& pin) {
    return pin.size() == 5 && all_digits(pin);
}

bool validate_date_format(const string& d) {
    if (d.size() != 10) return false;
    if (d[2] != '/' || d[5] != '/') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)d[i])) return false;
    }
    return true;
}

bool validate_email(const string& email) {
    size_t at = email.find('@');
    size_t dot = email.rfind('.');
    return at != string::npos && dot != string::npos && at < dot;
}

bool validate_said(const string& id) {
    if (id.size() != 13 || !all_digits(id)) return false;
    int sum = 0;
    bool alt = true;
    for (int i = 11; i >= 0; i--) {
        int digit = id[i] - '0';
        if (alt) {
            digit *= 2;
            if (digit > 9) digit = (digit % 10) + 1;
        }
        sum += digit;
        alt = !alt;
    }
    int check = (10 - (sum % 10)) % 10;
    return check == (id[12] - '0');
}

int date_key(const string& d) {
    int day = 0, mon = 0, year = 0;
    if (sscanf(d.c_str(), "%d/%d/%d", &day, &mon, &year) != 3) return 0;
    return year * 10000 + mon * 100 + day;
}

string today_date() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buf[40];
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d", ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
    return string(buf);
}

string current_timestamp() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buf[40];
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d %02d:%02d:%02d",
             ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year,
             ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return string(buf);
}

string add_months(const string& d, int months) {
    int day = 1, mon = 1, year = 2000;
    sscanf(d.c_str(), "%d/%d/%d", &day, &mon, &year);
    mon += months;
    while (mon > 12) {
        mon -= 12;
        year++;
    }
    char buf[11];
    sprintf(buf, "%02d/%02d/%04d", day, mon, year);
    return string(buf);
}

string gen_acc_number(const string& branch_code) {
    ifstream file("customers.dat", ios::binary);
    int count = 0;
    if (file) {
        CustomerRecord c;
        while (file.read((char*)&c, sizeof(c))) count++;
        file.close();
    }
    int num = 10000 + count + 1;
    return "ACC-" + branch_code + "-" + to_string(num);
}

string gen_pin() {
    return to_string(rand() % 90000 + 10000);
}

double read_config_value(const string& key, double def_value) {
    ifstream file("system_config.txt");
    if (!file) return def_value;

    string line;
    while (getline(file, line)) {
        if (line.rfind(key, 0) == 0) {
            try {
                return stod(line.substr(key.size()));
            } catch (...) {
                return def_value;
            }
        }
    }
    return def_value;
}

void log_system_event(const string& actor, const string& action) {
    ofstream file("system_log.dat", ios::binary | ios::app);
    if (!file) return;
    SystemLogRecord log;
    memset(&log, 0, sizeof(log));
    string ts = current_timestamp();
    strcpy(log.timestamp, ts.c_str());
    strcpy(log.actor, actor.c_str());
    strcpy(log.action, action.c_str());
    file.write((char*)&log, sizeof(log));
    file.close();
}

vector<BranchRecord> load_branches() {
    vector<BranchRecord> branches;
    ifstream file("branches.dat", ios::binary);
    if (!file) return branches;
    BranchRecord b;
    while (file.read((char*)&b, sizeof(b))) {
        branches.push_back(b);
    }
    file.close();
    return branches;
}

bool save_branches(vector<BranchRecord>& branches) {
    ofstream file("branches.dat", ios::binary);
    if (!file) return false;
    for (size_t i = 0; i < branches.size(); i++) {
        file.write((char*)&branches[i], sizeof(BranchRecord));
    }
    file.close();
    return true;
}

void update_branch_stats(const string& branch_code, double balance_change, int customer_change) {
    vector<BranchRecord> branches = load_branches();
    for (size_t i = 0; i < branches.size(); i++) {
        if (string(branches[i].code) == branch_code && branches[i].is_active) {
            branches[i].total_balance += balance_change;
            branches[i].customer_count += customer_change;
            if (branches[i].customer_count < 0) branches[i].customer_count = 0;
            if (branches[i].total_balance < 0) branches[i].total_balance = 0;
            break;
        }
    }
    save_branches(branches);
}

void init_system() {
    srand((unsigned)time(0));

    ifstream config_in("system_config.txt");
    if (!config_in.is_open()) {
        ofstream config("system_config.txt");
        config << "MAX_LOGIN_ATTEMPTS=3\n";
        config << "SAVINGS_INTEREST=2.5\n";
        config << "FIXED_INTEREST=5.0\n";
        config << "OVERDRAFT_FEE=25.0\n";
        config << "STUDENT_TRANSACTION_LIMIT=1000.0\n";
        config.close();
    } else {
        config_in.close();
    }

    ifstream branch_in("branches.dat", ios::binary);
    if (!branch_in.is_open()) {
        ofstream bfile("branches.dat", ios::binary);
        BranchRecord b[3] = {
            {"B001", "Sandton City", "1 Sandton Drive, Johannesburg", 0.0, 0, true},
            {"B002", "Cape Town V&A", "22 Long Street, Cape Town", 0.0, 0, true},
            {"B003", "Durban Beachfront", "5 Marine Parade, Durban", 0.0, 0, true}
        };
        bfile.write((char*)b, sizeof(b));
        bfile.close();
        cout << "Default branches created.\n";
    } else {
        branch_in.close();
    }

    ifstream teller_in("tellers.dat", ios::binary);
    if (!teller_in.is_open()) {
        ofstream tfile("tellers.dat", ios::binary);
        TellerRecord t;
        memset(&t, 0, sizeof(t));
        strcpy(t.id, "T001");
        strcpy(t.name, "Default Teller");
        strcpy(t.branch_code, "B001");
        t.is_active = true;
        string salt = "StandardBank#2024";
        hash<string> hasher;
        t.password_hash = hasher(string("1234") + salt);
        tfile.write((char*)&t, sizeof(t));
        tfile.close();
        cout << "Default teller created.\n";
    } else {
        teller_in.close();
    }
}

class Account {
public:
    virtual ~Account() {}
    virtual string type_name() const = 0;
    virtual double min_deposit() const = 0;
    virtual void setup(CustomerRecord& c) = 0;
};

class SavingsAccount : public Account {
public:
    string type_name() const { return "Savings"; }
    double min_deposit() const { return 100.0; }
    void setup(CustomerRecord& c) {
        strcpy(c.last_interest_date, today_date().c_str());
    }
};

class ChequeAccount : public Account {
public:
    string type_name() const { return "Cheque"; }
    double min_deposit() const { return 500.0; }
    void setup(CustomerRecord& c) {
        c.overdraft_limit = 500.0;
        c.monthly_fee = read_config_value("OVERDRAFT_FEE=", 25.0);
    }
};

class FixedDepositAccount : public Account {
public:
    string type_name() const { return "Fixed Deposit"; }
    double min_deposit() const { return 1000.0; }
    void setup(CustomerRecord& c) {
        string today = today_date();
        strcpy(c.last_interest_date, today.c_str());
        string maturity = add_months(today, 12);
        strcpy(c.fixed_maturity_date, maturity.c_str());
    }
};

class StudentAccount : public Account {
public:
    string type_name() const { return "Student"; }
    double min_deposit() const { return 50.0; }
    void setup(CustomerRecord& c) {
        c.transaction_limit = read_config_value("STUDENT_TRANSACTION_LIMIT=", 1000.0);
    }
};

bool find_customer(const string& acc_num, CustomerRecord& record) {
    ifstream file("customers.dat", ios::binary);
    if (!file) return false;
    while (file.read((char*)&record, sizeof(record))) {
        if (string(record.account_number) == acc_num) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool update_customer(const CustomerRecord& record) {
    ifstream infile("customers.dat", ios::binary);
    ofstream temp("temp.dat", ios::binary);
    if (!infile || !temp) return false;

    CustomerRecord old;
    while (infile.read((char*)&old, sizeof(old))) {
        if (string(old.account_number) == string(record.account_number))
            temp.write((char*)&record, sizeof(record));
        else
            temp.write((char*)&old, sizeof(old));
    }

    infile.close();
    temp.close();
    remove("customers.dat");
    rename("temp.dat", "customers.dat");
    return true;
}

bool verify_customer_pin(const string& acc_num, const string& entered_pin) {
    CustomerRecord rec;
    if (!find_customer(acc_num, rec)) {
        cout << "Account not found.\n";
        return false;
    }

    time_t now = time(0);
    if (rec.locked && now < rec.lock_until) {
        cout << "This account is temporarily locked. Try later.\n";
        return false;
    }

    if (rec.locked && now >= rec.lock_until) {
        rec.locked = false;
        rec.failed_attempts = 0;
        rec.lock_until = 0;
        update_customer(rec);
    }

    string real_pin = simple_encrypt(rec.encrypted_pin);
    if (entered_pin == real_pin) {
        rec.failed_attempts = 0;
        rec.locked = false;
        rec.lock_until = 0;
        update_customer(rec);
        return true;
    }

    rec.failed_attempts++;
    if (rec.failed_attempts >= 3) {
        rec.locked = true;
        rec.lock_until = now + 300;
        cout << "Too many wrong attempts. Account locked for 5 minutes.\n";
        log_system_event("SYSTEM", "Locked customer account " + acc_num);
    }
    update_customer(rec);
    return false;
}

void log_transaction(const string& acc_num, const string& type, double amount, double balance, const string& branch_code) {
    ofstream file("transactions.dat", ios::binary | ios::app);
    if (!file) return;

    TransactionRecord t;
    memset(&t, 0, sizeof(t));
    strcpy(t.account_number, acc_num.c_str());
    strcpy(t.type, type.c_str());
    t.amount = amount;
    t.new_balance = balance;
    strcpy(t.branch_code, branch_code.c_str());
    string ts = current_timestamp();
    strcpy(t.date, ts.c_str());

    file.write((char*)&t, sizeof(t));
    file.close();
}

void create_teller() {
    ofstream file("tellers.dat", ios::binary | ios::app);
    if (!file) {
        cout << "Could not open teller file.\n";
        return;
    }

    TellerRecord t;
    memset(&t, 0, sizeof(t));

    string id = get_line("Enter Teller ID: ");
    string name = get_line("Enter Full Name: ");
    string pass = get_line("Enter Password: ");
    string branch = get_line("Enter Branch Code: ");

    strcpy(t.id, id.c_str());
    strcpy(t.name, name.c_str());
    strcpy(t.branch_code, branch.c_str());
    t.is_active = true;

    string salt = "StandardBank#2024";
    hash<string> hasher;
    t.password_hash = hasher(pass + salt);

    file.write((char*)&t, sizeof(t));
    file.close();

    cout << "Teller created.\n";
    log_system_event("ADMIN", "Created teller " + id);
}

bool teller_login(string& branch_code, string& teller_id) {
    string id = get_line("Teller ID: ");
    string pass = get_line("Password: ");

    string salt = "StandardBank#2024";
    hash<string> hasher;
    size_t hashed = hasher(pass + salt);

    ifstream file("tellers.dat", ios::binary);
    if (!file) {
        cout << "No teller data found.\n";
        return false;
    }

    TellerRecord t;
    while (file.read((char*)&t, sizeof(t))) {
        if (id == t.id && hashed == t.password_hash && t.is_active) {
            branch_code = t.branch_code;
            teller_id = t.id;
            cout << "Login successful. Welcome " << t.name << "\n";
            cout << "Branch: " << branch_code << "\n";
            log_system_event(teller_id, "Logged in");
            file.close();
            return true;
        }
    }

    file.close();
    cout << "Invalid teller login.\n";
    return false;
}

void change_teller_password(const string& teller_id) {
    ifstream infile("tellers.dat", ios::binary);
    ofstream temp("temp_tellers.dat", ios::binary);
    if (!infile || !temp) {
        cout << "Could not update teller file.\n";
        return;
    }

    string old_pass = get_line("Enter current password: ");
    string new_pass = get_line("Enter new password: ");

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
                cout << "Password changed.\n";
                log_system_event(teller_id, "Changed password");
            } else {
                cout << "Current password is wrong.\n";
            }
        }
        temp.write((char*)&t, sizeof(t));
    }

    infile.close();
    temp.close();
    remove("tellers.dat");
    rename("temp_tellers.dat", "tellers.dat");

    if (!updated) cout << "Password not changed.\n";
}

void register_customer(const string& branch_code, const string& teller_id) {
    CustomerRecord c;
    memset(&c, 0, sizeof(c));

    cout << "Select account type:\n";
    cout << "1. Savings\n2. Cheque\n3. Fixed Deposit\n4. Student\n";
    int choice = get_int("Choice: ");

    Account* rule = 0;
    SavingsAccount sa;
    ChequeAccount ca;
    FixedDepositAccount fa;
    StudentAccount stu;

    if (choice == 1) rule = &sa;
    else if (choice == 2) rule = &ca;
    else if (choice == 3) rule = &fa;
    else if (choice == 4) rule = &stu;
    else {
        cout << "Invalid type.\n";
        return;
    }

    c.account_type = choice;

    string name = get_line("Full Name: ");
    string said;
    do {
        said = get_line("SA ID (13 digits): ");
        if (!validate_said(said)) cout << "Invalid SA ID.\n";
    } while (!validate_said(said));

    string phone;
    do {
        phone = get_line("Contact Number (10 digits): ");
        if (!validate_phone(phone)) cout << "Invalid contact number.\n";
    } while (!validate_phone(phone));

    string email;
    do {
        email = get_line("Email: ");
        if (!validate_email(email)) cout << "Invalid email.\n";
    } while (!validate_email(email));

    string addr = get_line("Physical Address: ");

    string dob;
    do {
        dob = get_line("Date of Birth (DD/MM/YYYY): ");
        if (!validate_date_format(dob)) cout << "Invalid date format.\n";
    } while (!validate_date_format(dob));

    strcpy(c.full_name, name.c_str());
    strcpy(c.sa_id, said.c_str());
    strcpy(c.contact, phone.c_str());
    strcpy(c.email, email.c_str());
    strcpy(c.address, addr.c_str());
    strcpy(c.dob, dob.c_str());
    strcpy(c.branch_code, branch_code.c_str());

    string acc_num = gen_acc_number(branch_code);
    strcpy(c.account_number, acc_num.c_str());

    rule->setup(c);

    double min_dep = rule->min_deposit();
    double dep = 0.0;
    while (true) {
        stringstream ss;
        ss << fixed << setprecision(2) << min_dep;
        dep = get_double("Initial deposit (minimum R" + ss.str() + "): R");
        if (dep >= min_dep) break;
        cout << "Deposit is too low.\n";
    }
    c.balance = dep;

    string pin = gen_pin();
    string enc = simple_encrypt(pin);
    strcpy(c.encrypted_pin, enc.c_str());
    c.failed_attempts = 0;
    c.locked = false;
    c.lock_until = 0;

    ofstream file("customers.dat", ios::binary | ios::app);
    if (!file) {
        cout << "Could not save customer file.\n";
        return;
    }
    file.write((char*)&c, sizeof(c));
    file.close();

    update_branch_stats(branch_code, dep, 1);
    log_system_event(teller_id, "Registered customer " + acc_num);

    cout << "\nAccount created.\n";
    cout << "Account Number: " << c.account_number << "\n";
    cout << "Generated PIN: " << pin << "\n";
    if (choice == 3) cout << "Maturity Date: " << c.fixed_maturity_date << "\n";
}

void edit_customer_profile(const string& acc_num, const string& teller_id) {
    CustomerRecord c;
    if (!find_customer(acc_num, c)) {
        cout << "Customer not found.\n";
        return;
    }

    cout << "Leave a field blank to keep the old value.\n";

    string name = get_line(string("New Full Name [") + c.full_name + "]: ");
    if (!name.empty()) strcpy(c.full_name, name.c_str());

    string phone = get_line(string("New Contact [") + c.contact + "]: ");
    if (!phone.empty() && validate_phone(phone)) strcpy(c.contact, phone.c_str());

    string email = get_line(string("New Email [") + c.email + "]: ");
    if (!email.empty() && validate_email(email)) strcpy(c.email, email.c_str());

    string addr = get_line(string("New Address [") + c.address + "]: ");
    if (!addr.empty()) strcpy(c.address, addr.c_str());

    if (update_customer(c)) {
        cout << "Profile updated.\n";
        log_system_event(teller_id, "Edited profile " + acc_num);
    } else {
        cout << "Update failed.\n";
    }
}

void close_account(const string& acc_num, const string& teller_id) {
    CustomerRecord c;
    if (!find_customer(acc_num, c)) {
        cout << "Account not found.\n";
        return;
    }

    if (c.balance > 0) {
        cout << "This account still has money in it. Please clear it first.\n";
        return;
    }

    ifstream infile("customers.dat", ios::binary);
    ofstream temp("temp.dat", ios::binary);
    if (!infile || !temp) {
        cout << "Could not close account.\n";
        return;
    }

    CustomerRecord r;
    bool removed = false;
    while (infile.read((char*)&r, sizeof(r))) {
        if (string(r.account_number) == acc_num) {
            removed = true;
            update_branch_stats(r.branch_code, -r.balance, -1);
        } else {
            temp.write((char*)&r, sizeof(r));
        }
    }

    infile.close();
    temp.close();
    remove("customers.dat");
    rename("temp.dat", "customers.dat");

    if (removed) {
        cout << "Account closed.\n";
        log_system_event(teller_id, "Closed account " + acc_num);
    } else {
        cout << "Account not found.\n";
    }
}

void deposit(CustomerRecord& c, const string& branch_code) {
    double amt = get_double("Deposit amount: R");
    c.balance += amt;
    if (update_customer(c)) {
        log_transaction(c.account_number, "DEPOSIT", amt, c.balance, branch_code);
        update_branch_stats(c.branch_code, amt, 0);
        cout << "New balance: R" << c.balance << "\n";
    } else {
        cout << "Deposit failed.\n";
    }
}

void withdraw(CustomerRecord& c, const string& branch_code) {
    double amt = get_double("Withdrawal amount: R");

    if (c.account_type == 3) {
        string today = today_date();
        if (date_key(today) < date_key(c.fixed_maturity_date)) {
            char ans;
            cout << "Early withdrawal penalty will apply. Continue? (y/n): ";
            cin >> ans;
            clear_cin();
            if (tolower((unsigned char)ans) != 'y') return;
            double penalty = amt * 0.01;
            amt += penalty;
            cout << "Total after penalty: R" << amt << "\n";
        }
    }

    if (c.account_type == 4 && amt > c.transaction_limit) {
        cout << "This is above the student transaction limit of R" << c.transaction_limit << "\n";
        return;
    }

    double available = c.balance;
    if (c.account_type == 2) available += c.overdraft_limit;
    if (amt > available) {
        cout << "Not enough money.\n";
        return;
    }

    c.balance -= amt;
    if (update_customer(c)) {
        log_transaction(c.account_number, "WITHDRAWAL", amt, c.balance, branch_code);
        update_branch_stats(c.branch_code, -amt, 0);
        cout << "New balance: R" << c.balance << "\n";
        if (c.account_type == 2 && c.balance < 0) {
            cout << "Cheque overdraft used. Monthly fee: R" << c.monthly_fee << "\n";
        }
    } else {
        cout << "Withdrawal failed.\n";
    }
}

void transfer(CustomerRecord& from, const string& branch_code) {
    string to_acc = get_line("Recipient account number: ");
    CustomerRecord to;

    if (!find_customer(to_acc, to)) {
        cout << "Recipient not found.\n";
        return;
    }

    double amt = get_double("Amount to transfer: R");

    if (from.account_type == 4 && amt > from.transaction_limit) {
        cout << "Above student transaction limit.\n";
        return;
    }

    double available = from.balance;
    if (from.account_type == 2) available += from.overdraft_limit;
    if (amt > available) {
        cout << "Not enough money.\n";
        return;
    }

    from.balance -= amt;
    to.balance += amt;

    if (update_customer(from) && update_customer(to)) {
        log_transaction(from.account_number, "TRANSFER OUT", amt, from.balance, branch_code);
        log_transaction(to.account_number, "TRANSFER IN", amt, to.balance, to.branch_code);
        update_branch_stats(from.branch_code, -amt, 0);
        update_branch_stats(to.branch_code, amt, 0);
        cout << "Transfer done.\n";
    } else {
        cout << "Transfer failed.\n";
    }
}

void view_statement(CustomerRecord& c) {
    cout << "\n=== STATEMENT FOR " << c.account_number << " ===\n";
    cout << "Name: " << c.full_name << "\n";
    cout << "Balance: R" << c.balance << "\n";
    cout << "Branch: " << c.branch_code << "\n";
    if (c.account_type == 2) cout << "Overdraft limit: R" << c.overdraft_limit << "\n";
    if (c.account_type == 3) cout << "Maturity date: " << c.fixed_maturity_date << "\n";

    ifstream file("transactions.dat", ios::binary);
    if (!file) {
        cout << "No transaction history.\n";
        return;
    }

    TransactionRecord t;
    int count = 0;
    while (file.read((char*)&t, sizeof(t))) {
        if (string(t.account_number) == string(c.account_number)) {
            cout << t.date << " | " << t.type << " | R" << t.amount << " | Balance R" << t.new_balance << "\n";
            count++;
            if (count == 10) break;
        }
    }
    file.close();

    if (count == 0) cout << "No transactions yet.\n";
}

void change_pin(CustomerRecord& c) {
    string old_pin = get_line("Current PIN: ");
    string real_pin = simple_encrypt(c.encrypted_pin);

    if (old_pin != real_pin) {
        cout << "Wrong PIN.\n";
        return;
    }

    string new_pin;
    do {
        new_pin = get_line("New PIN (5 digits): ");
        if (!validate_pin(new_pin)) cout << "PIN must be 5 digits.\n";
    } while (!validate_pin(new_pin));

    string enc = simple_encrypt(new_pin);
    strcpy(c.encrypted_pin, enc.c_str());

    if (update_customer(c)) cout << "PIN changed.\n";
    else cout << "Could not update PIN.\n";
}

void apply_interest() {
    ifstream infile("customers.dat", ios::binary);
    if (!infile) {
        cout << "No customer file found.\n";
        return;
    }

    ofstream temp("temp.dat", ios::binary);
    if (!temp) {
        cout << "Could not create temp file.\n";
        return;
    }

    double sav_rate = read_config_value("SAVINGS_INTEREST=", 2.5);
    double fix_rate = read_config_value("FIXED_INTEREST=", 5.0);
    string today = today_date();

    CustomerRecord c;
    int count = 0;
    try {
        while (infile.read((char*)&c, sizeof(c))) {
            if (c.account_type == 1) {
                double interest = c.balance * (sav_rate / 100.0);
                c.balance += interest;
                strcpy(c.last_interest_date, today.c_str());
                count++;
                cout << "Interest added to " << c.account_number << ": R" << fixed << setprecision(2) << interest << "\n";
                log_transaction(c.account_number, "INTEREST", interest, c.balance, c.branch_code);
                update_branch_stats(c.branch_code, interest, 0);
            } else if (c.account_type == 3) {
                double interest = c.balance * (fix_rate / 100.0);
                c.balance += interest;
                strcpy(c.last_interest_date, today.c_str());
                count++;
                cout << "Interest added to " << c.account_number << ": R" << fixed << setprecision(2) << interest << "\n";
                log_transaction(c.account_number, "INTEREST", interest, c.balance, c.branch_code);
                update_branch_stats(c.branch_code, interest, 0);
            }
            temp.write((char*)&c, sizeof(c));
        }
    } catch (...) {
        cout << "Interest calculation error.\n";
    }

    infile.close();
    temp.close();
    remove("customers.dat");
    rename("temp.dat", "customers.dat");

    cout << "Interest applied to " << count << " account(s).\n";
    log_system_event("SYSTEM", "Applied interest to " + to_string(count) + " accounts");
}

void search_customer() {
    string term = lower_str(get_line("Search by account, name, ID, or phone: "));

    ifstream file("customers.dat", ios::binary);
    if (!file) {
        cout << "No customer data.\n";
        return;
    }

    CustomerRecord c;
    bool found = false;
    while (file.read((char*)&c, sizeof(c))) {
        string acc = lower_str(string(c.account_number));
        string name = lower_str(string(c.full_name));
        string id = lower_str(string(c.sa_id));
        string phone = lower_str(string(c.contact));

        if (acc.find(term) != string::npos ||
            name.find(term) != string::npos ||
            id.find(term) != string::npos ||
            phone.find(term) != string::npos) {
            cout << c.account_number << " | " << c.full_name << " | R" << c.balance << " | Branch " << c.branch_code << "\n";
            found = true;
        }
    }
    file.close();

    if (!found) cout << "Nothing matched.\n";
}

void daily_transaction_report() {
    ifstream file("transactions.dat", ios::binary);
    if (!file) {
        cout << "No transactions found.\n";
        return;
    }

    string today = today_date();
    TransactionRecord t;
    int count = 0;
    double total = 0;

    cout << "\n=== DAILY TRANSACTION REPORT (" << today << ") ===\n";
    while (file.read((char*)&t, sizeof(t))) {
        string dt = string(t.date);
        if (dt.size() >= 10 && dt.substr(0, 10) == today) {
            cout << t.date << " | " << t.account_number << " | " << t.type << " | R" << t.amount << " | Branch " << t.branch_code << "\n";
            count++;
            total += t.amount;
        }
    }
    file.close();

    cout << "Total transactions: " << count << "\n";
    cout << "Total volume: R" << total << "\n";
}

void customer_summary_report() {
    ifstream file("customers.dat", ios::binary);
    if (!file) {
        cout << "No customer data.\n";
        return;
    }

    cout << "\n=== CUSTOMER ACCOUNT SUMMARY ===\n";
    CustomerRecord c;
    int count = 0;
    double total = 0.0;

    while (file.read((char*)&c, sizeof(c))) {
        string type;
        if (c.account_type == 1) type = "Savings";
        else if (c.account_type == 2) type = "Cheque";
        else if (c.account_type == 3) type = "Fixed Deposit";
        else type = "Student";

        cout << c.account_number << " | " << c.full_name << " | " << type << " | R" << c.balance << " | Branch " << c.branch_code << "\n";
        count++;
        total += c.balance;
    }
    file.close();

    cout << "\nTotal customers: " << count << "\n";
    cout << "Total balance: R" << total << "\n";
}

void branch_performance_report() {
    vector<BranchRecord> branches = load_branches();
    cout << "\n=== BRANCH PERFORMANCE REPORT ===\n";
    for (size_t i = 0; i < branches.size(); i++) {
        if (!branches[i].is_active) continue;
        cout << branches[i].name << " (" << branches[i].code << ")\n";
        cout << "Address: " << branches[i].address << "\n";
        cout << "Total balance: R" << branches[i].total_balance << "\n";
        cout << "Customers: " << branches[i].customer_count << "\n";
        if (branches[i].customer_count > 0) {
            cout << "Average per customer: R" << branches[i].total_balance / branches[i].customer_count << "\n";
        }
        cout << "----------------------\n";
    }
}

void view_all_branches() {
    vector<BranchRecord> branches = load_branches();
    if (branches.empty()) {
        cout << "No branches.\n";
        return;
    }
    cout << "\n=== ALL BRANCHES ===\n";
    for (size_t i = 0; i < branches.size(); i++) {
        if (branches[i].is_active) {
            cout << branches[i].code << " - " << branches[i].name << "\n";
        }
    }
}

void view_branch_details() {
    string code = get_line("Enter branch code: ");
    vector<BranchRecord> branches = load_branches();
    bool found = false;
    for (size_t i = 0; i < branches.size(); i++) {
        if (string(branches[i].code) == code) {
            found = true;
            cout << "\nBranch Code: " << branches[i].code << "\n";
            cout << "Name: " << branches[i].name << "\n";
            cout << "Address: " << branches[i].address << "\n";
            cout << "Active: " << (branches[i].is_active ? "Yes" : "No") << "\n";
            cout << "Customers: " << branches[i].customer_count << "\n";
            cout << "Total balance: R" << branches[i].total_balance << "\n";
            break;
        }
    }
    if (!found) cout << "Branch not found.\n";
}

void compare_branches() {
    vector<BranchRecord> branches = load_branches();
    if (branches.empty()) {
        cout << "No branches available.\n";
        return;
    }

    int best = -1;
    for (size_t i = 0; i < branches.size(); i++) {
        if (!branches[i].is_active) continue;
        if (best == -1 || branches[i].total_balance > branches[best].total_balance) {
            best = (int)i;
        }
    }

    if (best == -1) {
        cout << "No active branches.\n";
        return;
    }

    cout << "Highest deposits: " << branches[best].name << " (" << branches[best].code << ") with R" << branches[best].total_balance << "\n";
}

void view_system_logs() {
    ifstream file("system_log.dat", ios::binary);
    if (!file) {
        cout << "No logs.\n";
        return;
    }

    SystemLogRecord log;
    cout << "\n=== SYSTEM LOGS ===\n";
    while (file.read((char*)&log, sizeof(log))) {
        cout << log.timestamp << " | " << log.actor << " | " << log.action << "\n";
    }
    file.close();
}

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
    ok = copy_file("customers.dat", "customers_backup.dat") && ok;
    ok = copy_file("transactions.dat", "transactions_backup.dat") && ok;
    ok = copy_file("branches.dat", "branches_backup.dat") && ok;
    ok = copy_file("tellers.dat", "tellers_backup.dat") && ok;
    ok = copy_file("system_log.dat", "system_log_backup.dat") && ok;

    if (ok) cout << "Backup done.\n";
    else cout << "Backup done partly. Some files may be missing.\n";

    log_system_event("SYSTEM", "Backup performed");
}

void recover_data() {
    bool ok = true;
    ok = copy_file("customers_backup.dat", "customers.dat") && ok;
    ok = copy_file("transactions_backup.dat", "transactions.dat") && ok;
    ok = copy_file("branches_backup.dat", "branches.dat") && ok;
    ok = copy_file("tellers_backup.dat", "tellers.dat") && ok;
    ok = copy_file("system_log_backup.dat", "system_log.dat") && ok;

    if (ok) cout << "Recovery done.\n";
    else cout << "Recovery failed. Some backup files may be missing.\n";

    log_system_event("SYSTEM", "Recovery performed");
}

void export_to_csv() {
    ofstream csv("standard_bank_export.csv");
    if (!csv) {
        cout << "Could not create CSV file.\n";
        return;
    }

    csv << "Account Number,Full Name,SA ID,Contact,Email,Address,DOB,Type,Balance,Branch,Maturity\n";

    ifstream file("customers.dat", ios::binary);
    if (!file) {
        cout << "No customer data to export.\n";
        csv.close();
        return;
    }

    CustomerRecord c;
    while (file.read((char*)&c, sizeof(c))) {
        string type;
        if (c.account_type == 1) type = "Savings";
        else if (c.account_type == 2) type = "Cheque";
        else if (c.account_type == 3) type = "Fixed Deposit";
        else type = "Student";

        csv << c.account_number << "," << c.full_name << "," << c.sa_id << ","
            << c.contact << "," << c.email << "," << c.address << ","
            << c.dob << "," << type << "," << c.balance << "," << c.branch_code << ","
            << (c.account_type == 3 ? c.fixed_maturity_date : "") << "\n";
    }
    file.close();
    csv.close();
    cout << "CSV export complete.\n";
}

void export_to_text() {
    ofstream txt("standard_bank_export.txt");
    if (!txt) {
        cout << "Could not create text file.\n";
        return;
    }

    ifstream file("customers.dat", ios::binary);
    if (!file) {
        cout << "No customer data to export.\n";
        txt.close();
        return;
    }

    CustomerRecord c;
    while (file.read((char*)&c, sizeof(c))) {
        string type;
        if (c.account_type == 1) type = "Savings";
        else if (c.account_type == 2) type = "Cheque";
        else if (c.account_type == 3) type = "Fixed Deposit";
        else type = "Student";

        txt << "Account Number: " << c.account_number << "\n";
        txt << "Name: " << c.full_name << "\n";
        txt << "SA ID: " << c.sa_id << "\n";
        txt << "Contact: " << c.contact << "\n";
        txt << "Email: " << c.email << "\n";
        txt << "Address: " << c.address << "\n";
        txt << "DOB: " << c.dob << "\n";
        txt << "Type: " << type << "\n";
        txt << "Balance: R" << c.balance << "\n";
        txt << "Branch: " << c.branch_code << "\n";
        if (c.account_type == 3) txt << "Maturity: " << c.fixed_maturity_date << "\n";
        txt << "---------------------------------\n";
    }
    file.close();
    txt.close();
    cout << "Text export complete.\n";
}

void export_data() {
    export_to_csv();
    export_to_text();
    log_system_event("SYSTEM", "Exported data");
}

void add_branch() {
    BranchRecord b;
    memset(&b, 0, sizeof(b));

    string code = get_line("Enter branch code (example B004): ");
    string name = get_line("Enter branch name: ");
    string addr = get_line("Enter address: ");

    strcpy(b.code, code.c_str());
    strcpy(b.name, name.c_str());
    strcpy(b.address, addr.c_str());
    b.total_balance = 0.0;
    b.customer_count = 0;
    b.is_active = true;

    vector<BranchRecord> branches = load_branches();
    branches.push_back(b);

    if (save_branches(branches)) {
        cout << "Branch added.\n";
        log_system_event("ADMIN", "Added branch " + code);
    } else {
        cout << "Branch not added.\n";
    }
}

void remove_branch() {
    string code = get_line("Enter branch code to remove: ");
    vector<BranchRecord> branches = load_branches();
    bool found = false;

    for (size_t i = 0; i < branches.size(); i++) {
        if (string(branches[i].code) == code) {
            branches[i].is_active = false;
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

void reset_teller_password() {
    string teller_id = get_line("Enter Teller ID to reset: ");
    ifstream infile("tellers.dat", ios::binary);
    ofstream temp("temp_tellers.dat", ios::binary);
    if (!infile || !temp) {
        cout << "Could not open teller file.\n";
        return;
    }

    string new_pass = get_line("Enter new password: ");

    string salt = "StandardBank#2024";
    hash<string> hasher;
    size_t new_hash = hasher(new_pass + salt);

    TellerRecord t;
    bool updated = false;
    while (infile.read((char*)&t, sizeof(t))) {
        if (string(t.id) == teller_id) {
            t.password_hash = new_hash;
            updated = true;
            cout << "Teller password reset successfully.\n";
            log_system_event("ADMIN", "Reset password for teller " + teller_id);
        }
        temp.write((char*)&t, sizeof(t));
    }

    infile.close();
    temp.close();
    remove("tellers.dat");
    rename("temp_tellers.dat", "tellers.dat");

    if (!updated) cout << "Teller ID not found.\n";
}

void reset_customer_pin(const string& teller_id) {
    string acc_num = get_line("Enter Customer Account Number: ");
    CustomerRecord c;
    if (!find_customer(acc_num, c)) {
        cout << "Account not found.\n";
        return;
    }

    string said = get_line("Verify Customer SA ID (13 digits): ");
    if (string(c.sa_id) != said) {
        cout << "SA ID verification failed. Cannot reset PIN.\n";
        return;
    }

    string new_pin;
    do {
        new_pin = get_line("Enter New PIN (5 digits): ");
        if (!validate_pin(new_pin)) cout << "PIN must be 5 digits.\n";
    } while (!validate_pin(new_pin));

    string enc = simple_encrypt(new_pin);
    strcpy(c.encrypted_pin, enc.c_str());
    c.failed_attempts = 0;
    c.locked = false;
    c.lock_until = 0;

    if (update_customer(c)) {
        cout << "Customer PIN reset successfully.\n";
        log_system_event(teller_id, "Reset PIN for account " + acc_num);
    } else {
        cout << "Could not update PIN.\n";
    }
}

void customer_menu(CustomerRecord& c) {
    int choice;
    do {
        cout << "\n=== CUSTOMER PORTAL ===\n";
        cout << "Welcome, " << c.full_name << "\n";
        cout << "1. View Balance & Details\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. Transfer\n";
        cout << "5. Statement\n";
        cout << "6. Change PIN\n";
        cout << "7. Logout\n";
        choice = get_int("Choice: ");

        if (choice == 1) {
            cout << "Account Number: " << c.account_number << "\n";
            cout << "Balance: R" << c.balance << "\n";
            cout << "Branch: " << c.branch_code << "\n";
            if (c.account_type == 2) cout << "Overdraft Limit: R" << c.overdraft_limit << "\n";
            if (c.account_type == 3) cout << "Maturity Date: " << c.fixed_maturity_date << "\n";
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

    cout << "Logged out.\n";
}

void teller_menu(const string& branch_code, const string& teller_id) {
    int choice;
    do {
        cout << "\n=== TELLER MENU (Branch " << branch_code << ") ===\n";
        cout << "1. Register Customer\n";
        cout << "2. Assisted Transaction\n";
        cout << "3. Search Customer\n";
        cout << "4. Edit Customer Profile\n";
        cout << "5. Close Account\n";
        cout << "6. View All Branches\n";
        cout << "7. View Branch Details\n";
        cout << "8. Reports\n";
        cout << "9. Change Password\n";
        cout << "10. Reset Customer PIN\n";
        cout << "11. Logout\n";

        choice = get_int("Choice: ");

        if (choice == 1) {
            register_customer(branch_code, teller_id);
        } else if (choice == 2) {
            string acc = get_line("Customer account: ");
            string pin = get_line("Customer PIN: ");
            if (verify_customer_pin(acc, pin)) {
                CustomerRecord c;
                if (find_customer(acc, c)) {
                    cout << "Customer: " << c.full_name << "\n";
                    cout << "Balance: R" << c.balance << "\n";
                    int t = get_int("1. Deposit  2. Withdraw\nChoice: ");
                    if (t == 1) deposit(c, branch_code);
                    else if (t == 2) withdraw(c, branch_code);
                }
            } else {
                cout << "PIN verification failed.\n";
            }
        } else if (choice == 3) {
            search_customer();
        } else if (choice == 4) {
            string acc = get_line("Enter account number: ");
            edit_customer_profile(acc, teller_id);
        } else if (choice == 5) {
            string acc = get_line("Enter account number to close: ");
            close_account(acc, teller_id);
        } else if (choice == 6) {
            view_all_branches();
        } else if (choice == 7) {
            view_branch_details();
        } else if (choice == 8) {
            int r = get_int("1. Daily Transactions\n2. Customer Summary\n3. Branch Performance\nChoice: ");
            if (r == 1) daily_transaction_report();
            else if (r == 2) customer_summary_report();
            else if (r == 3) branch_performance_report();
        } else if (choice == 9) {
            change_teller_password(teller_id);
        } else if (choice == 10) {
            reset_customer_pin(teller_id);
        }
    } while (choice != 11);

    log_system_event(teller_id, "Logged out");
}

void admin_menu() {
    int choice;
    do {
        cout << "\n=== ADMIN MENU ===\n";
        cout << "1. Add Branch\n";
        cout << "2. Remove Branch\n";
        cout << "3. View System Logs\n";
        cout << "4. Apply Interest\n";
        cout << "5. Backup Data\n";
        cout << "6. Recover Data\n";
        cout << "7. Export Data\n";
        cout << "8. Reset Teller Password\n";
        cout << "9. Exit Admin Menu\n";

        choice = get_int("Choice: ");

        if (choice == 1) add_branch();
        else if (choice == 2) remove_branch();
        else if (choice == 3) view_system_logs();
        else if (choice == 4) apply_interest();
        else if (choice == 5) backup_data();
        else if (choice == 6) recover_data();
        else if (choice == 7) export_data();
        else if (choice == 8) reset_teller_password();
    } while (choice != 9);
}

int main() {
    init_system();

    cout << "\n*** WELCOME TO STANDARD BANK MULTI-BRANCH ***\n";

    int choice;
    do {
        cout << "\n=== MAIN MENU ===\n";
        cout << "1. Create Teller\n";
        cout << "2. Teller Login\n";
        cout << "3. Customer Login\n";
        cout << "4. View All Branches\n";
        cout << "5. View Branch Details\n";
        cout << "6. Inter-Branch Comparison\n";
        cout << "7. Apply Interest\n";
        cout << "8. Search Customer\n";
        cout << "9. Daily Transaction Report\n";
        cout << "10. Customer Summary\n";
        cout << "11. Branch Performance\n";
        cout << "12. Backup Data\n";
        cout << "13. Recover Data\n";
        cout << "14. Export Data\n";
        cout << "15. Admin Menu\n";
        cout << "16. Exit\n";

        choice = get_int("Choice: ");

        if (choice == 1) {
            create_teller();
        } else if (choice == 2) {
            string br, tid;
            if (teller_login(br, tid)) {
                teller_menu(br, tid);
            }
        } else if (choice == 3) {
            string acc = get_line("Account Number: ");
            string pin = get_line("PIN: ");
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
            view_all_branches();
        } else if (choice == 5) {
            view_branch_details();
        } else if (choice == 6) {
            compare_branches();
        } else if (choice == 7) {
            apply_interest();
        } else if (choice == 8) {
            search_customer();
        } else if (choice == 9) {
            daily_transaction_report();
        } else if (choice == 10) {
            customer_summary_report();
        } else if (choice == 11) {
            branch_performance_report();
        } else if (choice == 12) {
            backup_data();
        } else if (choice == 13) {
            recover_data();
        } else if (choice == 14) {
            export_data();
        } else if (choice == 15) {
            admin_menu();
        }
    } while (choice != 16);

    cout << "Thank you for banking with Standard Bank. Goodbye!\n";
    return 0;
}