#include "customer.h"
#include "accounts.h"
#include "branch.h"
#include "validation.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cctype>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// --- File-local helpers (not part of the public interface) ---

static string gen_acc_number(const string& branch_code) {
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

static bool update_customer(const CustomerRecord& record) {
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

// --- Public interface ---

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
