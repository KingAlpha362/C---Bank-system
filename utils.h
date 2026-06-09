#pragma once

// ---------------------------------------------------------------------------
// utils.h
// Low-level infrastructure shared by every module: console input helpers,
// string/date utilities, simple encryption, config reading and logging.
// ---------------------------------------------------------------------------

#include <string>

// --- Console input helpers ---
void clear_cin();
std::string get_line(const std::string& prompt);
int get_int(const std::string& prompt);
double get_double(const std::string& prompt);

// --- String helpers ---
std::string lower_str(std::string s);
bool all_digits(const std::string& s);
std::string simple_encrypt(std::string input);

// --- Date / time helpers ---
int date_key(const std::string& d);
std::string today_date();
std::string current_timestamp();
std::string add_months(const std::string& d, int months);

// --- Misc ---
std::string gen_pin();
double read_config_value(const std::string& key, double def_value);

// --- Logging ---
void log_system_event(const std::string& actor, const std::string& action);
void log_transaction(const std::string& acc_num, const std::string& type,
                     double amount, double balance, const std::string& branch_code);
