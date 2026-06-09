#pragma once

// ---------------------------------------------------------------------------
// validation.h
// Input validators for customer-facing fields.
// ---------------------------------------------------------------------------

#include <string>

bool validate_phone(const std::string& phone);
bool validate_pin(const std::string& pin);
bool validate_date_format(const std::string& d);
bool validate_email(const std::string& email);
bool validate_said(const std::string& id);  // SA ID number (Luhn checksum)
