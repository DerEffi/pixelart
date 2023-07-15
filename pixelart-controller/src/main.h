#ifndef MAIN_H_
#define MAIN_H_

#define DEBUG true

#define VERSION "1.0.0"
#define PREFERENCES "pixelart"



/***********
*  Macros  *
************/

#if DEBUG
    #define STARTLOG(baudrate) {            \
        Serial.begin(baudrate);             \
        delay(5000);                        \
    }
    #define LOG(level, format, ...) {               \
        Serial.printf("%06u.", millis() / 1000);    \
        Serial.printf("%03u ", millis() % 1000);    \
        Serial.printf("%5s -> ", level);            \
        Serial.printf(format, ##__VA_ARGS__);       \
        Serial.println();                           \
    }
#else
    #define STARTLOG
    #define LOG
#endif

#endif