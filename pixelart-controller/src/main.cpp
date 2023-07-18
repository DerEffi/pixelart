#include <main.h>

// common libs
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <WiFi.h>

// project
#include <Models/Config.h>
#include <Services/ConfigService.h>
#include <Services/GPIOService.h>
#include <Utils.h>


ConfigService config;
GPIOService gpio;

void setup() {
    STARTLOG(9600);

    config.init();
    gpio.init(config.getSettings().gpio);
}

void loop() {
    while(true) {
        delay(2000);
        Utils::printRAM();
    }
}