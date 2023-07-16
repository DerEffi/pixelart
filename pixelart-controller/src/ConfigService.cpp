#include <main.h>
#include <ConfigService.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <SPIFFS.h>

#include <Config.h>
#include <Version.h>

Preferences preferences;
Config::Config _settings;

//save settings to file
void saveConfig(Config::Config config, fs::FS &fs, String path) {
    File configFile = fs.open(path, "w");
    if(configFile) {
        if(configFile.isDirectory()) {
            Utils::removeRecursive(fs, configFile);
            File configFile = fs.open(path, "w");
        }

        DynamicJsonDocument config(8192);
        //todo settings to json
        serializeJson(config, configFile);
        configFile.close();
    }
}

// load settings from file
// @return success
bool ConfigService::loadConfig(fs::FS &fs, String path) {
    LOG("Info", "Reading config file");

    try {

        if(fs.exists(path)) {
            File configFile = fs.open(path, "r");
            if(configFile && !configFile.isDirectory()) {
                DynamicJsonDocument config(8192);
                DeserializationError error = deserializeJson(config, configFile);

                if(error == DeserializationError::Ok) {
                    if(config.containsKey("gpio") && config["gpio"].is<JsonObject>()) {
                        JsonObject gpio = config["gpio"].as<JsonObject>();

                        if(gpio.containsKey("i2c") && gpio["i2c"].is<JsonObject>()) {
                            JsonObject i2c = gpio["i2c"].as<JsonObject>();

                            if(i2c.containsKey("sda") && i2c["sda"].is<signed char>())
                                _settings.gpio.i2c.sda = i2c["sda"].as<signed char>();
                            if(i2c.containsKey("clk") && i2c["clk"].is<signed char>())
                                _settings.gpio.i2c.clk = i2c["clk"].as<signed char>();
                        }
                        if(gpio.containsKey("spi") && gpio["spi"].is<JsonObject>()) {
                            JsonObject spi = gpio["spi"].as<JsonObject>();

                            if(spi.containsKey("clk") && spi["clk"].is<signed char>())
                                _settings.gpio.spi.clk = spi["clk"].as<signed char>();
                            if(spi.containsKey("mosi") && spi["mosi"].is<signed char>())
                                _settings.gpio.spi.mosi = spi["mosi"].as<signed char>();
                            if(spi.containsKey("miso") && spi["miso"].is<signed char>())
                                _settings.gpio.spi.miso = spi["miso"].as<signed char>();
                        }
                        if(gpio.containsKey("sd") && gpio["sd"].is<JsonObject>()) {
                            JsonObject sd = gpio["sd"].as<JsonObject>();

                            if(sd.containsKey("cs") && sd["cs"].is<signed char>())
                                _settings.gpio.sd.cs = sd["cs"].as<signed char>();
                        }
                        if(gpio.containsKey("rtc") && gpio["rtc"].is<JsonObject>()) {
                            JsonObject rtc = gpio["rtc"].as<JsonObject>();

                            if(rtc.containsKey("address") && rtc["address"].is<signed char>())
                                _settings.gpio.rtc.address = rtc["address"].as<signed char>();
                        }
                        if(gpio.containsKey("display") && gpio["display"].is<JsonObject>()) {
                            JsonObject display = gpio["display"].as<JsonObject>();

                            if(display.containsKey("r1") && display["r1"].is<signed char>())
                                _settings.gpio.display.r1 = display["r1"].as<signed char>();
                            if(display.containsKey("g1") && display["g1"].is<signed char>())
                                _settings.gpio.display.g1 = display["g1"].as<signed char>();
                            if(display.containsKey("b1") && display["b1"].is<signed char>())
                                _settings.gpio.display.b1 = display["b1"].as<signed char>();
                            if(display.containsKey("r2") && display["r2"].is<signed char>())
                                _settings.gpio.display.r2 = display["r2"].as<signed char>();
                            if(display.containsKey("g2") && display["g2"].is<signed char>())
                                _settings.gpio.display.g2 = display["g2"].as<signed char>();
                            if(display.containsKey("b2") && display["b2"].is<signed char>())
                                _settings.gpio.display.b2 = display["b2"].as<signed char>();
                            if(display.containsKey("a") && display["a"].is<signed char>())
                                _settings.gpio.display.a = display["a"].as<signed char>();
                            if(display.containsKey("b") && display["b"].is<signed char>())
                                _settings.gpio.display.b = display["b"].as<signed char>();
                            if(display.containsKey("c") && display["c"].is<signed char>())
                                _settings.gpio.display.c = display["c"].as<signed char>();
                            if(display.containsKey("d") && display["d"].is<signed char>())
                                _settings.gpio.display.d = display["d"].as<signed char>();
                            if(display.containsKey("e") && display["e"].is<signed char>())
                                _settings.gpio.display.e = display["e"].as<signed char>();
                            if(display.containsKey("clk") && display["clk"].is<signed char>())
                                _settings.gpio.display.clk = display["clk"].as<signed char>();
                            if(display.containsKey("lat") && display["lat"].is<signed char>())
                                _settings.gpio.display.lat = display["lat"].as<signed char>();
                            if(display.containsKey("oe") && display["oe"].is<signed char>())
                                _settings.gpio.display.oe = display["oe"].as<signed char>();
                        }
                        if(gpio.containsKey("controls") && gpio["controls"].is<JsonObject>()) {
                            JsonObject controls = gpio["controls"].as<JsonObject>();

                            if(controls.containsKey("softwareDebounce") && controls["softwareDebounce"].is<bool>())
                                _settings.gpio.controls.softwareDebounce = controls["softwareDebounce"].as<bool>();                        
                            if(controls.containsKey("expander") && controls["expander"].is<JsonObject>()) {
                                JsonObject expander = controls["expander"].as<JsonObject>();

                                if(expander.containsKey("enabled") && expander["enabled"].is<bool>())
                                    _settings.gpio.controls.expander.enabled = expander["enabled"].as<bool>();
                                if(expander.containsKey("interrupt") && expander["interrupt"].is<signed char>())
                                    _settings.gpio.controls.expander.interrupt = expander["interrupt"].as<signed char>();
                                if(expander.containsKey("cs") && expander["cs"].is<signed char>())
                                    _settings.gpio.controls.expander.cs = expander["cs"].as<signed char>();
                                if(expander.containsKey("address") && expander["address"].is<signed char>())
                                    _settings.gpio.controls.expander.address = expander["address"].as<signed char>();
                            }
                            if(controls.containsKey("btn1") && controls["btn1"].is<signed char>())
                                _settings.gpio.controls.btn1 = controls["btn1"].as<signed char>();
                            if(controls.containsKey("btn2") && controls["btn2"].is<signed char>())
                                _settings.gpio.controls.btn2 = controls["btn2"].as<signed char>();
                            if(controls.containsKey("btn3") && controls["btn3"].is<signed char>())
                                _settings.gpio.controls.btn3 = controls["btn3"].as<signed char>();
                            if(controls.containsKey("rot2") && controls["rot2"].is<JsonObject>()) {
                                JsonObject rot2 = controls["rot2"].as<JsonObject>();

                                if(rot2.containsKey("btn") && rot2["btn"].is<signed char>())
                                    _settings.gpio.controls.rot2.btn = rot2["btn"].as<signed char>();
                                if(rot2.containsKey("a") && rot2["a"].is<signed char>())
                                    _settings.gpio.controls.rot2.a = rot2["a"].as<signed char>();
                                if(rot2.containsKey("b") && rot2["b"].is<signed char>())
                                    _settings.gpio.controls.rot2.b = rot2["b"].as<signed char>();
                            }
                            if(controls.containsKey("rot2") && controls["rot2"].is<JsonObject>()) {
                                JsonObject rot2 = controls["rot2"].as<JsonObject>();

                                if(rot2.containsKey("btn") && rot2["btn"].is<signed char>())
                                    _settings.gpio.controls.rot2.btn = rot2["btn"].as<signed char>();
                                if(rot2.containsKey("a") && rot2["a"].is<signed char>())
                                    _settings.gpio.controls.rot2.a = rot2["a"].as<signed char>();
                                if(rot2.containsKey("b") && rot2["b"].is<signed char>())
                                    _settings.gpio.controls.rot2.b = rot2["b"].as<signed char>();
                            }
                            if(controls.containsKey("rot3") && controls["rot3"].is<JsonObject>()) {
                                JsonObject rot3 = controls["rot3"].as<JsonObject>();

                                if(rot3.containsKey("btn") && rot3["btn"].is<signed char>())
                                    _settings.gpio.controls.rot3.btn = rot3["btn"].as<signed char>();
                                if(rot3.containsKey("a") && rot3["a"].is<signed char>())
                                    _settings.gpio.controls.rot3.a = rot3["a"].as<signed char>();
                                if(rot3.containsKey("b") && rot3["b"].is<signed char>())
                                    _settings.gpio.controls.rot3.b = rot3["b"].as<signed char>();
                            }
                        }
                    }

                    if(config.containsKey("wifi") && config["wifi"].is<JsonObject>()) {
                        JsonObject wifi = config["wifi"].as<JsonObject>();

                        if(wifi.containsKey("hostname") && wifi["hostname"].is<const char *>())
                            _settings.wifi.hostname = wifi["hostname"].as<String>();
                        if(wifi.containsKey("connect") && wifi["connect"].is<JsonObject>()) {
                            JsonObject connect = wifi["connect"].as<JsonObject>();

                            if(connect.containsKey("enabled") && connect["enabled"].is<bool>())
                                _settings.wifi.connect.enabled = connect["enabled"].as<bool>();
                            if(connect.containsKey("ssid") && connect["ssid"].is<const char *>())
                                _settings.wifi.connect.ssid = connect["ssid"].as<String>();
                            if(connect.containsKey("password") && connect["password"].is<const char *>())
                                _settings.wifi.connect.password = connect["password"].as<String>();
                        }
                        if(wifi.containsKey("host") && wifi["host"].is<JsonObject>()) {
                            JsonObject host = wifi["host"].as<JsonObject>();

                            if(host.containsKey("enabled") && host["enabled"].is<bool>())
                                _settings.wifi.host.enabled = host["enabled"].as<bool>();
                            if(host.containsKey("ssid") && host["ssid"].is<const char *>())
                                _settings.wifi.host.ssid = host["ssid"].as<String>();
                            if(host.containsKey("password") && host["password"].is<const char *>())
                                _settings.wifi.host.password = host["password"].as<String>();
                        }
                    }

                    if(config.containsKey("time") && config["time"].is<JsonObject>()) {
                        JsonObject time = config["time"].as<JsonObject>();

                        if(time.containsKey("showSeconds") && time["showSeconds"].is<bool>())
                            _settings.time.showSeconds = time["showSeconds"].as<bool>();
                        if(time.containsKey("blink") && time["blink"].is<bool>())
                            _settings.time.blink = time["blink"].as<bool>();
                        if(time.containsKey("showYear") && time["showYear"].is<bool>())
                            _settings.time.showYear = time["showYear"].as<bool>();
                        if(time.containsKey("format24h") && time["format24h"].is<bool>())
                            _settings.time.format24h = time["format24h"].as<bool>();
                        if(time.containsKey("update") && time["update"].is<bool>())
                            _settings.time.update = time["update"].as<bool>();
                        if(time.containsKey("server") && time["server"].is<const char *>())
                            _settings.time.server = time["server"].as<String>();
                        if(time.containsKey("zone") && time["zone"].is<const char *>())
                            _settings.time.zone = time["zone"].as<String>();
                    }

                    if(config.containsKey("socials") && config["socials"].is<JsonObject>()) {
                        JsonObject socials = config["socials"].as<JsonObject>();

                        if(socials.containsKey("enabled") && socials["enabled"].is<bool>())
                            _settings.socials.enabled = socials["enabled"].as<bool>();
                        if(socials.containsKey("interval") && socials["interval"].is<unsigned int>())
                            _settings.socials.interval = socials["interval"].as<unsigned int>();
                        if(socials.containsKey("key") && socials["key"].is<const char *>())
                            _settings.socials.key = socials["key"].as<String>();
                        if(socials.containsKey("key") && socials["key"].is<const char *>())
                            _settings.socials.key = socials["key"].as<String>();
                        if(socials.containsKey("server") && socials["server"].is<const char *>())
                            _settings.socials.server = socials["server"].as<String>();
                        if(socials.containsKey("request") && socials["request"].is<const char *>())
                            _settings.socials.request = socials["request"].as<String>();
                    }

                    if(config.containsKey("display") && config["display"].is<JsonObject>()) {
                        JsonObject display = config["display"].as<JsonObject>();

                        if(display.containsKey("brightness") && display["brightness"].is<unsigned char>())
                            _settings.display.brightness = display["brightness"].as<unsigned char>();
                        if(display.containsKey("mode") && display["mode"].is<unsigned char>())
                            _settings.display.mode = static_cast<Config::DisplayMode>(display["mode"].as<unsigned char>() % Config::DisplayModeCount);
                        if(display.containsKey("clock") && display["clock"].is<JsonObject>()) {
                            JsonObject clock = display["clock"].as<JsonObject>();

                            if(clock.containsKey("mode") && clock["mode"].is<unsigned char>())
                                _settings.display.clock.mode = static_cast<Config::ClockMode>(clock["mode"].as<unsigned char>() % Config::ClockModeCount);
                        }
                        if(display.containsKey("diashow") && display["diashow"].is<JsonObject>()) {
                            JsonObject diashow = display["diashow"].as<JsonObject>();

                            if(diashow.containsKey("enabled") && diashow["enabled"].is<bool>())
                                _settings.display.diashow.enabled = diashow["enabled"].as<bool>();
                            if(diashow.containsKey("interval") && diashow["interval"].is<unsigned int>())
                                _settings.display.diashow.interval = diashow["interval"].as<unsigned int>();
                            if(diashow.containsKey("switchModes") && diashow["switchModes"].is<bool>())
                                _settings.display.diashow.switchModes = diashow["switchModes"].as<bool>();
                        }
                        if(display.containsKey("animation") && display["animation"].is<JsonObject>()) {
                            JsonObject animation = display["animation"].as<JsonObject>();

                            if(animation.containsKey("enabled") && animation["enabled"].is<bool>())
                                _settings.display.animation.enabled = animation["enabled"].as<bool>();
                            if(animation.containsKey("interval") && animation["interval"].is<unsigned int>())
                                _settings.display.animation.interval = animation["interval"].as<unsigned int>();
                        }
                    }

                    // overwrite json file with minified version
                    if(configFile.size() > config.memoryUsage() + 200) {
                        configFile.close();
                        configFile = fs.open(path, "w");
                        serializeJson(config, configFile);
                    }
                    
                    configFile.close();
                    return true;
                } else {
                    LOG("Error", "Error reading json content - code: %u", error);
                }
                    
                configFile.close();
                return false;
            } else {
                Utils::removeRecursive(fs, configFile);
            }
            configFile.close();
        }

    } catch(...) {
        LOG("Error", "Could not load config file");
    }

    LOG("Warn", "Config file either not present or corrupted");
    return false;
}

