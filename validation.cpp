#include "validation.h"
#include "utils.h"   // all_digits

#include <cctype>
#include <cstdio>
#include <ctime>

using namespace std;

bool validate_full_name(const string& name) {
    // Require at least two whitespace-separated, non-empty tokens
    // (first name and surname); extra middle names are allowed.
    int tokens = 0;
    bool in_token = false;
    for (char ch : name) {
        if (isspace((unsigned char)ch)) {
            in_token = false;
        } else if (!in_token) {
            in_token = true;
            tokens++;
        }
    }
    return tokens >= 2;
}

bool validate_phone(const string& phone) {
    return phone.size() == 10 && all_digits(phone);
}

bool validate_pin(const string& pin) {
    return pin.size() == 5 && all_digits(pin);
}

bool validate_date_format(const string& d) {
    // Structural check: DD/MM/YYYY with digits in the right places.
    if (d.size() != 10) return false;
    if (d[2] != '/' || d[5] != '/') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)d[i])) return false;
    }

    // Calendar check: reject impossible dates (e.g. 45/45/2000, 31/02/2020).
    int day = 0, mon = 0, year = 0;
    if (sscanf(d.c_str(), "%d/%d/%d", &day, &mon, &year) != 3) return false;

    time_t now = time(0);
    int this_year = 1900 + localtime(&now)->tm_year;
    if (year < 1900 || year > this_year) return false;
    if (mon < 1 || mon > 12) return false;

    int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (mon == 2 && leap) days_in_month[1] = 29;
    if (day < 1 || day > days_in_month[mon - 1]) return false;

    return true;
}

bool validate_email(const string& email) {
    size_t at = email.find('@');
    size_t dot = email.rfind('.');
    return at != string::npos && dot != string::npos && at < dot;
}

bool validate_said(const string& id) {
    if (id.size() != 13 || !all_digits(id)) return false;
    int sum = 0;
    bool alt = true;
    for (int i = 11; i >= 0; i--) {
        int digit = id[i] - '0';
        if (alt) {
            digit *= 2;
            if (digit > 9) digit = (digit % 10) + 1;
        }
        sum += digit;
        alt = !alt;
    }
    int check = (10 - (sum % 10)) % 10;
    return check == (id[12] - '0');
}
