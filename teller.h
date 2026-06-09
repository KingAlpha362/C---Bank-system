#pragma once

// ---------------------------------------------------------------------------
// teller.h
// Teller account creation, authentication and password management.
// ---------------------------------------------------------------------------

#include <string>

void create_teller();
bool teller_login(std::string& branch_code, std::string& teller_id);
void change_teller_password(const std::string& teller_id);
void reset_teller_password();
