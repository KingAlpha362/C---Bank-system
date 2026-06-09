#include "accounts.h"
#include "utils.h"   // today_date, add_months, read_config_value

#include <cstring>

using namespace std;

// --- Savings ---
string SavingsAccount::type_name() const { return "Savings"; }
double SavingsAccount::min_deposit() const { return 100.0; }
void SavingsAccount::setup(CustomerRecord& c) {
    strcpy(c.last_interest_date, today_date().c_str());
}

// --- Cheque ---
string ChequeAccount::type_name() const { return "Cheque"; }
double ChequeAccount::min_deposit() const { return 500.0; }
void ChequeAccount::setup(CustomerRecord& c) {
    c.overdraft_limit = 500.0;
    c.monthly_fee = read_config_value("OVERDRAFT_FEE=", 25.0);
}

// --- Fixed Deposit ---
string FixedDepositAccount::type_name() const { return "Fixed Deposit"; }
double FixedDepositAccount::min_deposit() const { return 1000.0; }
void FixedDepositAccount::setup(CustomerRecord& c) {
    string today = today_date();
    strcpy(c.last_interest_date, today.c_str());
    string maturity = add_months(today, 12);
    strcpy(c.fixed_maturity_date, maturity.c_str());
}

// --- Student ---
string StudentAccount::type_name() const { return "Student"; }
double StudentAccount::min_deposit() const { return 50.0; }
void StudentAccount::setup(CustomerRecord& c) {
    c.transaction_limit = read_config_value("STUDENT_TRANSACTION_LIMIT=", 1000.0);
}
