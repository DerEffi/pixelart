#include <ConfigService.h>

#include <main.h>
#include <Arduino.h>
#include <Config.h>

Config _settings;

void ConfigService::Init() {
    LOG("Info", "Reading Configuration");
}