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

// save settings to file
// @param config Configuration to save to file
// @param fs Filesystem (i.e. `SD` or `SPIFFS`)
// @param file path to config on that filesystem
void ConfigService::saveConfig(Config::Config& config, fs::FS& fs, String path) {
    File configFile = fs.open(path, "w");
    if(configFile) {
        if(configFile.isDirectory()) {
            Utils::removeRecursive(fs, configFile);
            File configFile = fs.open(path, "w");
        }

        DynamicJsonDocument json(8192);
        
        struct GPIOConfig {
            struct {
                int8_t cs = 43;
            } sd;

            struct {
                int8_t address = 104;
            } rtc;
        };

        JsonObject gpio = json.createNestedObject("gpio");
        JsonObject gpioI2C = gpio.createNestedObject("i2c");
        gpioI2C["sda"] = config.gpio.i2c.sda;
        gpioI2C["clk"] = config.gpio.i2c.clk;
        JsonObject gpioSPI = gpio.createNestedObject("spi");
        gpioSPI["clk"] = config.gpio.spi.clk;
        gpioSPI["mosi"] = config.gpio.spi.mosi;
        gpioSPI["miso"] = config.gpio.spi.miso;
        JsonObject gpioSD = gpio.createNestedObject("sd");
        gpioSD["cs"] = config.gpio.sd.cs;
        JsonObject gpioRTC = gpio.createNestedObject("rtc");
        gpioRTC["address"] = config.gpio.rtc.address;

        JsonObject gpioDisplay = gpio.createNestedObject("display");
        gpioDisplay["r1"] = config.gpio.display.r1;
        gpioDisplay["g1"] = config.gpio.display.g1;
        gpioDisplay["b1"] = config.gpio.display.b1;
        gpioDisplay["r2"] = config.gpio.display.r2;
        gpioDisplay["g2"] = config.gpio.display.g2;
        gpioDisplay["b2"] = config.gpio.display.b2;
        gpioDisplay["a"] = config.gpio.display.a;
        gpioDisplay["b"] = config.gpio.display.b;
        gpioDisplay["c"] = config.gpio.display.c;
        gpioDisplay["d"] = config.gpio.display.d;
        gpioDisplay["e"] = config.gpio.display.e;
        gpioDisplay["clk"] = config.gpio.display.clk;
        gpioDisplay["lat"] = config.gpio.display.lat;
        gpioDisplay["oe"] = config.gpio.display.oe;

        JsonObject gpioControls = gpio.createNestedObject("controls");
        gpioControls["softwareDebounce"] = config.gpio.controls.softwareDebounce;
        gpioControls["btn1"] = config.gpio.controls.btn1;
        gpioControls["btn2"] = config.gpio.controls.btn2;
        gpioControls["btn3"] = config.gpio.controls.btn3;
        JsonObject gpioControlsExpander = gpioControls.createNestedObject("expander");
        gpioControlsExpander["enabled"] = config.gpio.controls.expander.enabled;
        gpioControlsExpander["interrupt"] = config.gpio.controls.expander.interrupt;
        gpioControlsExpander["cs"] = config.gpio.controls.expander.cs;
        gpioControlsExpander["address"] = config.gpio.controls.expander.address;
        JsonObject gpioControlsRot1 = gpioControls.createNestedObject("rot1");
        gpioControlsRot1["btn"] = config.gpio.controls.rot1.btn;
        gpioControlsRot1["a"] = config.gpio.controls.rot1.a;
        gpioControlsRot1["b"] = config.gpio.controls.rot1.b;
        JsonObject gpioControlsRot2 = gpioControls.createNestedObject("rot2");
        gpioControlsRot2["btn"] = config.gpio.controls.rot2.btn;
        gpioControlsRot2["a"] = config.gpio.controls.rot2.a;
        gpioControlsRot2["b"] = config.gpio.controls.rot2.b;
        JsonObject gpioControlsRot3 = gpioControls.createNestedObject("rot3");
        gpioControlsRot3["btn"] = config.gpio.controls.rot3.btn;
        gpioControlsRot3["a"] = config.gpio.controls.rot3.a;
        gpioControlsRot3["b"] = config.gpio.controls.rot3.b;

        JsonObject wifi = json.createNestedObject("wifi");
        wifi["hostname"] = config.wifi.hostname;
        JsonObject wifiHost = json.createNestedObject("host");
        wifiHost["enabled"] = config.wifi.host.enabled;
        wifiHost["ssid"] = config.wifi.host.ssid;
        wifiHost["password"] = config.wifi.host.password;
        JsonObject wifiConnect = json.createNestedObject("connect");
        wifiConnect["enabled"] = config.wifi.host.enabled;
        wifiConnect["ssid"] = config.wifi.host.ssid;
        wifiConnect["password"] = config.wifi.host.password;

        JsonObject time = json.createNestedObject("time");
        time["showSeconds"] = config.time.showSeconds;
        time["blink"] = config.time.blink;
        time["showYear"] = config.time.showYear;
        time["format24h"] = config.time.format24h;
        time["update"] = config.time.update;
        time["server"] = config.time.server;
        time["zone"] = config.time.zone;

        JsonObject socials = json.createNestedObject("socials");
        socials["enabled"] = config.socials.enabled;
        socials["interval"] = config.socials.interval;
        socials["key"] = config.socials.key;
        socials["server"] = config.socials.server;
        socials["request"] = config.socials.request;

        JsonObject display = json.createNestedObject("display");
        display["brightness"] = config.display.brightness;
        display["mode"] = (int)config.display.mode;
        display["position"] = config.display.position;
        JsonObject displayClock = display.createNestedObject("clock");
        displayClock["mode"] = (int)config.display.clock.mode;
        JsonObject displaySocials = display.createNestedObject("socials");
        displaySocials["position"] = config.display.socials.position;
        JsonObject diashow = display.createNestedObject("diashow");
        diashow["enabled"] = config.display.diashow.enabled;
        diashow["interval"] = config.display.diashow.interval;
        diashow["switchModes"] = config.display.diashow.switchModes;
        JsonObject animation = display.createNestedObject("animation");
        animation["enabled"] = config.display.animation.enabled;
        animation["interval"] = config.display.animation.interval;
        
        serializeJson(json, configFile);
        configFile.close();
    }
}

