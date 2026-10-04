// Classic99 for ESP32 - F12 on-screen menu
#pragma once
#include "fabgl.h"

// pauses emulation, runs the menu on the VGA screen, resumes. Call from the input task.
void menuRun(fabgl::VGAController *display, fabgl::Keyboard *keyboard);

// last used cartridge and disks, stored in TI_ROOT/config.txt
void configLoad();
void configSave();

// the saved "Debug log" setting, readable before the rest of the configuration is applied
bool configDebugEnabled();
