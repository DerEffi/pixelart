#pragma once

#include <Arduino.h>
#include <SPIFFS.h>

/*
*   Common used global functions grouped together as utilities
*/

namespace Utils {
    void printRAM();
    
    String generateUUID();

    String getParentDirectory(String folder);
    bool removeRecursive(fs::FS &fs, File &file);
    bool removeRecursive(fs::FS &fs, String path);
    bool ensureDirectory(fs::FS &fs, String path);
}