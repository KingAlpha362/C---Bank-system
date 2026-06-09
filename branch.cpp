#include "branch.h"
#include "utils.h"   // get_line, log_system_event

#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

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

void branch_performance_report(const string& branch_filter) {
    vector<BranchRecord> branches = load_branches();
    cout << "\n=== BRANCH PERFORMANCE REPORT ===\n";
    for (size_t i = 0; i < branches.size(); i++) {
        if (!branches[i].is_active) continue;
        if (!branch_filter.empty() && string(branches[i].code) != branch_filter) continue;
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
