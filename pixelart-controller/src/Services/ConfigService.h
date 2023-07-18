#pragma once

#include <Models/Config.h>
#include <Models/Version.h>
#include <ArduinoJson.h>

/*
*   Configuration Service for managing
*       - Preferences in nvm
*       - Factory Settings in SPIFFS
*       - Current Application settings/state for other services
*/

class ConfigService {
    private:
        Config::Config _settings;
        
        void saveConfig(Config::Config& config, fs::FS& fs, String path);
        bool loadConfig(fs::FS& fs, String path);
        void loadPreferences();
        
        void saveDisplay();
        void saveSocials();
        void saveServer();
        void saveTime();
        void saveWifi();
        
        void migrateConfig();
        void migratePreferences(Version& preferences_version);

        void loadPreference(const char *key, String& setting);
        void loadPreference(const char *key, bool& setting);
        void loadPreference(const char *key, uint& setting);
        void loadPreference(const char *key, uint8_t& setting);
        template<typename SettingType>
        void loadEnumPreference(const char *key, SettingType& setting);
        void loadBlobPreference(const char *key, String& setting);

        void savePreference(const char *key, const String& setting);
        void savePreference(const char *key, const bool& setting);
        void savePreference(const char *key, const uint& setting);
        void savePreference(const char *key, const uint8_t& setting);
        template<typename SettingType>
        void saveEnumPreference(const char *key, const SettingType& setting);
        void saveBlobPreference(const char *key, const String& setting);
    public:
        Config::Config getSettings() const { return _settings; }

        void init();

        void updateDisplay(Config::DisplayConfig config);
        void updateSocials(Config::SocialsConfig config);
        void updateServer(Config::ServerConfig config);
        void updateTime(Config::TimeConfig config);
        void updateWifi(Config::WiFiConfig config);
};
