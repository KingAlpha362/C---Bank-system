![Multi-Branch Banking Simulation — project cover](https://raw.githubusercontent.com/KingAlpha362/KingAlpha362/main/assets/covers/programming.png)

# Multi-Branch Banking Simulation

A modular C++ console project modelling branches, customers, tellers, accounts, and transactions.

## ✨ Highlights

- Separate modules for account rules, customer records, branches, tellers, validation, and menus.
- File-backed records, transaction reports, and administrative utilities.
- An accompanying original guide in `README.txt`.

## 🚀 Getting started

With a C++17 compiler installed, run from the repository root:

```sh
g++ -std=c++17 *.cpp -o bank.exe
./bank.exe
```

On Windows PowerShell, launch `.\bank.exe`. Alternatively, use the supplied `Makefile`. Read [README.txt](README.txt) for the original walkthrough and sample login details.

## 🗂️ Repository guide

- [Makefile](Makefile)
- [accounts.cpp](accounts.cpp)
- [accounts.h](accounts.h)
- [admin.cpp](admin.cpp)
- [admin.h](admin.h)
- [branch.cpp](branch.cpp)
- [branch.h](branch.h)
- [customer.cpp](customer.cpp)

## 📝 Project notes

An educational banking simulation, not production financial software. The original guide describes XOR-based PIN obfuscation and test credentials; these do not provide production-grade authentication. Use fictional records.

---

[← Portfolio](https://github.com/KingAlpha362/KingAlpha362) · [Browse all projects](https://github.com/KingAlpha362/KingAlpha362/blob/main/PORTFOLIO.md)
