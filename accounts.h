#pragma once

// ---------------------------------------------------------------------------
// accounts.h
// Account-type rules using polymorphism. Each concrete account defines its
// display name, minimum opening deposit and any type-specific setup applied
// to a new CustomerRecord.
// ---------------------------------------------------------------------------

#include <string>
#include "records.h"

class Account {
public:
    virtual ~Account() {}
    virtual std::string type_name() const = 0;
    virtual double min_deposit() const = 0;
    virtual void setup(CustomerRecord& c) = 0;
};

class SavingsAccount : public Account {
public:
    std::string type_name() const;
    double min_deposit() const;
    void setup(CustomerRecord& c);
};

class ChequeAccount : public Account {
public:
    std::string type_name() const;
    double min_deposit() const;
    void setup(CustomerRecord& c);
};

class FixedDepositAccount : public Account {
public:
    std::string type_name() const;
    double min_deposit() const;
    void setup(CustomerRecord& c);
};

class StudentAccount : public Account {
public:
    std::string type_name() const;
    double min_deposit() const;
    void setup(CustomerRecord& c);
};
