#pragma once

#include <main.h>

#include <Utils.h>
#include <models/Version.h>

/*
*   Defines datatypes and constant configuration data.
*   Used by Services to control the application
*   Used for storing preferences in nvm
*   Used for storing factory settings in SPIFFS
*/

namespace Config {

    // filepaths
    namespace Paths {
        const char Config[] = "/config.json";
        const char FirmwareUpdate[] = "/firmware.bin";
        const char ServerHome[] = "/webinterface";
        const char ServerIndex[] = "/webinterface/index.html";
        const char ServerVersion[] = "/webinterface/version.json";
        const char Upload[] = "/tmp";
        const char Images[] = "/images";
    }
    // preference keys (max 15 chars) -> $(category)_$(setting)
    namespace Preferences {
        const char Firmware_Version[] = "firm_version";

        const char DisplayBrightness[] = "disp_bright";
        const char DisplayMode[] = "disp_mode";
        const char DisplayPosition[] = "disp_pos";
        const char DisplayClockMode[] = "disp_clk_mode";
        const char DisplaySocialsPosition[] = "disp_soci_pos";
        const char DisplayDiashowEnabled[] = "disp_dia_en";
        const char DisplayDiashowInterval[] = "disp_dia_int";
        const char DisplayDiashowSwitchModes[] = "disp_dia_sw";
        const char DisplayAnimationEnabled[] = "disp_anim_en";
        const char DisplayAnimationInterval[] = "disp_anim_int";

        const char SocialsEnabled[] = "soci_en";
        const char SocialsInterval[] = "soci_interval";
        const char SocialsServer[] = "soci_server";
        const char SocialsKey[] = "soci_key";

        const char ServerKey[] = "serv_key";

        const char TimeShowSeconds[] = "time_seconds";
        const char TimeShowYear[] = "time_year";
        const char TimeBlink[] = "time_blink";
        const char TimeFormat24h[] = "time_24h";
        const char TimeUpdate[] = "time_update";
        const char TimeServer[] = "time_server";
        const char TimeZone[] = "time_zone";
        
        const char WiFiHostname[] = "wifi_hostname";
        const char WiFiHostEnabled[] = "wifi_host_en";
        const char WiFiHostSSID[] = "wifi_host_ssid";
        const char WiFiHostPassword[] = "wifi_host_pass";
        const char WiFiConnectEnabled[] = "wifi_con_en";
        const char WiFiConnectSSID[] = "wifi_con_ssid";
        const char WiFiConnectPassword[] = "wifi_con_pass";
    }

    // modes
    const uint8_t ClockModeCount = 4;
    enum ClockMode {
        CLOCK_ANALOG,
        CLOCK_DIGITAL,
        CLOCK_DIGITAL_DATE,
        CLOCK_DIGITAL_BIG
    };

    const uint8_t DisplayModeCount = 3;
    enum DisplayMode {
        DISPLAY_IMAGES,
        DISPLAY_CLOCK,
        DISPLAY_SOCIALS,
    };

    enum OverlayMode {
        OVERLAY_NONE,
        OVERLAY_BRIGHTNESS,
        OVERLAY_ANIMATION_SPEED,
        OVERLAY_DIASHOW_SPEED,
        OVERLAY_TEXT
    };

    enum MenuMode {
        MENU_NONE,
        MENU_OVERVIEW,
        MENU_CLOCK,
        MENU_DATETIME,
        MENU_WIFI,
        MENU_WIFI_CONNECT,
        MENU_WIFI_HOST
    };

    // consfiguration structure with default values for backwards compatibility
    // for explanation of these settings look in `/data/config.json`
    struct GPIOConfig {
        struct {
            int8_t sda = 15;
            int8_t clk = 7;
        } i2c;

        struct {
            int8_t clk = 4;
            int8_t mosi = 5;
            int8_t miso = 6;
        } spi;

        struct {
            int8_t cs = 43;
        } sd;

        struct {
            int8_t address = 104;
        } rtc;

        struct {
            int8_t r1 = 35;
            int8_t g1 = 36;
            int8_t b1 = 37;
            int8_t r2 = 38;
            int8_t g2 = 39;
            int8_t b2 = 40;
            int8_t a = 41;
            int8_t b = 42;
            int8_t c = 2;
            int8_t d = 1;
            int8_t e = 44;
            int8_t clk = 47;
            int8_t lat = 41;
            int8_t oe = 45;
        } display;

        struct {
            bool softwareDebounce = false;
            bool pullup = true;

            struct {
                bool enabled = false;
                int8_t interrupt = -1;
                int8_t cs = -1;
                int8_t address = -1;
            } expander;
            
            int8_t btn1 = 8;
            int8_t btn2 = 3;
            int8_t btn3 = 10;
            
            struct {
                int8_t btn = 12;
                int8_t b = 13;
                int8_t a = 14;
            } rot1;
            
            struct {
                int8_t btn = 9;
                int8_t b = 11;
                int8_t a = 46;
            } rot2;
            
            struct {
                int8_t btn = 18;
                int8_t b = 16;
                int8_t a = 14;
            } rot3;
        } controls;
    };

    struct WiFiConnection {
        bool enabled = false;
        String ssid = "";
        String password = "";
    };

    struct WiFiConfig {
        String hostname = "dereffis-pixelart";
        WiFiConnection connect;
        WiFiConnection host;
    };

    struct TimeConfig {
        bool showSeconds = false;
        bool blink = false;
        bool showYear = true;
        bool format24h = true;

        bool update = true;
        String server = "pool.ntp.org";
        String zone = "UTC0";
    };

    struct FirmwareConfig {
        Version version = Version(VERSION);
        String preferences = PREFERENCES;
    };

    struct SocialsConfig {
        bool enabled = true;
        uint interval = 31000;
        String key = "";
        String server = "";
        String request = "";
    };

    struct ServerConfig {
        String key = Utils::generateUUID();
    };

    struct DisplayConfig {
        uint8_t brightness = 128;
        DisplayMode mode = DISPLAY_IMAGES;
        uint position = 0;

        struct {
            ClockMode mode = CLOCK_ANALOG;
        } clock;

        struct {
            uint position = 0;
        } socials;

        struct {
            bool enabled = false;
            uint interval = 10000;
            bool switchModes = true;
        } diashow;

        struct {
            bool enabled = true;
            uint interval = 200;
        } animation;
    };


    struct Config {
        GPIOConfig gpio;
        WiFiConfig wifi;
        SocialsConfig socials;
        TimeConfig time;
        FirmwareConfig firmware;
        ServerConfig server;
        DisplayConfig display;
    };

}