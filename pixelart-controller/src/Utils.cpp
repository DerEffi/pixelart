#include <main.h>

#include <Arduino.h>

namespace Utils {

    void print_ram() {
        LOG("Debug", "RAM: %u.%03u - PSRAM: %u.%03u.%03u", ESP.getFreeHeap() / 1000, ESP.getFreeHeap() % 1000, ESP.getFreePsram() / 1000000, (ESP.getFreePsram() / 1000) % 1000, ESP.getFreePsram() % 1000);
    }

    String generate_uuid() {
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

}