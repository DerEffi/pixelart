#include <main.h>

#include <Arduino.h>
#include <SPIFFS.h>

namespace Utils {

    void printRAM() {
        LOG("Debug", "RAM: %u.%03u - PSRAM: %u.%03u.%03u", ESP.getFreeHeap() / 1000, ESP.getFreeHeap() % 1000, ESP.getFreePsram() / 1000000, (ESP.getFreePsram() / 1000) % 1000, ESP.getFreePsram() % 1000);
    }

    String generateUUID() {
        const char possible[] = "0123456789abcdef";
        char uuid[37];
        for(int p = 0, i = 0; i < 36; i++){
            int r = random(0, 16);
            uuid[p++] = possible[r];
        }

        uuid[8] = '-';
        uuid[13] = '-';
        uuid[14] = '4';
        uuid[18] = '-';
        uuid[19] = possible[random(8, 12)];
        uuid[23] = '-';
        uuid[36] = '\0';
        
        return String(uuid);
    }

    String getParentFolder(String folder) {
        std::string folder_str(folder.c_str());
        size_t found = folder_str.find_last_of("/\\");
        return found > 0 ? String(folder_str.substr(0, found).c_str()) : String("/");
    }

    bool removeRecursive(fs::FS &fs, File &file) {
        if(file) {
            String path(file.path());

            if(file.isDirectory()) {

                File nested_file = file.openNextFile();
                while(nested_file) {
                    if(!removeRecursive(fs, nested_file)) {
                        return false;
                    }
                    nested_file = file.openNextFile();
                }

                file.close();
                return fs.rmdir(path);
            } else {
                file.close();
                return fs.remove(path);
            }
        }

        return false;
    }

    bool removeRecursive(fs::FS &fs, String path) {
        if(fs.exists(path)) {
            File file = fs.open(path);
            return removeRecursive(fs, file);		
        }

        return false;
    }

    bool ensureFolder(fs::FS &fs, String path) {
        if(fs.exists(path)) {
            File path_object = fs.open(path);
            if(path_object) {
                bool isDirectory = path_object.isDirectory();
                path_object.close();
                if(!isDirectory) {
                    if(fs.remove(path)) {
                        return fs.mkdir(path);
                    }
                } else {
                    return true;
                }
            }
        } else {
            String parent = getParentFolder(path);
            if(parent.length() > 1) {
                ensureFolder(fs, parent);
            }
            return fs.mkdir(path);
        }

        return false;
    }
}