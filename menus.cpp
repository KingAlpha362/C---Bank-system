#include "menus.h"
#include "records.h"
#include "utils.h"
#include "customer.h"
#include "teller.h"
#include "branch.h"
#include "admin.h"

#include <iostream>
#include <string>

using namespace std;

static void customer_menu(CustomerRecord& c) {
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

static void admin_menu();  // forward declaration (defined below)

static void teller_menu(const string& branch_code, const string& teller_id) {
    int choice;
    do {
        cout << "\n=== TELLER MENU (Branch " << branch_code << ") ===\n";
        cout << "1. Register Customer\n";
        cout << "2. Assisted Transaction\n";
        cout << "3. Search Customer (my branch)\n";
        cout << "4. Edit Customer Profile\n";
        cout << "5. Close Account\n";
        cout << "6. View All Branches\n";
        cout << "7. View Branch Details\n";
        cout << "8. Inter-Branch Comparison\n";
        cout << "9. Reports (my branch)\n";
        cout << "10. Change My Password\n";
        cout << "11. Reset Customer PIN\n";
        cout << "12. Admin / System Operations\n";
        cout << "13. Logout\n";

        choice = get_int("Choice: ");

        if (choice == 1) {
            register_customer(branch_code, teller_id);
        } else if (choice == 2) {
            string acc = get_line("Customer account: ");
            string pin = get_line("Customer PIN: ");
            if (verify_customer_pin(acc, pin)) {
                CustomerRecord c;
                if (find_customer(acc, c)) {
                    if (string(c.branch_code) != branch_code) {
                        cout << "Access denied: this customer belongs to another branch.\n";
                    } else {
                        cout << "Customer: " << c.full_name << "\n";
                        cout << "Balance: R" << c.balance << "\n";
                        int t = get_int("1. Deposit  2. Withdraw\nChoice: ");
                        if (t == 1) deposit(c, branch_code);
                        else if (t == 2) withdraw(c, branch_code);
                    }
                }
            } else {
                cout << "PIN verification failed.\n";
            }
        } else if (choice == 3) {
            search_customer(branch_code);  // restricted to this teller's branch
        } else if (choice == 4) {
            string acc = get_line("Enter account number: ");
            CustomerRecord c;
            if (find_customer(acc, c) && string(c.branch_code) != branch_code)
                cout << "Access denied: this customer belongs to another branch.\n";
            else
                edit_customer_profile(acc, teller_id);
        } else if (choice == 5) {
            string acc = get_line("Enter account number to close: ");
            CustomerRecord c;
            if (find_customer(acc, c) && string(c.branch_code) != branch_code)
                cout << "Access denied: this customer belongs to another branch.\n";
            else
                close_account(acc, teller_id);
        } else if (choice == 6) {
            view_all_branches();
        } else if (choice == 7) {
            view_branch_details();
        } else if (choice == 8) {
            compare_branches();
        } else if (choice == 9) {
            int r = get_int("1. Daily Transactions\n2. Customer Summary\n3. Branch Performance\nChoice: ");
            if (r == 1) daily_transaction_report(branch_code);
            else if (r == 2) customer_summary_report(branch_code);
            else if (r == 3) branch_performance_report(branch_code);
        } else if (choice == 10) {
            change_teller_password(teller_id);
        } else if (choice == 11) {
            reset_customer_pin(teller_id, branch_code);
        } else if (choice == 12) {
            admin_menu();
        }
    } while (choice != 13);

    log_system_event(teller_id, "Logged out");
}

static void admin_menu() {
    int choice;
    do {
        cout << "\n=== ADMIN / SYSTEM OPERATIONS ===\n";
        cout << "1. Create Teller\n";
        cout << "2. Add Branch\n";
        cout << "3. Remove Branch\n";
        cout << "4. View System Logs\n";
        cout << "5. Apply Interest\n";
        cout << "6. Backup Data\n";
        cout << "7. Recover Data\n";
        cout << "8. Export Data\n";
        cout << "9. Reset Teller Password\n";
        cout << "10. Back\n";

        choice = get_int("Choice: ");

        if (choice == 1) create_teller();
        else if (choice == 2) add_branch();
        else if (choice == 3) remove_branch();
        else if (choice == 4) view_system_logs();
        else if (choice == 5) apply_interest();
        else if (choice == 6) backup_data();
        else if (choice == 7) recover_data();
        else if (choice == 8) export_data();
        else if (choice == 9) reset_teller_password();
    } while (choice != 10);
}

void main_menu() {
    cout << "\n*** WELCOME TO STANDARD BANK MULTI-BRANCH ***\n";

    // Opening gate: nothing in the system is reachable without logging in first
    // (assignment req 1.1.1). All teller, admin and customer operations live
    // behind these logins.
    int choice;
    do {
        cout << "\n=== MAIN MENU ===\n";
        cout << "1. Teller Login\n";
        cout << "2. Customer Login\n";
        cout << "3. Exit\n";

        choice = get_int("Choice: ");

        if (choice == 1) {
            string br, tid;
            if (teller_login(br, tid)) {
                teller_menu(br, tid);
            }
        } else if (choice == 2) {
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
        }
    } while (choice != 3);

    cout << "Thank you for banking with Standard Bank. Goodbye!\n";
}
