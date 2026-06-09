#include "teller.h"
#include "records.h"
#include "utils.h"   // get_line, log_system_event

#include <iostream>
#include <fstream>
#include <cstring>
#include <functional>

using namespace std;

// Salt used when hashing teller passwords. Kept in one place so every
// teller operation hashes identically.
static const string SALT = "StandardBank#2024";

// --- Teller class methods (declared in records.h) ---
void Teller::set_password(const string& plain) {
    hash<string> hasher;
    password_hash = hasher(plain + SALT);
}

bool Teller::check_password(const string& plain) const {
    hash<string> hasher;
    return password_hash == hasher(plain + SALT);
}

void create_teller() {
    ofstream file("tellers.dat", ios::binary | ios::app);
    if (!file) {
        cout << "Could not open teller file.\n";
        return;
    }

    Teller t;
    memset(&t, 0, sizeof(t));

    string id = get_line("Enter Teller ID: ");
    string name = get_line("Enter Full Name: ");
    string pass = get_line("Enter Password: ");
    string branch = get_line("Enter Branch Code: ");

    strcpy(t.id, id.c_str());
    strcpy(t.name, name.c_str());
    strcpy(t.branch_code, branch.c_str());
    t.is_active = true;
    t.set_password(pass);

    file.write((char*)&t, sizeof(t));
    file.close();

    cout << "Teller created.\n";
    log_system_event("ADMIN", "Created teller " + id);
}

bool teller_login(string& branch_code, string& teller_id) {
    string id = get_line("Teller ID: ");
    string pass = get_line("Password: ");

    ifstream file("tellers.dat", ios::binary);
    if (!file) {
        cout << "No teller data found.\n";
        return false;
    }

    Teller t;
    while (file.read((char*)&t, sizeof(t))) {
        if (id == t.id && t.check_password(pass) && t.is_active) {
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

    Teller t;
    bool updated = false;
    while (infile.read((char*)&t, sizeof(t))) {
        if (string(t.id) == teller_id) {
            if (t.check_password(old_pass)) {
                t.set_password(new_pass);
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

void reset_teller_password() {
    string teller_id = get_line("Enter Teller ID to reset: ");
    ifstream infile("tellers.dat", ios::binary);
    ofstream temp("temp_tellers.dat", ios::binary);
    if (!infile || !temp) {
        cout << "Could not open teller file.\n";
        return;
    }

    string new_pass = get_line("Enter new password: ");

    Teller t;
    bool updated = false;
    while (infile.read((char*)&t, sizeof(t))) {
        if (string(t.id) == teller_id) {
            t.set_password(new_pass);
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