// load single preference (String) into settings member
// @param setting String
void ConfigService::loadPreference(const char *key, String& setting) {
    if(preferences.isKey(key))
        setting = preferences.getString(key, setting);
}
// load single preference (bool) into settings member
// @param setting bool
void ConfigService::loadPreference(const char *key, bool& setting) {
    if(preferences.isKey(key))
        setting = preferences.getBool(key, setting);
}
// load single preference (uint) into settings member
// @param setting uint
void ConfigService::loadPreference(const char *key, uint& setting) {
    if(preferences.isKey(key))
        setting = preferences.getUInt(key, setting);
}
// load single preference (uint8) into settings member
// @param setting uint8_t
void ConfigService::loadPreference(const char *key, uint8_t& setting) {
    if(preferences.isKey(key))
        setting = preferences.getUChar(key, setting);
}
// load single preference (enum) into settings member
// @param setting Enum
template<typename SettingType>
void ConfigService::loadEnumPreference(const char *key, SettingType& setting) {
    if(preferences.isKey(key))
        setting = static_cast<SettingType>(preferences.getUChar(key, 0));
}

// load user preferences from nvm
void ConfigService::loadPreferences() {
    LOG("Info", "Loading user preferences");

    try {
        preferences.begin(_settings.firmware.preferences.c_str(), true);

        // checking for older version
        String version = "0";
        this->loadPreference(Config::P_Firmware_Version, version);
        Version pref_version(version);
        Version firm_version(_settings.firmware.version);
        if(pref_version < firm_version) {
            preferences.end();
            this->migratePreferences(pref_version);
            preferences.begin(_settings.firmware.preferences.c_str(), true);
        }
        
        //loading preferences
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

        preferences.end();
    } catch (...) {
        LOG("Error", "Could not load user preferences");
    }
}

