================================================================================
                   MULTI-BRANCH BANKING MANAGEMENT SYSTEM
                         Console Application (C++)
================================================================================

1. SYSTEM REQUIREMENTS
----------------------
- C++ compiler (g++ recommended, or Visual Studio/MSVC)
- Windows, Linux, or macOS

2. HOW TO COMPILE AND RUN
--------------------------
Open a terminal / command prompt in the folder containing "bank.cpp".

Using g++ (MinGW on Windows, or Linux/macOS):
   g++ bank.cpp -o bank.exe
   ./bank.exe      (or just "bank.exe" on Windows)

Using Microsoft Visual Studio:
   - Create a new Console Application project.
   - Replace the contents of the main .cpp file with bank.cpp.
   - Build and run (Ctrl+F5).

Note: The program creates several binary data files (.dat) and a config file
in the same folder. Do not delete them while the program is running.

3. FIRST TIME SETUP – CREATE A TELLER
---------------------------------------
When you run the program for the first time, there are no teller accounts.
You must create one before you can log in as a teller.

   a) From the main menu, choose option 1. "Create Teller"
   b) Enter a Teller ID (e.g., T001)
   c) Enter a Full Name (e.g., John Doe)
   d) Enter a Password (e.g., pass123)
   e) Enter a Branch Code (use B001, B002, or B003 – these are pre‑defined)

After creation, you can use these credentials for teller login.

4. DEFAULT BRANCHES (ALREADY PRESENT)
--------------------------------------
The system automatically creates three branches:
   B001 - Johannesburg Main
   B002 - Cape Town Branch
   B003 - Durban Branch

5. HOW TO REGISTER A NEW CUSTOMER (TELLER FUNCTION)
-----------------------------------------------------
1. Log in as a teller (Main menu option 2).
2. Choose "1. Register New Customer".
3. Select an account type:
     1 - Savings   (min deposit R100)
     2 - Cheque    (min deposit R500, overdraft facility)
     3 - Fixed Deposit (min deposit R1000, no early withdrawal)
     4 - Student   (min deposit R50, daily transaction limit)
4. Fill in the required details (name, SA ID, phone, email, address, DOB).
   The program validates:
      - SA ID: exactly 13 digits
      - Phone: exactly 10 digits
      - Email: must contain '@' and '.'
5. Enter an initial deposit (must be at least the minimum for that account type).
6. The system generates a 5‑digit PIN and an account number automatically.
   IMPORTANT: The PIN is displayed ONCE on the screen – write it down!
7. The account is saved in "customers.dat" and the branch statistics are updated.

6. CUSTOMER LOGIN AND TRANSACTIONS
-----------------------------------
After a customer account has been created, the customer can log in:

   a) From the main menu, choose option 3. "Customer Login"
   b) Enter the Account Number (e.g., ACC-B001-1001)
   c) Enter the 5‑digit PIN provided at registration.
   d) If the PIN is correct, the customer menu appears.

Customer menu options:
   1. View Balance – shows current balance and account details.
   2. Deposit – add money to the account (logged in transactions.dat).
   3. Withdraw – take money out (checks for sufficient funds/overdraft).
   4. Transfer – send money to another account (requires recipient account number).
   5. Statement – shows last 10 transactions for this account.
   6. Change PIN – allows the customer to set a new 5‑digit PIN.
   7. Logout – returns to the main menu.

Security features:
   - After 3 failed PIN attempts, the account is temporarily locked (in‑memory only).
   - PINs are stored encrypted (simple XOR) in the binary file.

7. TELLER-ASSISTED TRANSACTIONS
--------------------------------
Tellers can also process deposits/withdrawals on behalf of a customer:
   - Log in as teller.
   - Choose "2. Assisted Transaction".
   - Enter the customer's account number and PIN (the teller must verify the PIN).
   - Select Deposit or Withdrawal.
   - The transaction is logged and balance updated.

8. ADDITIONAL FEATURES (MAIN MENU)
-----------------------------------
4. View All Branches – lists all branch codes and names.
5. Inter‑Branch Comparison – shows which branch has the highest total deposits.
6. Apply Interest – adds interest to all Savings (2.5%) and Fixed Deposit (5.0%) accounts.
7. Search Customer – find a customer by account number or name.
8. Daily Transaction Report – shows all transactions made today.
9. Customer Summary – lists all customers with balances.
10. Branch Performance – detailed stats for each branch.
11. Backup Data – copies all .dat files to *_backup.dat.
12. Recover Data – restores data from the backup files.
13. Export CSV – creates "bank_export.csv" with all customer details.
14. Exit – closes the program.

9. FILES CREATED BY THE SYSTEM
-------------------------------
customers.dat      – binary file containing all customer records.
transactions.dat   – binary file with all financial transactions.
branches.dat       – binary file with branch information.
tellers.dat        – binary file storing teller credentials (hashed passwords).
system_config.txt  – plain text file for interest rates and settings.
bank_export.csv    – CSV export of customer data (created when option 13 is used).

10. TESTING TIPS
-----------------
- To quickly test the system, first create a teller (e.g., T001 / pass123 / B001).
- Log in as that teller and register a Savings account with minimal deposit.
- Note the generated account number and PIN.
- Log out, then log in as the customer using those credentials.
- Perform a deposit, withdrawal, and view the statement.
- Use the main menu to view reports and export data.

================================================================================
                                END OF README
================================================================================