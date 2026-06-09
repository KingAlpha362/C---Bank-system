// ---------------------------------------------------------------------------
// main.cpp
// Entry point for the Multi-Branch Banking Management System.
// Performs first-run setup, then hands control to the main menu loop.
// ---------------------------------------------------------------------------

#include "admin.h"   // init_system
#include "menus.h"   // main_menu

int main() {
    init_system();
    main_menu();
    return 0;
}
