#pragma once

// ---------------------------------------------------------------------------
// branch.h
// Branch data storage and branch-level admin/reporting.
// ---------------------------------------------------------------------------

#include <string>
#include <vector>
#include "records.h"

// Storage
std::vector<BranchRecord> load_branches();
bool save_branches(std::vector<BranchRecord>& branches);
void update_branch_stats(const std::string& branch_code, double balance_change, int customer_change);

// Admin
void add_branch();
void remove_branch();

// Views / reports
void view_all_branches();
void view_branch_details();
void compare_branches();
// branch_filter (when non-empty) reports on that single branch only.
void branch_performance_report(const std::string& branch_filter = "");
