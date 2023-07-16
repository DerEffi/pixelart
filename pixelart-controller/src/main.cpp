#include <main.h>

// common libs
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <WiFi.h>

// project
#include <Config.h>
#include <ConfigService.h>
#include <Utils.h>


ConfigService config;

void setup() {
    STARTLOG(9600);

    config.init();
}

void loop() {
    while(true) {
        delay(2000);
    }
}