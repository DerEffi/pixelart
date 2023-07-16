#include <main.h>
#include <ConfigService.h>

#include <Arduino.h>
#include <Preferences.h>
#include <SPIFFS.h>

#include <Config.h>

Preferences preferences;
Config::Config _settings;

//load factory setting from spiffs
//@return success
bool ConfigService::loadConfig() {
    LOG("Info", "Reading config file");

    try {

        if(SPIFFS.begin() && SPIFFS.exists(Config::Path_Config)) {
            File configFile = SPIFFS.open(Config::Path_Config);
            if(!configFile.isDirectory()) {
                //todo read config file
            }
        }
    } catch(...) {
        LOG("Error", "Could not load config file");
    }

    SPIFFS.end();
    return false;
}

// load single preference (String) into settings member
// @param setting String
void ConfigService::loadPreference(const char *key, String& setting) {
    setting = preferences.getString(key, setting);
}
// load single preference (bool) into settings member
void ConfigService::loadPreference(const char *key, bool& setting) {
    setting = preferences.getBool(key, setting);
}
// load single preference (uint) into settings member
void ConfigService::loadPreference(const char *key, uint& setting) {
    setting = preferences.getUInt(key, setting);
}
// load single preference (uint8) into settings member
void ConfigService::loadPreference(const char *key, uint8_t& setting) {
    setting = preferences.getUChar(key, setting);
}
// load single preference (enum) into settings member
template<typename SettingType>
void ConfigService::loadEnumPreference(const char *key, SettingType& setting) {
    setting = static_cast<SettingType>preferences.getUChar(key, 0);
}

// load user preferences from nvm
bool ConfigService::loadPreferences() {
    LOG("Info", "Loading user preferences");

    try {
        preferences.begin(_settings.firmware.preferences.c_str());

        String version;
        this->loadPreference(Config::P_Firmware_Version, version);
        //todo compare versions
        //return false;

        this->loadPreference(Config::P_Display_Brightness, _settings.display.brightness);
        this->loadEnumPreference(Config::P_Display_Mode, _settings.display.mode);
        this->loadPreference(Config::P_Display_Position, _settings.display.position);
        this->loadEnumPreference(Config::P_Display_Clock_Mode, _settings.display.clock.mode);
        this->loadPreference(Config::P_Display_Socials_Position, _settings.display.socials.position);
        this->loadPreference(Config::P_Display_Diashow_Enabled, _settings.display.diashow.enabled);
        this->loadPreference(Config::P_Display_Diashow_Interval, _settings.display.diashow.interval);
        this->loadPreference(Config::P_Display_Diashow_SwitchModes, _settings.display.diashow.switchModes);
        this->loadPreference(Config::P_Display_Animation_Enabled, _settings.display.animation.enabled);
        this->loadPreference(Config::P_Display_Animation_Interval, _settings.display.animation.interval);

        this->loadPreference(Config::P_Socials_Enabled, _settings.socials.enabled);
        this->loadPreference(Config::P_Socials_Interval, _settings.socials.interval);
        this->loadPreference(Config::P_Socials_Server, _settings.socials.server);
        this->loadPreference(Config::P_Socials_Key, _settings.socials.key);

        this->loadPreference(Config::P_Server_Key, _settings.server.key);

        this->loadPreference(Config::P_Time_ShowSeconds, _settings.time.showSeconds);
        this->loadPreference(Config::P_Time_ShowYear, _settings.time.showYear);
        this->loadPreference(Config::P_Time_Blink, _settings.time.blink);
        this->loadPreference(Config::P_Time_Format24h, _settings.time.format24h);
        this->loadPreference(Config::P_Time_Update, _settings.time.update);
        this->loadPreference(Config::P_Time_Server, _settings.time.server);
        this->loadPreference(Config::P_Time_Zone, _settings.time.zone);

        this->loadPreference(Config::P_WiFi_Hostname, _settings.wifi.hostname);
        this->loadPreference(Config::P_WiFi_Host_Enabled, _settings.wifi.host.enabled);
        this->loadPreference(Config::P_WiFi_Host_SSID, _settings.wifi.host.ssid);
        this->loadPreference(Config::P_WiFi_Host_Password, _settings.wifi.host.password);
        this->loadPreference(Config::P_WiFi_Connect_Enabled, _settings.wifi.connect.enabled);
        this->loadPreference(Config::P_WiFi_Connect_SSID, _settings.wifi.connect.ssid);
        this->loadPreference(Config::P_WiFi_Connect_Password, _settings.wifi.connect.password);

        preferences.putString(Config::P_Firmware_Version, _settings.firmware.version);

        preferences.end();
    } catch (...) {
        LOG("Error", "Could not load user preferences");
        return false;
    }

    return true;
}

// enabled backwards compatibility with older versions of this firmware or unused devices
void ConfigService::migrate() {

}

// load configuration from filesystem (faactory settings) and user preferences
void ConfigService::init() {
    LOG("Info", "Starting configuration");

    bool configLoaded = this->loadConfig();
    bool preferencesLoaded = this->loadPreferences();

    //todo migration
}