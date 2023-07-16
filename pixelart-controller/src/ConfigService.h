#pragma once

#include <Config.h>
#include <Version.h>
#include <ArduinoJson.h>

class ConfigService {
    private:
        Config::Config _settings;
        void saveConfig(Config::Config config, fs::FS &fs, String path);
        bool loadConfig(fs::FS &fs, String path);
        void loadPreferences();
        void migrateConfig();
        void migratePreferences(Version &preferences_version);
        void loadPreference(const char *key, String& setting);
        void loadPreference(const char *key, bool& setting);
        void loadPreference(const char *key, uint& setting);
        void loadPreference(const char *key, uint8_t& setting);
        template<typename SettingType>
        void loadEnumPreference(const char *key, SettingType& setting);
    public:
        Config::Config getSettings() const { return _settings; }
        void init();
};