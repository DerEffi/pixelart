#pragma once

#include <Arduino.h>
#include <SPIFFS.h>

namespace Utils {
    void printRAM();
    String generateUUID();
    String getParentFolder(String folder);
    bool removeRecursive(fs::FS &fs, File &file);
    bool removeRecursive(fs::FS &fs, String path);
    bool ensureFolder(fs::FS &fs, String path);
}