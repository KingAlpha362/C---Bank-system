#pragma once

// ---------------------------------------------------------------------------
// records.h
// Shared on-disk data structures for the Multi-Branch Banking System.
//
// IMPORTANT: these structs are written/read directly to/from the binary .dat
// files. Do NOT change field order, types or sizes, or existing data files
// (customers.dat, branches.dat, tellers.dat, transactions.dat, system_log.dat)
// will no longer load correctly.
// ---------------------------------------------------------------------------

#include <ctime>    // time_t
#include <cstddef>  // size_t
#include <string>

// Teller is a standard-layout class (all members public, no virtuals) so it can
// still be written/read directly to tellers.dat with the same byte layout the
// original struct used. The two helper methods encapsulate password hashing;
// they are defined in teller.cpp.
class Teller {
public:
    char id[10];
    char name[50];
    size_t password_hash;
    char branch_code[10];
    bool is_active;

    void set_password(const std::string& plain);          // hash + store
    bool check_password(const std::string& plain) const;  // compare against stored hash
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
