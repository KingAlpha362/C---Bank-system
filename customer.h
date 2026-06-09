#pragma once

// ---------------------------------------------------------------------------
// customer.h
// Customer storage, authentication, transactions and customer reports.
// ---------------------------------------------------------------------------

#include <string>
#include "records.h"

// Storage / auth
bool find_customer(const std::string& acc_num, CustomerRecord& record);
bool verify_customer_pin(const std::string& acc_num, const std::string& entered_pin);

// Account lifecycle
void register_customer(const std::string& branch_code, const std::string& teller_id);
void edit_customer_profile(const std::string& acc_num, const std::string& teller_id);
void close_account(const std::string& acc_num, const std::string& teller_id);
// branch_code (when non-empty) restricts the reset to a customer in that branch.
void reset_customer_pin(const std::string& teller_id, const std::string& branch_code = "");

// Transactions
void deposit(CustomerRecord& c, const std::string& branch_code);
void withdraw(CustomerRecord& c, const std::string& branch_code);
void transfer(CustomerRecord& from, const std::string& branch_code);
void view_statement(CustomerRecord& c);
void change_pin(CustomerRecord& c);

// Search / reports.
// branch_filter (when non-empty) limits results to that branch only, so a teller
// sees only their own branch while the main menu (default "") sees everything.
void search_customer(const std::string& branch_filter = "");
void customer_summary_report(const std::string& branch_filter = "");
void daily_transaction_report(const std::string& branch_filter = "");
