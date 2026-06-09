#include "validation.h"
#include "utils.h"   // all_digits

#include <cctype>

using namespace std;

bool validate_phone(const string& phone) {
    return phone.size() == 10 && all_digits(phone);
}

bool validate_pin(const string& pin) {
    return pin.size() == 5 && all_digits(pin);
}

bool validate_date_format(const string& d) {
    if (d.size() != 10) return false;
    if (d[2] != '/' || d[5] != '/') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)d[i])) return false;
    }
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
