#pragma once

#include <Arduino.h>
#include <SPIFFS.h>

/*
*   Common used global functions grouped together as utilities
*/

namespace Utils {
    void printRAM();
    
    String generateUUID();

    String getParentDirectory(const String &folder);
    bool removeRecursive(fs::FS &fs, File &file);
    bool removeRecursive(fs::FS &fs, const String &path);
    bool ensureDirectory(fs::FS &fs, const String &path);
}