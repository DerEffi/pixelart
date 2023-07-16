#pragma once

#include <Config.h>

class ConfigService {
    private:
        Config::Config _settings;
        bool loadConfig();
        bool loadPreferences();
        void migrate();
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