void ConfigService::migrateConfig() {
    LOG("Info", "Migrating missing config file from older version or unused device");
    
    if(SPIFFS.begin()) {
        const char wifi_conf[] = "/wifi.conf";
        const char socials_conf[] = "/socials.conf";

        if(SPIFFS.exists(wifi_conf)) {
            File file = SPIFFS.open(wifi_conf);
            if(file && !file.isDirectory()) {
                size_t filesize = file.size();
                if(filesize > 1) {
                    char buffer[filesize];
                    file.readBytes(buffer, filesize);
                    buffer[filesize] = '\0';
                    _settings.wifi.hostname = buffer;
                }
            }
            Utils::removeRecursive(SPIFFS, file);
        }
        if(SPIFFS.exists(socials_conf)) {
            File file = SPIFFS.open(socials_conf);
            if(file && !file.isDirectory()) {
                size_t filesize = file.size();
                if(filesize > 1) {
                    char buffer[filesize];
                    file.readBytes(buffer, filesize);
                    buffer[filesize] = '\0';
                    _settings.socials.key = buffer;
                }
            }
            Utils::removeRecursive(SPIFFS, file);
        }
    }

    this->saveConfig(_settings, SPIFFS, Config::Path_Config);

	SPIFFS.end();
}

// enabled backwards compatibility with older versions of this firmware or unused devices
// @param pref_version version of the saved preferences (last firmware version)
void ConfigService::migratePreferences(Version &preferences_version) {
    LOG("Info", "Migrating preferences from older version or unused device");

    //todo migrate preferences

    preferences.begin(_settings.firmware.preferences.c_str());
    preferences.putString(Config::P_Firmware_Version, _settings.firmware.version.toString());
    preferences.end();
}

// load configuration from filesystem (faactory settings) and user preferences
void ConfigService::init() {
    LOG("Info", "Starting configuration");

    if(SPIFFS.begin()) {
        bool configLoaded = this->loadConfig(SPIFFS, String(Config::Path_Config));
        if(!configLoaded)
            this->migrateConfig();
        
        SPIFFS.end();
    } else {
        LOG("Error", "Could not mount SPIFFS");
    }

    this->loadPreferences();
}