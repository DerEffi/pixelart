#include <main.h>

#include <Arduino.h>
#include <SPIFFS.h>

namespace Utils {

    // Send current RAM stats to Serial interface
    void printRAM() {
        LOG("Debug", "RAM: %u.%03u - PSRAM: %u.%03u.%03u", ESP.getFreeHeap() / 1000, ESP.getFreeHeap() % 1000, ESP.getFreePsram() / 1000000, (ESP.getFreePsram() / 1000) % 1000, ESP.getFreePsram() % 1000);
    }

    // Generate new pseudo-random UUID
    // @return UUIDv4
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

    // Searches for parent directory of given path
    // @param directory path to search
    // @return parent directoryr path to search String
    String getParentDirectory(String directory) {
        std::string directory_str(directory.c_str());
        size_t found = directory_str.find_last_of("/\\");
        return found > 0 ? String(directory_str.substr(0, found).c_str()) : String("/");
    }

    // removes files and directorys with all their sub-elements
    // @param fs Filesystem (i.e. `SD` or `SPIFFS`)
    // @param file filesystem element handle
    // @return success
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

    // removes files and directorys with all their sub-elements
    // @param fs Filesystem (i.e. `SD` or `SPIFFS`)
    // @param file path to element on that filesystem
    // @return success
    bool removeRecursive(fs::FS &fs, String path) {
        if(fs.exists(path)) {
            File file = fs.open(path);
            return removeRecursive(fs, file);		
        }

        return false;
    }

    // ensures that a directory on a given path is available
    // if not creates a directory with all necessary parent directories
    // files present on given path will be deleted and replaced with a directory
    // @param fs Filesystem (i.e. `SD` or `SPIFFS`)
    // @param path to the wanted directory
    // @return success
    bool ensureDirectory(fs::FS &fs, String path) {
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
            String parent = getParentDirectory(path);
            if(parent.length() > 1) {
                ensureDirectory(fs, parent);
            }
            return fs.mkdir(path);
        }

        return false;
    }
}