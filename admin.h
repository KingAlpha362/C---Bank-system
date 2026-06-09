#pragma once

// ---------------------------------------------------------------------------
// admin.h
// System-level administration: first-run setup, interest run, backup/recover,
// data export and system-log viewing.
// ---------------------------------------------------------------------------

void init_system();
void apply_interest();
void backup_data();
void recover_data();
void export_data();
void view_system_logs();
