#include "admin.h"
#include "records.h"
#include "branch.h"   // update_branch_stats
#include "utils.h"    // read_config_value, today_date, log_transaction, log_system_event

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iomanip>
#include <functional>

using namespace std;

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
        Teller t;
        memset(&t, 0, sizeof(t));
        strcpy(t.id, "T001");
        strcpy(t.name, "Default Teller");
        strcpy(t.branch_code, "B001");
        t.is_active = true;
        t.set_password("1234");
        tfile.write((char*)&t, sizeof(t));
        tfile.close();
        cout << "Default teller created.\n";
    } else {
        teller_in.close();
    }
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
            // Guard: only eligible accounts that have NOT already been credited
            // today receive interest, so re-running the option cannot double-pay.
            bool already_done = (string(c.last_interest_date) == today);
            if (c.account_type == 1 && !already_done) {
                double interest = c.balance * (sav_rate / 100.0);
                c.balance += interest;
                strcpy(c.last_interest_date, today.c_str());
                count++;
                cout << "Interest added to " << c.account_number << ": R" << fixed << setprecision(2) << interest << "\n";
                log_transaction(c.account_number, "INTEREST", interest, c.balance, c.branch_code);
                update_branch_stats(c.branch_code, interest, 0);
            } else if (c.account_type == 3 && !already_done) {
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

static bool copy_file(const string& src, const string& dst) {
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

static void export_to_csv() {
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

static void export_to_text() {
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