// load settings from file
// @param fs Filesystem (i.e. `SD` or `SPIFFS`)
// @param file path to config on that filesystem
// @return success
bool ConfigService::loadConfig(fs::FS& fs, String path) {
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
// @param key preference identifier (max 15 chars)
// @param setting String to load preference into
void ConfigService::loadPreference(const char *key, String& setting) {
    if(preferences.isKey(key) && preferences.getType(key) == PT_STR)
        setting = preferences.getString(key, setting);
}
// load single preference (bool) into settings member
// @param key preference identifier (max 15 chars)
// @param setting bool to load preference into
void ConfigService::loadPreference(const char *key, bool& setting) {
    if(preferences.isKey(key) && preferences.getType(key) == PT_U8)
        setting = preferences.getBool(key, setting);
}
// load single preference (uint) into settings member
// @param key preference identifier (max 15 chars)
// @param setting uint to load preference into
void ConfigService::loadPreference(const char *key, uint& setting) {
    if(preferences.isKey(key) && preferences.getType(key) == PT_U32)
        setting = preferences.getUInt(key, setting);
}
// load single preference (uint8) into settings member
// @param key preference identifier (max 15 chars)
// @param setting uint8_t to load preference into
void ConfigService::loadPreference(const char *key, uint8_t& setting) {
    if(preferences.isKey(key) && preferences.getType(key) == PT_U8)
        setting = preferences.getUChar(key, setting);
}
// load single preference (enum) into settings member
// @param key preference identifier (max 15 chars)
// @param setting Enum to load preference into
template<typename SettingType>
void ConfigService::loadEnumPreference(const char *key, SettingType& setting) {
    if(preferences.isKey(key) && preferences.getType(key) == PT_U8)
        setting = static_cast<SettingType>(preferences.getUChar(key, 0));
}
// load single preference (String) into settings member from blob
// @param key preference identifier (max 15 chars)
// @param setting String to load preference into
void ConfigService::loadBlobPreference(const char *key, String& setting) {
    if(preferences.isKey(key) && preferences.getType(key) == PT_BLOB) {
        size_t blobSize = preferences.getBytesLength(key);
        if(blobSize > 2) {
            char buffer[blobSize];
            preferences.getBytes(key, buffer, blobSize);
            setting = String(buffer);
        }
    }
}


// save single preference (String) from settings member
// @param key preference identifier (max 15 chars)
// @param setting String
void ConfigService::savePreference(const char *key, String& setting) {
    if(preferences.isKey(key) && preferences.getType(key) != PT_STR)
        preferences.remove(key);
    preferences.putString(key, setting);
}
// save single preference (bool) from settings member
// @param key preference identifier (max 15 chars)
// @param setting bool
void ConfigService::savePreference(const char *key, bool& setting) {
    if(preferences.isKey(key) && preferences.getType(key) != PT_U8)
        preferences.remove(key);
    preferences.putBool(key, setting);
}
// save single preference (uint) from settings member
// @param key preference identifier (max 15 chars)
// @param setting uint
void ConfigService::savePreference(const char *key, uint& setting) {
    if(preferences.isKey(key) && preferences.getType(key) != PT_U32)
        preferences.remove(key);
    preferences.putUInt(key, setting);
}
// save single preference (uint8) from settings member
// @param key preference identifier (max 15 chars)
// @param setting uint8_t
void ConfigService::savePreference(const char *key, uint8_t& setting) {
    if(preferences.isKey(key) && preferences.getType(key) != PT_U8)
        preferences.remove(key);
    preferences.putUChar(key, setting);
}
// save single preference (enum) from settings member
// @param key preference identifier (max 15 chars)
// @param setting Enum
template<typename SettingType>
void ConfigService::saveEnumPreference(const char *key, SettingType& setting) {
    if(preferences.isKey(key) && preferences.getType(key) != PT_U8)
        preferences.remove(key);
    preferences.putUChar(key, (int)setting);
}
// save single preference (String) from settings member as Blob
// @param key preference identifier (max 15 chars)
// @param setting String
void ConfigService::saveBlobPreference(const char *key, String& setting) {
    if(preferences.isKey(key) && preferences.getType(key) != PT_BLOB)
        preferences.remove(key);
    const char * blobData = setting.c_str();
    preferences.putBytes(key, blobData, sizeof(blobData));
}


// Update current settings for category: Display
// @param config new configuration to save
void ConfigService::updateDisplay(Config::DisplayConfig& config) {
    _settings.display = config;
    this->saveDisplay();
}
void ConfigService::saveDisplay() {
    preferences.begin("pixelart");
    this->savePreference(Config::P_Display_Brightness, _settings.display.brightness);
    this->saveEnumPreference(Config::P_Display_Mode, _settings.display.mode);
    this->savePreference(Config::P_Display_Position, _settings.display.position);
    this->saveEnumPreference(Config::P_Display_Clock_Mode, _settings.display.clock.mode);
    this->savePreference(Config::P_Display_Socials_Position, _settings.display.socials.position);
    this->savePreference(Config::P_Display_Diashow_Enabled, _settings.display.diashow.enabled);
    this->savePreference(Config::P_Display_Diashow_Interval, _settings.display.diashow.interval);
    this->savePreference(Config::P_Display_Diashow_SwitchModes, _settings.display.diashow.switchModes);
    this->savePreference(Config::P_Display_Animation_Enabled, _settings.display.animation.enabled);
    this->savePreference(Config::P_Display_Animation_Interval, _settings.display.animation.interval);
    preferences.end();
}

// Update current settings for category: Socials
// @param config new configuration to save
void ConfigService::updateSocials(Config::SocialsConfig& config) {
    _settings.socials = config;
    this->saveSocials();
}
void ConfigService::saveSocials() {
    preferences.begin("pixelart");
    this->savePreference(Config::P_Socials_Enabled, _settings.socials.enabled);
    this->savePreference(Config::P_Socials_Interval, _settings.socials.interval);
    this->savePreference(Config::P_Socials_Server, _settings.socials.server);
    this->savePreference(Config::P_Socials_Key, _settings.socials.key);
    this->saveBlobPreference(Config::P_Socials_Key, _settings.socials.request);
    preferences.end();
}

// Update current settings for category: Server
// @param config new configuration to save
void ConfigService::updateServer(Config::ServerConfig& config) {
    _settings.server = config;
    this->saveServer();
}
void ConfigService::saveServer() {
    preferences.begin("pixelart");
    this->savePreference(Config::P_Server_Key, _settings.server.key);
    preferences.end();
}

// Update current settings for category: Time
// @param config new configuration to save
void ConfigService::updateTime(Config::TimeConfig& config) {
    _settings.time = config;
    this->saveTime();
}
void ConfigService::saveTime() {
    preferences.begin("pixelart");
    this->savePreference(Config::P_Time_ShowSeconds, _settings.time.showSeconds);
    this->savePreference(Config::P_Time_ShowYear, _settings.time.showYear);
    this->savePreference(Config::P_Time_Blink, _settings.time.blink);
    this->savePreference(Config::P_Time_Format24h, _settings.time.format24h);
    this->savePreference(Config::P_Time_Update, _settings.time.update);
    this->savePreference(Config::P_Time_Server, _settings.time.server);
    this->savePreference(Config::P_Time_Zone, _settings.time.zone);
    preferences.end();
}

// Update current settings for category: WiFi
// @param config new configuration to save
void ConfigService::updateWifi(Config::WiFiConfig& config) {
    _settings.wifi = config;
    this->saveWifi();
}
void ConfigService::saveWifi() {
    preferences.begin("pixelart");
    this->savePreference(Config::P_WiFi_Hostname, _settings.wifi.hostname);
    this->savePreference(Config::P_WiFi_Host_Enabled, _settings.wifi.host.enabled);
    this->savePreference(Config::P_WiFi_Host_SSID, _settings.wifi.host.ssid);
    this->savePreference(Config::P_WiFi_Host_Password, _settings.wifi.host.password);
    this->savePreference(Config::P_WiFi_Connect_Enabled, _settings.wifi.connect.enabled);
    this->savePreference(Config::P_WiFi_Connect_SSID, _settings.wifi.connect.ssid);
    this->savePreference(Config::P_WiFi_Connect_Password, _settings.wifi.connect.password);
    preferences.end();
}


// load user preferences from nvm
void ConfigService::loadPreferences() {
    LOG("Info", "Loading user preferences");

    try {
        preferences.begin(_settings.firmware.preferences.c_str(), true);
        
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
        this->loadBlobPreference(Config::P_Socials_Key, _settings.socials.request);

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

        // checking for older version
        String version = "0";
        this->loadPreference(Config::P_Firmware_Version, version);
        preferences.end();

        Version pref_version(version);
        Version firm_version(_settings.firmware.version);
        if(pref_version < firm_version) {
            this->migratePreferences(pref_version);
            return;
        }

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

    // searching for old preference keys and clear up
    preferences.begin("pixelart");
	
	this->loadPreference("brightness", _settings.display.brightness);
	this->loadEnumPreference("current_mode", _settings.display.mode);
	this->loadPreference("selected_image", _settings.display.position);
	this->loadEnumPreference("clock_mode", _settings.display.clock.mode);
	this->loadPreference("current_social", _settings.display.socials.position);
	this->loadPreference("diashow", _settings.display.diashow.enabled);
	this->loadPreference("diashow_time", _settings.display.diashow.interval);
	this->loadPreference("diashow_modes", _settings.display.diashow.switchModes);
	this->loadPreference("animation", _settings.display.animation.enabled);
	this->loadPreference("animation_time", _settings.display.animation.interval);

	this->loadPreference("wifi_connect", _settings.wifi.connect.enabled);
	this->loadPreference("wifi_ssid", _settings.wifi.connect.ssid);
	this->loadPreference("wifi_password", _settings.wifi.connect.password);
    this->loadPreference("wifi_host", _settings.wifi.host.enabled);
	this->loadPreference("wifi_ap_ssid", _settings.wifi.host.ssid);
	this->loadPreference("wifi_ap_pass", _settings.wifi.host.password);

	this->loadPreference("clock_seconds", _settings.time.showSeconds);
	this->loadPreference("clock_blink", _settings.time.blink);
	this->loadPreference("clock_year", _settings.time.showYear);
	this->loadPreference("time_format24", _settings.time.format24h);
	this->loadPreference("update_time", _settings.time.update);
	this->loadPreference("ntpServer", _settings.time.server);
	this->loadPreference("timezone", _settings.time.zone);

	this->loadPreference("socials_server", _settings.socials.server);
	this->loadPreference("socials_api_key", _settings.socials.key);
	this->loadPreference("socials_request", _settings.socials.request);

	this->loadPreference("api_key", _settings.server.key);
    
    preferences.clear();
	preferences.end();

    // write new preferences
    preferences.begin(_settings.firmware.preferences.c_str());
    this->saveDisplay();
    this->saveSocials();
    this->saveServer();
    this->saveTime();
    this->saveWifi();
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
