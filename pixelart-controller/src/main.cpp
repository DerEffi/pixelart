#include <main.h>

#include <Arduino.h>
#include <Config.h>
#include <ConfigService.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Font4x5Fixed.h>
#include <Font4x7Fixed.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <SD.h>
#include <Update.h>
#include <Preferences.h>

#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncHTTPSRequest_Generic.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

#include <time.h>
#include <Wire.h>
#include <RTClib.h>
#include <ESP32Time.h>

ConfigService config;

void print_ram() {
    uint heap = ESP.getFreeHeap();
    uint psram = ESP.getFreePsram();
    LOG("Debug", "RAM: %u.%03u - PSRAM: %u.%03u.%03u", heap / 1000, heap % 1000, psram / 1000000, (psram / 1000) % 1000, psram % 1000);
}

void setup() {
    STARTLOG(9600);

    print_ram();

    config.Init();

    print_ram();
}

void loop() {
    while(true) {
        delay(2000);
        print_ram();
    }
}