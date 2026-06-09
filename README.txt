================================================================================
MULTI-BRANCH BANKING MANAGEMENT SYSTEM (C++)
============================================

1. SYSTEM REQUIREMENTS

---

* C++ Compiler (g++, CodeBlocks, or Visual Studio)
* Works on Windows, Linux, or macOS

2. PROJECT STRUCTURE

---

The program is split into separate modules instead of one large file:

records.h                 - shared data structures (written to the .dat files)
utils.h / .cpp            - input, string, date, config and logging helpers
validation.h / .cpp       - field validators (SA ID, phone, email, PIN, date)
accounts.h / .cpp         - account-type rules (Savings/Cheque/Fixed/Student)
branch.h / .cpp           - branch storage, admin and branch reports
customer.h / .cpp         - customer storage, transactions and customer reports
teller.h / .cpp           - teller creation, login and password management
admin.h / .cpp            - setup, interest, backup/recover, export, logs
menus.h / .cpp            - console menu loops (main / customer / teller / admin)
main.cpp                  - entry point (init_system then main_menu)
Makefile                  - builds everything into bank.exe

3. HOW TO COMPILE AND RUN

---

Using make (recommended):
    make
    ./bank.exe     (or just bank.exe on Windows)

Using g++ directly:
    g++ -std=c++17 *.cpp -o bank.exe
    ./bank.exe

Using VS Code:
    Press the Build button (the task compiles all .cpp files into bank.exe).

NOTE:
The program will automatically create required files (.dat, .txt) in the same folder.

3. FIRST TIME SETUP – DEFAULT TELLER

---

A default teller is already available for testing:

Teller ID: T001
Password: 1234
Branch: B001

You can also create your own teller from the main menu.

4. DEFAULT BRANCHES

---

The system automatically creates 3 branches:

B001 - Sandton City
B002 - Cape Town V&A
B003 - Durban Beachfront

5. REGISTERING A CUSTOMER (TELLER)

---

Steps:

1. Login as Teller (Option 2)

2. Choose "Register Customer"

3. Select Account Type:
   1 - Savings (min R100)
   2 - Cheque (min R500, overdraft allowed)
   3 - Fixed Deposit (min R1000, has maturity date)
   4 - Student (min R50, transaction limit)

4. Enter customer details:

   * Full Name
   * SA ID (13 digits, validated)
   * Phone (10 digits)
   * Email (must have @ and .)
   * Address
   * Date of Birth

5. Enter initial deposit

6. System generates:

   * Account Number (ACC-BRANCH-XXXX)
   * 5-digit PIN

IMPORTANT:
PIN is shown ONCE — save it!

6. CUSTOMER LOGIN

---

Steps:

1. Choose "Customer Login"
2. Enter:

   * Account Number
   * PIN

If correct → customer menu opens

Security:

* 3 wrong PIN attempts → account gets locked
* PIN is stored encrypted (XOR method)

7. CUSTOMER FEATURES

---

1. View Balance

2. Deposit

3. Withdraw

4. Transfer

5. View Statement (last 10 transactions)

6. Change PIN

7. Logout

8. TELLER FEATURES

---

After login, tellers can:

1. Register Customer

2. Assisted Transactions

   * Verify customer PIN
   * Deposit or Withdraw

3. Search Customer

4. Edit Customer Details

5. Close Account

6. View Branches

7. Reports:

   * Daily Transactions
   * Customer Summary
   * Branch Performance

8. Change Password

9. ADMIN FEATURES

---

1. Add Branch

2. Remove Branch

3. View System Logs

4. Apply Interest

5. Backup Data

6. Recover Data

7. Export CSV

8. TRANSACTIONS

---

All transactions include:

* Deposit
* Withdrawal
* Transfer

Rules:

* Fixed Deposit → penalty if withdrawn early
* Student → daily transaction limit
* Cheque → overdraft allowed

All transactions are saved in transactions.dat

11. INTEREST SYSTEM

---

* Savings accounts → ~2.5%
* Fixed Deposit → ~5%
* Applied using "Apply Interest" option

12. FILES USED

---

customers.dat        - stores customer accounts
transactions.dat     - stores transactions
branches.dat         - stores branch data
tellers.dat          - stores teller accounts
system_config.txt    - system settings
system_log.dat       - activity logs
standard_bank_export.csv - exported data

13. BACKUP & RECOVERY

---

* Backup creates copies of all files
* Recovery restores from backup files

14. TESTING TIPS

---

1. Login using:
   T001 / 1234

2. Register a customer

3. Save account number + PIN

4. Login as customer

5. Test:

   * Deposit
   * Withdraw
   * Transfer
   * Statement

6. Check reports from main menu

References:

Alex (2007). 14.5 — Public and private members and access specifiers – Learn C++. [online] Learncpp.com. Available at: https://www.learncpp.com/cpp-tutorial/public-and-private-members-and-access-specifiers/ [Accessed 23 Apr. 2026].

‌Cppreference.com. (2024). Main function - cppreference.com. [online] Available at: https://en.cppreference.com/cpp/language/main_function [Accessed 23 Apr. 2026].

‌W3schools.com. (2025). W3Schools.com. [online] Available at: https://www.w3schools.com/cpp/cpp_ref_iostream.asp.

‌W3schools.com. (2024). W3Schools.com. [online] Available at: https://www.w3schools.com/cpp/cpp_ref_fstream.asp.

‌GeeksforGeeks (2018). File Handling through C++ Classes. [online] GeeksforGeeks. Available at: https://www.geeksforgeeks.org/cpp/file-handling-c-classes/.

‌cplusplus.com. (n.d.). Input/output with files - C++ Tutorials. [online] Available at: https://cplusplus.com/doc/tutorial/files/.

‌Alen Bluberry (2025). Binary File Handling in C++: A Beginner’s Guide. [online] DEV Community. Available at: https://dev.to/alen_pythonista_bb/binary-file-handling-in-c-a-beginners-guide-148o [Accessed 23 Apr. 2026].

‌tbhaxor (2022). GitHub - tbhaxor/cpp-file-handling-tutorial: A comprehensive guide to file handling in C++ language. [online] GitHub. Available at: https://github.com/tbhaxor/cpp-file-handling-tutorial.

‌GeeksforGeeks (2015). Object Oriented Programming in C++. [online] GeeksforGeeks. Available at: https://www.geeksforgeeks.org/cpp/object-oriented-programming-in-cpp/.

‌Alex (2008). 27.1 — The need for exceptions – Learn C++. [online] Learncpp.com. Available at: https://www.learncpp.com/cpp-tutorial/the-need-for-exceptions/ [Accessed 23 Apr. 2026].

‌GeeksforGeeks. (2017). Luhn algorithm - GeeksforGeeks. [online] Available at: https://www.geeksforgeeks.org/luhn-algorithm/.

‌

================================================================================
END OF README
=============
