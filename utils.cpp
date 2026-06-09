#include "utils.h"
#include "records.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cctype>
#include <algorithm>

using namespace std;

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
