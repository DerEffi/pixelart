#include <main.h>
#include <Arduino.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Font4x5Fixed.h>
#include <Font4x7Fixed.h>
#include <SPI.h>
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




//TODO remove tests
unsigned long ms_test = 0;

/**************
**	Globals  **
***************/

//colors
const uint16_t color_twitch = 25108;
const uint16_t color_youtube = 0xF800;
const uint16_t color_coral = 0xFC10;
const uint16_t color_dark_read = 0xBA00;

//bitmaps
const uint8_t icon_person[7] = {0x78, 0x48, 0x48, 0x78, 0x00, 0xfc, 0x84}; //6x7
const uint8_t icon_sun[7] = {0x10, 0x44, 0x38, 0xba, 0x38, 0x44, 0x10}; //7x7
const uint8_t icon_diashow[7] = {0x3e, 0x22, 0xfa, 0x8a, 0x8e, 0x88, 0xf8}; //7x7
const uint8_t icon_animation[7] = {0x00, 0x72, 0x06, 0xee, 0x06, 0x72, 0x00}; //7x7
const uint8_t icon_star[7] = {0x10, 0x38, 0xfe, 0x7c, 0x38, 0x6c, 0x44}; //7x7
const uint8_t icon_heart[7] = {0x66, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x18}; //8x7
const uint8_t icon_bell[7] = {0x00, 0x30, 0x78, 0xfc, 0xfc, 0x00, 0x10}; //6x7
const uint8_t icon_eye[14] = {0x1c, 0x00, 0x22, 0x00, 0x49, 0x00, 0x9c, 0x80, 0x49, 0x00, 0x22, 0x00, 0x1c, 0x00}; //9x7
const uint8_t icon_card[128] = {0x00, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xf0, 0x01, 0x92, 0x49, 0x30, 0x01, 0x92, 0x49, 0x30, 0x01, 0x92, 0x49, 0x30, 0x01, 0x92, 0x49, 0x30, 0x01, 0x92, 0x49, 0x30, 0x01, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xe0}; //32x32
const uint8_t icon_arrow_down[3] = {0xfc, 0x78, 0x30}; //6x3
const uint8_t icon_arrow_up[3] = {0x30, 0x78, 0xfc}; //6x3

const uint8_t icon_twitch[120] = {0x0f, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xf0, 0x30, 0x00, 0x00, 0x30, 0x70, 0x00, 0x00, 0x30, 0xf0, 0x00, 0x00, 0x30, 0xf0, 0x00, 0x00, 0x30, 0xf0, 0x00, 0x00, 0x30, 0xf0, 0x18, 0x0c, 0x30, 0xf0, 0x18, 0x0c, 0x30, 0xf0, 0x18, 0x0c, 0x30, 0xf0, 0x18, 0x0c, 0x30, 0xf0, 0x18, 0x0c, 0x30, 0xf0, 0x18, 0x0c, 0x30, 0xf0, 0x18, 0x0c, 0x30, 0xf0, 0x18, 0x0c, 0x30, 0xf0, 0x00, 0x00, 0x30, 0xf0, 0x00, 0x00, 0x30, 0xf0, 0x00, 0x00, 0x30, 0xf0, 0x00, 0x00, 0x30, 0xf0, 0x00, 0x00, 0x30, 0xf0, 0x00, 0x00, 0x70, 0xf0, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x01, 0xe0, 0xff, 0xf1, 0xff, 0xc0, 0xff, 0xf3, 0xff, 0x80, 0xff, 0xf7, 0xff, 0x00, 0xff, 0xff, 0xfe, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00}; //28x30
const uint8_t icon_youtube_base[96] = {0x0f, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xf0}; //32x24
const uint8_t icon_youtube_play[24] = {0x80, 0x00, 0xc0, 0x00, 0xf0, 0x00, 0xfc, 0x00, 0xfe, 0x00, 0xff, 0x80, 0xff, 0x80, 0xfe, 0x00, 0xfc, 0x00, 0xf0, 0x00, 0xc0, 0x00, 0x80, 0x00}; //9x12

const uint8_t marks_clock[44][2] = {{0,31},{1,31},{2,31},{3,31},{0,32},{1,32},{2,32},{3,32},{60,31},{61,31},{62,31},{63,31},{60,32},{61,32},{62,32},{63,32},{31,0},{31,1},{31,2},{31,3},{32,0},{32,1},{32,2},{32,3},{31,60},{31,61},{31,62},{31,63},{32,60},{32,61},{32,62},{32,63},{16,5},{47,5},{58,16},{58,47},{5,16},{5,47},{16,58},{47,58},{32,32},{31,31},{32,31},{31,32}};

//slopes for mapping ranges to pixel width
//? (output_end - output_start) / (input_end - input_start)
//? 44 / input_range
const double slope_brightness = .189;
const double slope_animation = .09166;
const double slope_diashow = .0007457;

//structs
enum overlay_type {
	OVERLAY_NONE,
	OVERLAY_BRIGHTNESS,
	OVERLAY_ANIMATION_SPEED,
	OVERLAY_DIASHOW_SPEED,
	OVERLAY_TEXT
};

#define DISPLAY_MODE_NUMBER 3
enum display_mode {
	MODE_IMAGES,
	MODE_CLOCK,
	MODE_SOCIALS,
};

#define CLOCK_TYPE_NUMBER 4
enum clock_type {
	CLOCK_ANALOG,
	CLOCK_DIGITAL,
	CLOCK_DIGITAL_DATE,
	CLOCK_DIGITAL_BIG
};

enum menu_mode {
	MENU_NONE,
	MENU_OVERVIEW,
	MENU_CLOCK,
	MENU_DATETIME,
	MENU_WIFI,
	MENU_WIFI_CONNECT,
	MENU_WIFI_HOST
};

struct socials_channel {
	char* type;
	char* name;
	char* follower;
	char* views;
	socials_channel(char* type, char* name, char* follower, char* views):
	type(type), name(name), follower(follower), views(views) {}
};

struct image_meta {
	char* folder;
	uint16_t prefix;
	bool animated;
	image_meta(char* folder, uint16_t prefix = 0, bool animated = false):
	folder(folder), prefix(prefix), animated(animated) {}
};

struct pixel_data {
	uint8_t x;
	uint8_t y;
	uint16_t color;
	pixel_data(uint8_t x, uint8_t y, uint16_t color):
	x(x), y(y), color(color) {}
};

struct boot_row {
	uint8_t row;
	std::vector<uint8_t> pixels;
	boot_row(uint8_t row, std::vector<uint8_t> pixels):
	row(row), pixels(pixels) {}
};

const std::vector<boot_row> boot_sequence = {{59,{31,32}},{58,{30,31,32,33}},{57,{29,30,33,34}},{56,{28,29,34,35}},{55,{28,35}},{54,{27,28,35,36}},{53,{26,27,36,37}},{52,{25,26,37,38}},{51,{24,25,38,39}},{50,{24,39}},{49,{23,24,39,40}},{48,{22,23,40,41}},{47,{21,22,41,42}},{46,{20,21,42,43}},{45,{20,43}},{44,{19,20,43,44}},{43,{18,19,44,45}},{42,{17,18,45,46}},{41,{16,17,46,47}},{40,{16,47}},{39,{15,16,47,48}},{38,{14,15,48,49}},{37,{13,14,49,50}},{36,{12,13,50,51}},{35,{12,51}},{34,{11,12,51,52}},{33,{11,52}},{32,{10,11,52,53}},{31,{10,53}},{30,{10,53}},{29,{10,53}},{28,{10,53}},{27,{10,53}},{26,{10,53}},{25,{10,53}},{24,{10,53}},{23,{9,10,53,54}},{22,{9,10,53,54}},{21,{9,10,53,54}},{20,{9,10,53,54}},{19,{9,10,53,54}},{18,{9,54}},{17,{9,54}},{16,{9,54}},{15,{9,54}},{14,{9,54}},{13,{9,54}},{12,{9,54}},{11,{9,54}},{10,{9,54}},{9,{8,9,54,55}},{8,{8,9,54,55}},{7,{8,9,54,55}},{6,{8,9,54,55}},{5,{8,9,54,55}},{4,{8,9,54,55}},{3,{8,9,54,55}},{4,{10,53}},{5,{10,11,52,53}},{6,{10,11,12,51,52,53}},{7,{10,12,13,50,51,53}},{8,{10,13,14,49,50,53}},{9,{10,11,14,15,48,49,52,53}},{10,{11,15,16,47,48,52}},{11,{11,16,17,46,47,52}},{12,{11,12,17,18,45,46,51,52}},{13,{12,18,19,44,45,51}},{14,{12,13,19,20,43,44,50,51}},{15,{13,20,21,42,43,50}},{16,{13,21,22,41,42,50}},{17,{13,14,22,23,40,41,49,50}},{18,{14,23,24,39,40,49}},{19,{14,15,24,25,38,39,48,49}},{19, {26,27,28,35,36,37}}, {18, {29,30,31,32,33,34}},{20,{15,23,24,39,40,48}},{21,{15,22,23,40,41,48}},{22,{15,16,21,22,41,42,47,48}},{23,{16,20,21,42,43,47}},{24,{16,17,19,20,43,44,46,47}},{25,{17,18,19,44,45,46}},{26,{17,18,45,46}},{27,{16,17,18,45,46,47}},{28,{15,16,18,19,44,45,47,48}},{29,{14,15,19,44,48,49}},{30,{13,14,19,20,43,44,49,50}},{31,{12,13,20,43,50,51}},{32,{12,13,20,43,50,51}},{33,{12,13,14,15,16,20,21,42,43,47,48,49,50,51}},{34,{16,17,21,42,46,47}},{35,{17,18,19,21,22,41,42,44,45,46}},{36,{19,20,22,41,43,44}},{37,{20,21,22,41,42,43}},{38,{21,22,23,40,41,42}},{39,{22,23,40,41}},{40,{23,24,39,40}},{41,{24,39}},{42,{24,25,38,39}},{43,{25,38}},{44,{25,26,37,38}},{45,{26,37}},{46,{26,37}},{47,{26,27,36,37}},{48,{27,36}},{49,{27,28,35,36}},{50,{28,35}},{51,{28,35}},{52,{28,29,34,35}},{53,{28,29,34,35}},{54,{29,34}},{55,{29,30,33,34}},{56,{30,33}},{57,{31,32}}};
uint8_t loading_step = 0;
unsigned long ms_loading = 0;

//settings
Preferences preferences;
unsigned long ms_current = 0;
bool requested_restart = false;
bool loading_cycle = true;

overlay_type overlay = OVERLAY_NONE;
char* overlay_text = strdup("");
unsigned long ms_overlay = 0;

menu_mode menu = MENU_NONE;
uint8_t menu_selection = 0;

boolean menu_time_changed = false;
uint8_t menu_day = 0;
uint8_t menu_month = 0;
uint8_t menu_year = 0;
uint8_t menu_hour = 0;
uint8_t menu_minute = 0;
uint8_t menu_second = 0;

uint8_t brightness = 128;
bool animation_enabled = false;
bool diashow_enabled = false;
bool diashow_modes = false;
uint16_t animation_time = 100;
unsigned long ms_animation = 0;
uint32_t diashow_time = 5000;
unsigned long ms_diashow = 0;

//Wifi
bool wifi_connect = WIFI_CONNECT_DEFAULT;
bool wifi_host = WIFI_HOST_DEFAULT;
bool wifi_setup_complete = true;
char* wifi_ssid = strdup(WIFI_SSID_DEFAULT);
char* wifi_ap_ssid = strdup(WIFI_AP_SSID_DEFAULT);
char* wifi_password = strdup(WIFI_PASSWORD_DEFAULT);
char* wifi_ap_password = strdup(WIFI_AP_PASSWORD_DEFAULT);
unsigned long ms_wifi_routine = 0;
unsigned long ms_wifi_reconnect = 0;
DNSServer dns_server;

//Server
AsyncWebServer server(80);
char* api_key = strdup("");
unsigned long ms_api_key_request = 0;
unsigned long ms_api_key_approve = 0;

//LED Panel
MatrixPanel_I2S_DMA *panel = nullptr;
display_mode current_mode = MODE_IMAGES;
bool display_change = false;

std::vector<image_meta> image_index;
uint16_t image_prefix_max = 0;
uint16_t selected_image = 0;
uint16_t current_image[64][64] = {};
std::vector<std::vector<pixel_data>> animation;
bool image_loaded = false;
uint16_t animation_frame = 0;

//Socials
AsyncHTTPSRequest http_socials;
char* socials_api_key = strdup(SOCIALS_API_KEY);
char* socials_request_server = strdup(SOCIALS_API_SERVER);
unsigned long ms_socials_request = 0;
unsigned long socials_response_check = 0;
char* socials_request = strdup(SOCIALS_REQUEST);
int socials_channel_current = 0;
std::vector<socials_channel> socials_channels;

//Clock
unsigned long ms_clock = 0;
clock_type current_clock_mode = CLOCK_ANALOG;
bool clock_seconds = true;
bool clock_blink = false;
bool clock_year = true;

//Interrupt flags
bool rot1_a_flag = false;
bool rot1_b_flag = false;
bool rot2_a_flag = false;
bool rot2_b_flag = false;
bool rot3_a_flag = false;
bool rot3_b_flag = false;

bool btn1_pressed = false;
bool btn2_pressed = false;
unsigned long ms_btn3_pressed = 0;
bool btn3_released = false;
bool rot1_pressed = false;
bool rot2_pressed = false;
bool rot3_pressed = false;
int rot1_clicks = 0;
int rot2_clicks = 0;
int rot3_clicks = 0;

//RTC
RTC_DS3231 rtc_ext;
ESP32Time rtc_int(0);
bool rtc_ext_enabled = false;
char* ntp_server = strdup(TIME_SERVER);
char* timezone = strdup(TIME_ZONE);
bool update_time = TIME_UPDATE_AUTO;
bool time_format24 = TIME_FORMAT24;
bool rtc_ext_adjust = false;
unsigned long ms_rtc_ext_adjust = 0;




/**************
**	Socials  **
***************/

void on_socials_response(){
	if(http_socials.responseHTTPcode() == 200) {
		DynamicJsonDocument socials_response_doc(8000);
		DeserializationError error = deserializeJson(socials_response_doc, http_socials.responseText());
		if(!error && socials_response_doc.is<JsonArray>()) {
			JsonArray socials_response_array = socials_response_doc.as<JsonArray>();

			//clear old data
			for(int i = 0; i < socials_channels.size(); i++) {
				free(socials_channels[i].follower);
				free(socials_channels[i].views);
				free(socials_channels[i].type);
				free(socials_channels[i].name);
			}
			socials_channels.clear();

			//fill with new data
			for(int i = 0; i < socials_response_array.size(); i++) {
				const char* t = socials_response_array[i]["t"].as<const char*>();
				const char* d = socials_response_array[i]["d"].as<const char*>();
				const char* c = socials_response_array[i]["c"].as<const char*>();
				const char* f = socials_response_array[i]["f"].as<const char*>();
				const char* v = socials_response_array[i]["v"].as<const char*>();
			
				socials_channel channel = {
					t ? strdup(t) : strdup(""),
					d ? strdup(d) : c ? strdup(c) : strdup(""),
					f ? strdup(f) : strdup("0"),
					v ? strdup(v) : strdup("0")
				};
				socials_channels.emplace_back(channel);
			}

			display_change = true;
		}
	}

	http_socials.abort();
	ms_socials_request = millis() + SOCIAL_REFRESH_INTERVAL;
}

void socials_refresh() {
	ms_socials_request = millis() + SOCIAL_REFRESH_INTERVAL;
	if(http_socials.readyState() == readyStateUnsent || http_socials.readyState() == readyStateDone) {
		char request_url[strlen(socials_request_server) + 9];
		strcpy(request_url, socials_request_server);
		strcat(request_url, "/socials");
		bool openResult = http_socials.open("POST", request_url);

		if(openResult) {
			http_socials.setReqHeader("Authorization", socials_api_key);
			http_socials.setReqHeader("Content-Type", "application/json");
			http_socials.setReqHeader("Accept", "application/json");
			if(http_socials.send(socials_request)) {
				socials_response_check = millis() + 500;
			}
		}
	}
}




/***********
**	Misc  **
************/
void rtc_internal_adjust() {
	DateTime time = rtc_ext.now();
	rtc_int.setTime(time.second(), time.minute(), time.hour(), time.day(), time.month(), time.year());
}

void rtc_external_adjust() {
	if(rtc_ext_enabled) {
		struct tm timeinfo;
		if(getLocalTime(&timeinfo, 500))
		{
			rtc_ext.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
		}
	}
	rtc_ext_adjust = false;
	ms_rtc_ext_adjust = 0;
}

void wifi_on_connected() {
	if(update_time) {
		configTzTime(timezone, ntp_server);
		if(rtc_ext_enabled) {
			ms_rtc_ext_adjust = millis() + 10000;
			rtc_ext_adjust = true;
		}
	}

	display_change = true;
	socials_refresh();
}

char* generate_uid(){
  /* Change to allowable characters */
  const char possible[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  static char uid[33];
  for(int p = 0, i = 0; i < 32; i++){
    int r = random(0, strlen(possible));
    uid[p++] = possible[r];
  }
  uid[32] = '\0';
  return uid;
}

bool verify_api_key(AsyncWebServerRequest * request) {
	return request->hasHeader("apiKey") && request->getHeader("apiKey")->value().equals(api_key);
}

void menu_copy_time() {
	menu_year = rtc_int.getYear() % 100;
	menu_month = rtc_int.getMonth() + 1;
	menu_day = rtc_int.getDay();
	menu_hour = rtc_int.getHour(true);
	menu_minute = rtc_int.getMinute();
	menu_second = rtc_int.getSecond();
}

void menu_correct_date() {
	if(menu_day > 30 && (menu_month == 4 || menu_month == 6 || menu_month == 9 || menu_month == 11))
		menu_day = rot3_clicks > 0 ? 1 : 30;

	if(menu_day > 29 && menu_month == 2 && menu_year % 4 == 0)
		menu_day = rot3_clicks > 0 ? 1 : 29;
	
	if(menu_day > 28 && menu_month == 2 && menu_year % 4 != 0)
		menu_day = rot3_clicks > 0 ? 1 : 28;
}

void menu_update_time() {
	struct tm t = {0};
	t.tm_year = menu_year + 100;
	t.tm_mon = menu_month - 1;
	t.tm_mday = menu_day;
	t.tm_hour = menu_hour;
	t.tm_min = menu_minute;
	t.tm_sec = menu_second;
	rtc_int.setTime(mktime(&t));
	
	if(rtc_ext_enabled) {
		ms_rtc_ext_adjust = millis() + 1000;
		rtc_ext_adjust = true;
	}

	if(wifi_connect) {
		preferences.begin(PREFERENCES_NAMESPACE);
		update_time = false;
		preferences.putBool("update_time", update_time);
		preferences.end();
	}

	menu_time_changed = false;
}




/************
**	SPFFS  **
*************/

bool compare_image_name(image_meta i1, image_meta i2) {
	return strcmp(i1.folder, i2.folder) < 0;
}

//reindex images folder of sd card for loading and diplaying byte data
void sd_index() {

	//remove old data from index
	for(int i = 0; i < image_index.size(); i++) {
		free(image_index[i].folder);
	}
	image_index.clear();

	//fill vector with new data
	if(SD.exists("/images")) {
		File folder = SD.open("/images");
		if(folder.isDirectory()) {
			char file_path[255]; 
			File image = folder.openNextFile();
			while(image) {
				//check for image file
				if(image.isDirectory()) {
					image_meta meta = {
						strdup(image.path())
					};
					strcpy(file_path, meta.folder);
					strcat(file_path, "/image.pxart");

					if(SD.exists(file_path) && !SD.open(file_path).isDirectory()) {
						//rename folder if no prefix number is given
						const char* name = image.name();
						bool rename = false;
						for(int i = 0; i < 3; i++) {
							if(name[i] < 48 || name[i] > 57) {
								rename = true;
								break;
							}
						}
						if(name[3] != 32 || name[4] != 45 || name[5] != 32)
							rename = true;

						char prefix[4];
						if(rename) {
							do {
								meta.prefix++;
								sprintf(prefix, "%03d", meta.prefix);
								strcpy(file_path, "/images/");
								strcat(file_path, prefix);
								strcat(file_path, " - ");
								strcat(file_path, name);
							} while(SD.exists(file_path));
							if(SD.rename(meta.folder, file_path)) {
								free(meta.folder);
								meta.folder = strdup(file_path);
							}
						} else {
							strncpy(prefix, name, 3);
							sscanf(prefix, "%d", &meta.prefix);
						}

						if(meta.prefix > image_prefix_max)
							image_prefix_max = meta.prefix;

						//check for animation file				
						strcpy(file_path, meta.folder);
						strcat(file_path, "/animation.pxart");
						if(SD.exists(file_path) && !SD.open(file_path).isDirectory()) {
							meta.animated = true;
						}
						
						image_index.emplace_back(meta);
					}
				}
				image = folder.openNextFile();
			}
		}
	}

	//sort vector by image name
	std::sort(image_index.begin(), image_index.end(), compare_image_name);
}

bool sd_connected() {
	if(SD.exists("/"))
		return true;
	
	SD.end();
	if(!SD.begin(GPIO_SD_CS, SPI))
		return false;
	
	bool success = SD.exists("/");
	if(success)
		sd_index();
	
	return success;
}

bool sd_load_image(image_meta image) {
	animation.clear();
	char file_path[255];
	strcpy(file_path, image.folder);
	strcat(file_path, "/image.pxart");

	if(SD.exists(file_path)) {
		File file = SD.open(file_path);

		uint8_t x = 0;
		uint8_t y = 0;
		uint position = 0;
		int length = file.available();
		const static uint16_t buffersize = 512;
		char buffer_data[buffersize] = {};

		//read in 4 pixelmatrix lines because of array size limit
		while(length > position) {
			file.seek(position);
			file.readBytes(buffer_data, buffersize);

			for(int i = 0; i < buffersize; i += 2) {
				current_image[y][x] = (buffer_data[i+1] << 8) + buffer_data[i];
				x++;
				if(x >= 64) {
					x = 0;
					y++;
				}
				if(y >= 64)
					break;
			}

			if(y >= 64)
				break;
			position += buffersize;
		}

		if(image.animated) {
			strcpy(file_path, image.folder);
			strcat(file_path, "/animation.pxart");

			if(SD.exists(file_path)) {
				File file = SD.open(file_path);
				
				file.seek(0);
				file.readBytes(buffer_data, 4);
				std::vector<pixel_data> frame;

				position = 4;
				while(length > position) {
					uint fragment_size = length - position - 1;
					if(fragment_size > buffersize)
						fragment_size = buffersize;
					file.seek(position);
					file.readBytes(buffer_data, fragment_size);

					for(int i = 0; i < fragment_size - 3; i += 4) {
						x = buffer_data[i];
						y = buffer_data[i+1];
						if(x >= 64 || y >= 64) {
							animation.emplace_back(frame);
							frame.clear();
						} else {
							pixel_data pixel = {
								x,
								y,
								(uint16_t)((buffer_data[i+3] << 8) + buffer_data[i+2])
							};
							frame.emplace_back(pixel);
						}
					}

					position += buffersize;
				}

				animation.emplace_back(frame);
				frame.clear();
			}
		}

		return true;
	}
	
	return false;
}

void restart() {
	panel->clearScreen();
	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();

	panel->stopDMAoutput();
	if(sd_connected())
		SD.end();

	ESP.restart();
}

void firmware_update() {
	if(sd_connected() && SD.exists("/firmware.bin")) {
		//Display update process
		panel->setTextSize(1);
		panel->setTextWrap(false);
		panel->setBrightness(192);
		panel->setCursor(5, 12);
		panel->write("Updating Device");
		panel->setCursor(7, 28);
		panel->write("Do NOT remove");
		panel->setCursor(12, 37);
		panel->write("the SD Card");
		panel->setCursor(10, 46);
		panel->write("or power off");
		panel->setCursor(13, 55);
		panel->write("this device");
		
		if(PANEL_DOUBLE_BUFFER)
			panel->flipDMABuffer();

		//Update
		File firmware = SD.open("/firmware.bin");
		if(!firmware.isDirectory()) {
			size_t update_size = firmware.size();
			if(update_size > 1024 && Update.begin(update_size)) {
				size_t written_size = Update.writeStream(firmware);
				if((written_size != update_size || !Update.end() || !Update.isFinished()) && Update.canRollBack())
					Update.rollBack();
			}
		}
		firmware.close();
		SD.remove("/firmware.bin");
		restart();
	}
}



/**************
**	Display  **
**************/

//Display loading animation
void display_loading() {
	if(ms_loading == 0)
		ms_loading = millis() + 20;

	if(loading_step == 0 && loading_cycle) {
		panel->clearScreen();
		if(PANEL_DOUBLE_BUFFER) {
			panel->flipDMABuffer();
			panel->clearScreen();
		}
	}

	for(int pixel = 0; pixel < boot_sequence[loading_step].pixels.size(); pixel++) {
		panel->drawPixel(boot_sequence[loading_step].pixels[pixel], boot_sequence[loading_step].row, loading_cycle ? 0xFFFF : 0);
	}

	if(PANEL_DOUBLE_BUFFER) {
		panel->flipDMABuffer();

		for(int pixel = 0; pixel < boot_sequence[loading_step].pixels.size(); pixel++) {
			panel->drawPixel(boot_sequence[loading_step].pixels[pixel], boot_sequence[loading_step].row, loading_cycle ? 0xFFFF : 0);
		}
	}
}

void reset_loading() {
	ms_loading = 0;
	loading_step = 0;
	loading_cycle = true;
}

//Display overlay menu for io switches
void display_overlay() {
	
	if(overlay != OVERLAY_NONE) {
		//Background
		panel->fillRect(0, 0, 64, 11, 0);
		
		if(overlay == OVERLAY_BRIGHTNESS) {
			//Progress Bar
			panel->drawRect(2, 2, 49, 7, 0xFFFF);

			//maps brightness range to progress bar width (max: 45px)
			//? floor(output_start + (slope * (input - input_start)) + 0.5)
			panel->fillRect(4, 4, floor((1 + slope_brightness * (brightness - 16)) + .5), 3, 0xFFFF);
		
			//Sun icon
			panel->drawBitmap(54, 2, icon_sun, 7, 7, 0xFFFF);
		} else if(overlay == OVERLAY_ANIMATION_SPEED) {
			//Progress Bar
			panel->drawRect(2, 2, 49, 7, 0xFFFF);

			//maps animation_time range to progress bar width (max: 45px)
			//? floor(output_start + (slope * (input - input_start)) + 0.5)
			panel->fillRect(4, 4, floor((45 - slope_animation * (animation_time - 20)) + .5), 3, 0xFFFF);
		
			//Sun icon
			panel->drawBitmap(54, 2, icon_animation, 7, 7, 0xFFFF);
		} else if(overlay == OVERLAY_DIASHOW_SPEED) {
			//Progress Bar
			panel->drawRect(2, 2, 49, 7, 0xFFFF);

			//maps diashow_time range to progress bar width (max: 45px)
			//? floor(output_start + (slope * (input - input_start)) + 0.5)
			panel->fillRect(4, 4, floor((45 - slope_diashow * (diashow_time - 1000)) + .5), 3, 0xFFFF);
		
			//Sun icon
			panel->drawBitmap(54, 2, icon_diashow, 7, 7, 0xFFFF);
		} else if(overlay == OVERLAY_TEXT) {
			int16_t x1, y1;
			uint16_t width, height;
			panel->setTextColor(0xFFFF);
			panel->setTextSize(1);

			panel->getTextBounds(overlay_text, 0, 2, &x1, &y1, &width, &height);
			panel->setCursor(64 > width ? .5 * (64 - width) : 0, 9);
			panel->write(overlay_text);
		}
	}
}

void display_overlay(overlay_type type, const char* text = "") {
	overlay = type;

	switch(overlay) {
		case OVERLAY_TEXT:
			free(overlay_text);
			overlay_text = strdup(text);
			break;
		case OVERLAY_BRIGHTNESS:
			panel->setPanelBrightness(brightness);
			break;
	}

	ms_overlay = ms_current + 2000;
	display_change = true;
}

void display_menu() {
	//transparent background
	panel->clearScreen();

	panel->setTextSize(1);
	char * headline = nullptr;
	
	char clockText[10];
	int year = rtc_int.getYear() % 100;
	int month = rtc_int.getMonth() + 1;
	int day = rtc_int.getDay();
	int hour = rtc_int.getHour(true);
	int minute = rtc_int.getMinute();
	int second = rtc_int.getSecond();
	int arrowOffset = menu_selection == 0 ? 13 : 36;

	int selY = 0;
	switch(menu_selection) {
		case 0:
			selY = 12;
			break;
		case 1:
			selY = 23;
			break;
		case 2:
			selY = 34;
			break;
		case 3:
			selY = 45;
			break;
	}
	
	switch(menu) {
		case MENU_OVERVIEW:
			headline = strdup("Settings");

			//Draw menu points
			panel->setCursor(3, 21);
			panel->write("Clock Settings");
			panel->setCursor(3, 32);
			panel->write("Set Date/Time");
			panel->setCursor(3, 43);
			panel->write("Wifi Settings");

			//Draw selection box			
			panel->drawRect(1, selY, 62, 11, 0xFFFF);
			break;
		case MENU_CLOCK:
			headline = strdup("Clock Settings");

			//Draw menu points
			panel->setCursor(3, 21);
			panel->write("Show Seconds");
			panel->setCursor(3, 32);
			panel->write("Show Year");
			panel->setCursor(3, 43);
			panel->write("Show Blink");
			panel->setCursor(3, 54);
			panel->write("24h Format");

			//draw checkboxes
			if(clock_seconds)
				panel->fillRect(55, 15, 5, 5, 0xFFFF);
			else
				panel->drawRect(55, 15, 5, 5, 0xFFFF);

			if(clock_year)
				panel->fillRect(55, 26, 5, 5, 0xFFFF);
			else
				panel->drawRect(55, 26, 5, 5, 0xFFFF);

			if(clock_blink)
				panel->fillRect(55, 37, 5, 5, 0xFFFF);
			else
				panel->drawRect(55, 37, 5, 5, 0xFFFF);

			if(time_format24)
				panel->fillRect(55, 48, 5, 5, 0xFFFF);
			else
				panel->drawRect(55, 48, 5, 5, 0xFFFF);

			//Draw selection box			
			panel->drawRect(1, selY, 62, 11, 0xFFFF);
			break;
		case MENU_DATETIME:
			headline = strdup("Set Date/Time");

			panel->setTextSize(2);
			
			//time
			if(menu_time_changed) {
				year = menu_year;
				month = menu_month;
				day = menu_day;
				hour = menu_hour;
				minute = menu_minute;
				second = menu_second;
			}
			sprintf(clockText, "%02d.%02d.%02d", day, month, year);
			panel->setCursor(5, 32);
			panel->write(clockText);
			
			sprintf(clockText, "%02d:%02d:%02d", hour, minute, second);
			panel->setCursor(5, 55);
			panel->write(clockText);

			//arrows
			panel->drawBitmap(9, arrowOffset, icon_arrow_up, 6, 3, 0xFFFF);
			panel->drawBitmap(29, arrowOffset, icon_arrow_up, 6, 3, 0xFFFF);
			panel->drawBitmap(49, arrowOffset, icon_arrow_up, 6, 3, 0xFFFF);
			
			panel->drawBitmap(9, arrowOffset + 21, icon_arrow_down, 6, 3, 0xFFFF);
			panel->drawBitmap(29, arrowOffset + 21, icon_arrow_down, 6, 3, 0xFFFF);
			panel->drawBitmap(49, arrowOffset + 21, icon_arrow_down, 6, 3, 0xFFFF);

			panel->setTextSize(1);

			break;
		case MENU_WIFI:
			headline = strdup("Wifi Settings");

			//Draw menu points
			panel->setCursor(3, 21);
			panel->write("Connect");
			panel->setCursor(3, 32);
			panel->write("Connection Stats");
			panel->setCursor(3, 43);
			panel->write("Host");
			panel->setCursor(3, 54);
			panel->write("Host Stats");

			//draw checkboxes
			if(wifi_connect)
				panel->fillRect(55, 15, 5, 5, 0xFFFF);
			else
				panel->drawRect(55, 15, 5, 5, 0xFFFF);

			if(wifi_host)
				panel->fillRect(55, 37, 5, 5, 0xFFFF);
			else
				panel->drawRect(55, 37, 5, 5, 0xFFFF);

			//Draw selection box			
			panel->drawRect(1, selY, 62, 11, 0xFFFF);
			break;
		case MENU_WIFI_CONNECT:
			headline = strdup("Wifi Connection");

			//Draw menu points
			panel->setCursor(3, 21);
			panel->write(WiFi.localIP().toString().c_str());
			panel->setCursor(3, 32);
			panel->write(wifi_ssid);
			panel->setCursor(3, 43);
			panel->write("Connected");
			if(wifi_connect && wifi_setup_complete)
				panel->fillRect(55, 37, 5, 5, 0xFFFF);
			else
				panel->drawRect(55, 37, 5, 5, 0xFFFF);

			break;
		case MENU_WIFI_HOST:
			headline = strdup("Wifi Hosting");

			//Draw menu points
			panel->setCursor(3, 21);
			panel->write(WiFi.softAPIP().toString().c_str());
			panel->setCursor(3, 32);
			panel->write(wifi_ap_ssid);
			panel->setCursor(3, 43);
			panel->setTextWrap(true);
			panel->write(wifi_ap_password);
			panel->setTextWrap(false);

			break;
	}

	//display headline
	if(headline) {
		int16_t x1, y1;
		uint16_t width, height;
		panel->setTextColor(0xFFFF);

		panel->getTextBounds(headline, 0, 2, &x1, &y1, &width, &height);
		panel->setCursor(64 > width ? .5 * (64 - width) : 0, 9);
		panel->write(headline);
		free(headline);
	}

	display_overlay();

	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}

// Display static images on led matrix by pixelarray
void display_loaded_image() {
	for(int y = 0; y < 64; y++) {
		for(int x = 0; x < 64; x++) {
			panel->drawPixel(x, y, current_image[y][x]);
		}
	}

	display_overlay();

	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}

void display_next_frame() {
	if(image_index[selected_image].animated && animation.size() > 0) {
		animation_frame++;
		if(animation_frame >= animation.size())
			animation_frame = 0;
		std::vector<pixel_data> changes = animation[animation_frame];
		for(int i = 0; i < changes.size(); i++) {
			current_image[changes[i].y][changes[i].x] = changes[i].color;
		}
	}
}

void display_card_missing() {
	//transparent background
	panel->clearScreen();

	//social icon
	panel->drawBitmap(15, 15, icon_card, 32, 32, color_dark_read);

	display_overlay();

	//print on screen
	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}

//Displays twitch channel with respective viewers / subs
void display_social_channel(char* type, char* channel, char* subs, char* views) {

	//transparent background
	panel->clearScreen();

	//social icon
	if(strcmp(type, "t") == 0) {
			panel->drawBitmap(18, 2, icon_twitch, 28, 30, color_twitch);
	} else if (strcmp(type, "y") == 0) {
			panel->drawBitmap(16, 5, icon_youtube_base, 32, 24, color_youtube);
			panel->drawBitmap(28, 11, icon_youtube_play, 9, 12, 0xFFFF);
	}

	//text props
	int16_t x1, y1;
	uint16_t width, height;
	panel->setTextColor(0xFFFF);
	panel->setTextSize(1);

	panel->getTextBounds(channel, 0, 0, &x1, &y1, &width, &height);
	panel->setCursor(64 > width ? .5 * (64 - width) : 0, 42);
	panel->write(channel);

	int textbox_start_position = 0;
	int textbox_offset = 0;

	//follows - subs
	if(strcmp(subs, "") != 0 && strcmp(subs, "0") != 0) {
		if(strcmp(type, "t") == 0)
			textbox_offset = 10;
		else if (strcmp(type, "y") == 0)
			textbox_offset = 8;
		panel->getTextBounds(subs, 0, 0, &x1, &y1, &width, &height);
		textbox_start_position = 64 - textbox_offset > width ? .5 * (64 - textbox_offset - width) : 0;
		panel->setCursor(textbox_start_position + textbox_offset, 52);
		panel->write(subs);
	}

	//subs icon
	if(strcmp(type, "t") == 0)
		panel->drawBitmap(textbox_start_position, 45, icon_heart, 8, 7, color_twitch);
	else if (strcmp(type, "y") == 0)
		panel->drawBitmap(textbox_start_position, 45, icon_bell, 6, 7, 0xFFFF);

	//views
	if(strcmp(views, "") != 0 && strcmp(views, "0") != 0) {
		textbox_offset = 0;
		if(strcmp(type, "t") == 0)
			textbox_offset = 8;
		else if (strcmp(type, "y") == 0)
			textbox_offset = 11;
		panel->getTextBounds(views, 0, 0, &x1, &y1, &width, &height);
		textbox_start_position = 64 - textbox_offset > width ? .5 * (64 - textbox_offset - width) : 0;
		panel->setCursor(textbox_start_position + textbox_offset, 62);
		panel->write(views);

		//views icon
		if(strcmp(type, "t") == 0)
			panel->drawBitmap(textbox_start_position, 55, icon_person, 6, 7, color_coral);
		if(strcmp(type, "y") == 0)
			panel->drawBitmap(textbox_start_position, 55, icon_eye, 9, 7, 0xFFFF);
	}

	display_overlay();

	//print on screen
	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}

void display_clock_analog() {

	panel->clearScreen();

	//hands
	int hour = rtc_int.getHour();
	int minute = rtc_int.getMinute();

	//hour hand
	int hourX = (hour >= 6 ? 31 : 32);
	int hourY = (hour >= 9 || hour <= 3 ? 31 : 32);
	float radiant_hour = radians(180 - (30 * hour) - (minute / 2));
	panel->drawLine(hourX + 15 * sin(radiant_hour), hourY + 15 * cos(radiant_hour), hourX, hourY, 0xFFFF);
	
	//minute hand
	int minuteX = (minute >= 30 ? 31 : 32);
	int minuteY = (minute >= 45 || minute <= 15 ? 31 : 32);
	float radiant_minute = radians(180 - (6 * minute));
	panel->drawLine(minuteX + 23 * sin(radiant_minute), minuteY + 23 * cos(radiant_minute), minuteX, minuteY, 0xFFFF);
	
	//second hand
	if(clock_seconds) {
		int second = rtc_int.getSecond();
		int secondX = (second >= 30 ? 31 : 32);
		int secondY = (second >= 45 || second <= 15 ? 31 : 32);
		float radiant_second = radians(180 - (6 * second));
		panel->drawLine(secondX + 28 * sin(radiant_second), secondY + 28 * cos(radiant_second), secondX, secondY, 0xF800);
	}

	//marks
	for(int mark = 0; mark < 44; mark++) {
		panel->drawPixel(marks_clock[mark][0], marks_clock[mark][1], 0xFFFF);
	}

	display_overlay();

	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}

void display_clock_digital(bool withDate) {
	
	//transparent background
	panel->clearScreen();

	int hour = rtc_int.getHour(time_format24);
	int minute = rtc_int.getMinute();
	int second = rtc_int.getSecond();

	//text props
	panel->setTextColor(0xFFFF);
	panel->setTextSize(2);
	
	char clockText[10];
	int x_start;
	int y_start = 39;

	if(withDate) {
		int year = rtc_int.getYear();
		int month = rtc_int.getMonth() + 1;
		int day = rtc_int.getDay();
		y_start = 49;
		if(clock_year) {
			x_start = 5;
			sprintf(clockText, "%02d.%02d.%02d", day, month, year % 100);
		} else {
			x_start = 14;
			sprintf(clockText, "%02d.%02d.", day, month);
		}
		panel->setCursor(x_start, clock_seconds ? 29 : 25);
		panel->write(clockText);
	}

	
	if(clock_seconds) {
		x_start = 5;
		sprintf(clockText, "%02d:%02d:%02d", hour, minute, second);
	} else {
		x_start = 6;
		y_start += 4;
		panel->setTextSize(3);
		if(clock_blink && second % 2 == 1)
			sprintf(clockText, "%02d %02d", hour, minute);
		else
			sprintf(clockText, "%02d:%02d", hour, minute);
	}

	panel->setCursor( x_start, y_start);
	panel->write(clockText);
	panel->setTextSize(1);

	display_overlay();

	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}

void display_clock_big() {

	//transparent background
	panel->clearScreen();

	int hour = rtc_int.getHour(time_format24);
	int minute = rtc_int.getMinute();

	//text props
	panel->setFont(&Font4x5Fixed);
	panel->setTextColor(0xFFFF);
	panel->setTextSize(5);
	
	char clockText[4];

	sprintf(clockText, "%02d", hour);
	panel->setCursor(14, 25);
	panel->write(clockText);

	sprintf(clockText, "%02d", minute);
	panel->setCursor(14, 54);
	panel->write(clockText);

	panel->setTextSize(1);
	panel->setFont(&Font4x7Fixed);
	display_overlay();

	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}

void display_current() {
	if(menu != MENU_NONE) {
		display_menu();
		return;
	}

	if(current_mode != MODE_SOCIALS) {
		reset_loading();
	}

	switch(current_mode) {
		case MODE_SOCIALS:
			//go to next mode if wifi wont be connected
			if(!wifi_connect) {
				current_mode = MODE_IMAGES;
				display_change = true;
				break;
			}

			if(socials_channels.size() > 0) {
				reset_loading();
				if(socials_channels.size() < socials_channel_current)
					socials_channel_current = 0;

				display_social_channel(
					socials_channels[socials_channel_current].type,
					socials_channels[socials_channel_current].name,
					socials_channels[socials_channel_current].follower,
					socials_channels[socials_channel_current].views
				);
			} else {
				display_loading();
				display_overlay();
			}

			break;
		case MODE_CLOCK:
			switch(current_clock_mode) {
				case CLOCK_ANALOG:
					display_clock_analog();
					break;
				case CLOCK_DIGITAL:
					display_clock_digital(false);
					break;
				case CLOCK_DIGITAL_DATE:
					display_clock_digital(true);
					break;
				case CLOCK_DIGITAL_BIG:
					display_clock_big();
					break;
			}
			break;
		case MODE_IMAGES:
			if(image_loaded) {
				display_loaded_image();
			} else {
				display_card_missing();
			}
			break;
	}
}




/*****************
**	Interrupts  **
******************/

void IRAM_ATTR trigger_btn1() {
	btn1_pressed = true;
}

void IRAM_ATTR trigger_btn2() {
	btn2_pressed = true;
}

void IRAM_ATTR trigger_btn3() {
	if(digitalRead(GPIO_BTN3) == LOW) {
		ms_btn3_pressed = millis() + 10000;
	} else {
		btn3_released = ms_btn3_pressed != 0;
		ms_btn3_pressed = 0;
	}
}

void IRAM_ATTR trigger_rot1_a() {
	if(rot1_a_flag && digitalRead(GPIO_ROT1_B) == LOW) {
		rot1_a_flag = false;
		rot1_b_flag = false;
		rot1_clicks--;
	} else {
		rot1_b_flag = true;
	}
}

void IRAM_ATTR trigger_rot1_b() {
	if(rot1_b_flag && digitalRead(GPIO_ROT1_A) == LOW) {
		rot1_a_flag = false;
		rot1_b_flag = false;
		rot1_clicks++;
	} else {
		rot1_a_flag = true;
	}
}

void IRAM_ATTR trigger_rot1_btn() {
	rot1_pressed = true;
}


void IRAM_ATTR trigger_rot2_a() {
	if(rot2_a_flag && digitalRead(GPIO_ROT2_B) == LOW) {
		rot2_a_flag = false;
		rot2_b_flag = false;
		rot2_clicks--;
	} else {
		rot2_b_flag = true;
	}
}

void IRAM_ATTR trigger_rot2_b() {
	if(rot2_b_flag && digitalRead(GPIO_ROT2_A) == LOW) {
		rot2_a_flag = false;
		rot2_b_flag = false;
		rot2_clicks++;
	} else {
		rot2_a_flag = true;
	}
}

void IRAM_ATTR trigger_rot2_btn() {
	rot2_pressed = true;
}


void IRAM_ATTR trigger_rot3_a() {
	if(rot3_a_flag && digitalRead(GPIO_ROT3_B) == LOW) {
		rot3_a_flag = false;
		rot3_b_flag = false;
		rot3_clicks--;
	} else {
		rot3_b_flag = true;
	}
}

void IRAM_ATTR trigger_rot3_b() {
	if(rot3_b_flag && digitalRead(GPIO_ROT3_A) == LOW) {
		rot3_a_flag = false;
		rot3_b_flag = false;
		rot3_clicks++;
	} else {
		rot3_a_flag = true;
	}
}

void IRAM_ATTR trigger_rot3_btn() {
	rot3_pressed = true;
}




/************
**	Setup  **
*************/

// Show boot sequence
void setup_boot_sequence() {
	for(int row = 0; row < boot_sequence.size(); row++) {

		for(int pixel = 0; pixel < boot_sequence[row].pixels.size(); pixel++) {
			panel->drawPixel(boot_sequence[row].pixels[pixel], boot_sequence[row].row, 0xFFFF);
		}

		if(PANEL_DOUBLE_BUFFER) {
			panel->flipDMABuffer();

			for(int pixel = 0; pixel < boot_sequence[row].pixels.size(); pixel++) {
				panel->drawPixel(boot_sequence[row].pixels[pixel], boot_sequence[row].row, 0xFFFF);
			}
		}

		delay(7);
	}
}

// Initialize GPIO Pins
void gpio_setup() {

	//Turning off onboard led
	pinMode(48, PULLDOWN);

	//Setting PinModes
	pinMode(GPIO_BTN1, PULLUP);
	pinMode(GPIO_BTN2, PULLUP);
	pinMode(GPIO_BTN3, PULLUP);
	pinMode(GPIO_ROT1_A, PULLUP);
	pinMode(GPIO_ROT1_B, PULLUP);
	pinMode(GPIO_ROT1_BTN, PULLUP);
	pinMode(GPIO_ROT2_A, PULLUP);
	pinMode(GPIO_ROT2_B, PULLUP);
	pinMode(GPIO_ROT2_BTN, PULLUP);
	pinMode(GPIO_ROT3_A, PULLUP);
	pinMode(GPIO_ROT3_B, PULLUP);
	pinMode(GPIO_ROT3_BTN, PULLUP);

	//Setting interrupts
	attachInterrupt(GPIO_BTN1, trigger_btn1, FALLING);
	attachInterrupt(GPIO_BTN2, trigger_btn2, FALLING);
	attachInterrupt(GPIO_BTN3, trigger_btn3, CHANGE);
	attachInterrupt(GPIO_ROT1_A, trigger_rot1_a, FALLING);
	attachInterrupt(GPIO_ROT1_B, trigger_rot1_b, FALLING);
	attachInterrupt(GPIO_ROT1_BTN, trigger_rot1_btn, FALLING);
	attachInterrupt(GPIO_ROT2_A, trigger_rot2_a, FALLING);
	attachInterrupt(GPIO_ROT2_B, trigger_rot2_b, FALLING);
	attachInterrupt(GPIO_ROT2_BTN, trigger_rot2_btn, FALLING);
	attachInterrupt(GPIO_ROT3_A, trigger_rot3_a, FALLING);
	attachInterrupt(GPIO_ROT3_B, trigger_rot3_b, FALLING);
	attachInterrupt(GPIO_ROT3_BTN, trigger_rot3_btn, FALLING);

	//I2C
	Wire.setPins(GPIO_RTC_SDA, GPIO_RTC_SCL);
  	Wire.begin();

	//SPI
	SPI.begin(GPIO_SD_SCLK, GPIO_SD_MISO, GPIO_SD_MOSI, GPIO_SD_CS);
}

// Initialize LED Matrix
void panel_setup() {

	HUB75_I2S_CFG mxconfig(
		64,
		64,
		1,
		{
			GPIO_LEDPANEL_R1,
			GPIO_LEDPANEL_G1,
			GPIO_LEDPANEL_B1,
			GPIO_LEDPANEL_R2,
			GPIO_LEDPANEL_G2,
			GPIO_LEDPANEL_B2,
			GPIO_LEDPANEL_A,
			GPIO_LEDPANEL_B,
			GPIO_LEDPANEL_C,
			GPIO_LEDPANEL_D,
			GPIO_LEDPANEL_E,
			GPIO_LEDPANEL_LAT,
			GPIO_LEDPANEL_OE,
			GPIO_LEDPANEL_CLK
		},
		PANEL_DRIVER,
		PANEL_DOUBLE_BUFFER,
		PANEL_I2C_SPEED,
		PANEL_LATCH_BLINK,
		PANEL_CLOCK_PHASE
	);

	// Display Setup
	panel = new MatrixPanel_I2S_DMA(mxconfig);
	panel->begin();
	panel->clearScreen();
	panel->setPanelBrightness(brightness);
	panel->setFont(&Font4x7Fixed);
	panel->setTextSize(1);
	panel->setTextWrap(false);

	if(PANEL_DOUBLE_BUFFER) {
		panel->flipDMABuffer();
		panel->clearScreen();
	}

}

// Server setup
void server_setup() {
	if(WiFi.getMode() != WIFI_MODE_NULL) {
		
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
		server.begin();
		server.onNotFound([](AsyncWebServerRequest *request) {
			if(request->method() == HTTP_OPTIONS)
				request->send(200);
			else if(wifi_host) {
				if(sd_connected() && SD.exists("/webserver/index.html")) {
					request->send(SD, "/webserver/index.html", String());
				} else {
					request->send(200, "text/plain", "SD Card or Files missing");
				}
			} else
				request->send(404, "application/json", String());
		});

		//API trigger
		server.on("/api/display", HTTP_GET, [](AsyncWebServerRequest * request) {
			if(verify_api_key(request)) {
				AsyncJsonResponse *response = new AsyncJsonResponse();
				response->setContentType("application/json");
				response->setCode(200);
				JsonVariant& root = response->getRoot();

				root["displayMode"] = current_mode;
				root["brightness"] = brightness;
				root["version"] = VERSION;
				root["freeMemory"] = ESP.getFreeHeap();
				root["refreshRate"] = panel->calculated_refresh_rate;
				root["animation"] = animation_enabled;
				root["animationTime"] = animation_time;
				root["diashow"] = diashow_enabled;
				root["diashowTime"] = diashow_time;
				root["diashowModes"] = diashow_modes;

				response->setLength();
				request->send(response);
			} else {
				request->send(403, "application/json", "{}");
			}
		});

		server.on("/api/socials", HTTP_GET, [](AsyncWebServerRequest * request) {
			if(verify_api_key(request)) {
				AsyncJsonResponse *response = new AsyncJsonResponse();
				response->setContentType("application/json");
				response->setCode(200);
				JsonVariant& root = response->getRoot();

				root["request"] = socials_request;
				root["server"] = socials_request_server;
				root["apiKey"] = socials_api_key;
				root["displayChannel"] = socials_channel_current;

				response->setLength();
				request->send(response);
			} else {
				request->send(403, "application/json", "{}");
			}
		});

		server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest * request) {
			if(verify_api_key(request)) {
				AsyncJsonResponse *response = new AsyncJsonResponse();
				response->setContentType("application/json");
				response->setCode(200);
				JsonVariant& root = response->getRoot();

				root["online"] = ms_current;
				root["time"] = rtc_int.getLocalEpoch();
				root["displayMode"] = current_clock_mode;
				root["seconds"] = clock_seconds;
				root["year"] = clock_year;
				root["blink"] = clock_blink;
				root["format24"] = time_format24;
				root["externalRTCConnected"] = rtc_ext_enabled;
				root["updateTime"] = update_time;
				root["ntpServer"] = ntp_server;
				root["timezone"] = timezone;

				response->setLength();
				request->send(response);
			} else {
				request->send(403, "application/json", "{}");
			}
		});

		server.on("/api/wifi", HTTP_GET, [](AsyncWebServerRequest * request) {
			if(verify_api_key(request)) {
				AsyncJsonResponse *response = new AsyncJsonResponse();
				response->setContentType("application/json");
				response->setCode(200);
				JsonVariant& root = response->getRoot();

				root["wifiAP"] = wifi_host;
				root["wifiAPSSID"] = wifi_ap_ssid;
				root["wifiAPPassword"] = wifi_ap_password;
				root["wifiAPIP"] = WiFi.softAPIP();

				root["wifiConnect"] = wifi_connect;
				root["wifiSetupComplete"] = wifi_setup_complete;
				root["wifiSSID"] = wifi_ssid;
				root["wifiIP"] = WiFi.localIP();
				root["wifiHostname"] = WiFi.getHostname();
				
				response->setLength();
				request->send(response);
			} else {
				request->send(403, "application/json", "{}");
			}
		});

		server.on("/api/refresh", HTTP_POST, [](AsyncWebServerRequest * request) {
			if(verify_api_key(request)) {
				display_change = true;
				request->send(200, "application/json");
			} else {
				request->send(403, "application/json");
			}
		});

		server.on("/api/apiKey", HTTP_DELETE, [](AsyncWebServerRequest * request) {
			if(verify_api_key(request)) {
				preferences.begin(PREFERENCES_NAMESPACE, false);
				api_key = generate_uid();
				preferences.putString("api_key", api_key);
				preferences.end();
				request->send(200);
			} else {
				request->send(403, "application/json");
			}
		});

		server.on("/api/apiKey", HTTP_GET, [](AsyncWebServerRequest * request) {
			AsyncJsonResponse *response = new AsyncJsonResponse();
			response->setContentType("application/json");
			JsonVariant& root = response->getRoot();

			if(ms_api_key_approve > millis()) {
				response->setCode(200);
				root["apiKey"] = api_key;
				ms_api_key_approve = 0;
				ms_api_key_request = 0;
			} else {
				ms_api_key_request = millis() + 5000;
				response->setCode(204);
			}
			
			response->setLength();
			request->send(response);
		});

		server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest * request) {
			if(verify_api_key(request)) {
				preferences.begin(PREFERENCES_NAMESPACE, false);
				preferences.clear();
				preferences.end();
				request->send(200);
				requested_restart = true;
			} else {
				request->send(403, "application/json");
			}
		});

		//API Data processors
		server.addHandler(new AsyncCallbackJsonWebHandler("/api/time", [](AsyncWebServerRequest * request, JsonVariant &json) {
			if(verify_api_key(request)) {
				JsonObject body = json.as<JsonObject>();
				preferences.begin(PREFERENCES_NAMESPACE);
				
				if(body.containsKey("seconds") && body["seconds"].is<bool>()) {
					clock_seconds = body["seconds"].as<bool>();
					preferences.putBool("clock_seconds", clock_seconds);
				}
				if(body.containsKey("blink") && body["blink"].is<bool>()) {
					clock_blink = body["blink"].as<bool>();
					preferences.putBool("clock_blink", clock_blink);
				}
				if(body.containsKey("year") && body["year"].is<bool>()) {
					clock_year = body["year"].as<bool>();
					preferences.putBool("clock_year", clock_year);
				}
				if(body.containsKey("format24") && body["format24"].is<bool>()) {
					time_format24 = body["format24"].as<bool>();
					preferences.putBool("time_format24", time_format24);
				}
				if(body.containsKey("updateTime") && body["updateTime"].is<bool>()) {
					update_time = body["updateTime"].as<bool>();
					preferences.putBool("update_time", update_time);
				}
				if(body.containsKey("timezone") && body["timezone"].is<const char*>()) {
					free(timezone);
					timezone = strdup(body["timezone"]);
					preferences.putString("timezone", timezone);
					setenv("TZ", timezone, 1);
					tzset();
				}
				if(body.containsKey("ntpServer") && body["ntpServer"].is<const char*>()) {
					free(ntp_server);
					ntp_server = strdup(body["ntpServer"]);
					preferences.putString("ntpServer", ntp_server);
				}
				if(body.containsKey("displayMode") && body["displayMode"].is<int>()) {
					current_mode = MODE_CLOCK;
					preferences.putInt("current_mode", current_mode);
					current_clock_mode = body["displayMode"] > CLOCK_TYPE_NUMBER ? CLOCK_ANALOG : static_cast<clock_type>(body["displayMode"]);
					preferences.putInt("clock_mode", current_clock_mode);
				}
				if(body.containsKey("time") && body["time"].is<int>() && body["time"] > 0) {
					rtc_int.setTime(body["time"]);
					update_time = false;
					preferences.putBool("update_time", false);
					if(rtc_ext_enabled) {
						ms_rtc_ext_adjust = millis() + 1000;
						rtc_ext_adjust = true;
					}
				}

				//get current time from ntp server if wanted with new settings
				if(update_time && WiFi.waitForConnectResult(50) == WL_CONNECTED) {
					configTzTime(timezone, ntp_server);
					ms_rtc_ext_adjust = millis() + 10000;
					rtc_ext_adjust = true;
				}
				
				preferences.end();
				display_change = true;
				request->send(200, "application/json");
			} else {
				request->send(403, "application/json");
			}
		}));
		server.addHandler(new AsyncCallbackJsonWebHandler("/api/display", [](AsyncWebServerRequest * request, JsonVariant &json) {
			if(verify_api_key(request)) {
				JsonObject body = json.as<JsonObject>();
				preferences.begin(PREFERENCES_NAMESPACE);
				
				if(body.containsKey("displayMode") && body["displayMode"].is<int>()) {
					current_mode = body["displayMode"] > DISPLAY_MODE_NUMBER ? MODE_IMAGES : static_cast<display_mode>(body["displayMode"]);
					preferences.putInt("current_mode", current_mode);
				}
				if(body.containsKey("brightness") && body["brightness"].is<int>()) {
					brightness = body["brightness"] > 248 ? 248 : body["brightness"] < 16 ? 16 : body["brightness"];
					preferences.putUInt("brightness", brightness);
					panel->setBrightness(brightness);
				}
				
				preferences.end();
				display_change = true;
				request->send(200, "application/json");
			} else {
				request->send(403, "application/json");
			}
		}));

		//webserver
		server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
			if(sd_connected() && SD.exists("/webserver/index.html")) {
				request->send(SD, "/webserver/index.html", String());
			} else {
				request->send(200, "text/plain", "SD Card or Files missing");
			}
		});

		server.on("/interface.version.json", HTTP_GET, [](AsyncWebServerRequest *request) {
			if(sd_connected() && SD.exists("/webserver/interface.version.json")) {
				request->send(SD, "/webserver/interface.version.json", String());
			} else {
				request->send(200, "text/plain", "SD Card or Files missing");
			}
		});

		server.serveStatic("/", SD, "/webserver").setDefaultFile("index.html").setCacheControl("max-age=600");
	}
}

// Initialize Wifi if enabled
void wifi_setup() {

	WiFi.setHostname(WIFI_HOSTNAME);
	WiFi.setAutoReconnect(true);
	WiFi.disconnect(true);

	WiFi.softAPdisconnect(true);
	
	ms_wifi_reconnect = millis() + 60000;
	wifi_setup_complete = true;
	
	dns_server.stop();
	dns_server.setErrorReplyCode(DNSReplyCode::NoError);

	if(wifi_connect) {
		WiFi.begin(wifi_ssid, wifi_password);
		WiFi.waitForConnectResult(500);
		wifi_setup_complete = false;
	}
	
	if(wifi_host) {
		WiFi.softAP(wifi_ap_ssid, wifi_ap_password);
		dns_server.start(53, "*", WiFi.softAPIP());
	}
}

// Load preferences from flash
void preferences_load() {

	preferences.begin(PREFERENCES_NAMESPACE, false);
	
	//settings
	if(preferences.isKey("brightness"))
		brightness = preferences.getShort("brightness", brightness);
	if(preferences.isKey("current_mode"))
		current_mode = static_cast<display_mode>(preferences.getInt("current_mode", current_mode));
	if(preferences.isKey("selected_image"))
		selected_image = preferences.getInt("selected_image", selected_image);
	if(preferences.isKey("animation"))
		animation_enabled = preferences.getBool("animation", animation_enabled);
	if(preferences.isKey("diashow"))
		diashow_enabled = preferences.getBool("diashow", diashow_enabled);
	if(preferences.isKey("animation_time"))
		animation_time = preferences.getUInt("animation_time", animation_time);
	if(preferences.isKey("diashow_time"))
		diashow_time = preferences.getUInt("diashow_time", diashow_time);
	
	//wifi
	if(preferences.isKey("wifi_connect"))
		wifi_connect = preferences.getBool("wifi_connect", WIFI_CONNECT_DEFAULT);

	if(preferences.isKey("wifi_host"))
		wifi_host = preferences.getBool("wifi_host", WIFI_HOST_DEFAULT);
	
	if(preferences.isKey("wifi_ssid")) {
		free(wifi_ssid);
		wifi_ssid = strdup(preferences.getString("wifi_ssid", WIFI_SSID_DEFAULT).c_str());
	}
	
	if(preferences.isKey("wifi_ap_ssid")) {
		free(wifi_ap_ssid);
		wifi_ap_ssid = strdup(preferences.getString("wifi_ap_ssid", WIFI_AP_SSID_DEFAULT).c_str());
		}

	if(preferences.isKey("wifi_password")) {
		free(wifi_password);
		wifi_password = strdup(preferences.getString("wifi_password", WIFI_PASSWORD_DEFAULT).c_str());
		}

	if(preferences.isKey("wifi_ap_password")) {
		free(wifi_ap_password);
		wifi_ap_password = strdup(preferences.getString("wifi_ap_password", WIFI_AP_PASSWORD_DEFAULT).c_str());
	}

	//ntp
	if(preferences.isKey("ntpServer")) {
		free(ntp_server);
		ntp_server = strdup(preferences.getString("ntpServer", ntp_server).c_str());
	}

	if(preferences.isKey("timezone")) {
		free(timezone);
		timezone = strdup(preferences.getString("timezone", timezone).c_str());
	}

	if(preferences.isKey("update_time"))
		update_time = preferences.getBool("update_time", update_time);

	if(preferences.isKey("time_format24"))
		time_format24 = preferences.getBool("time_format24", time_format24);

	//Server
	if(preferences.isKey("api_key")) {
		free(api_key);
		api_key = strdup(preferences.getString("api_key", api_key).c_str());
	} else {
		free(api_key);
		api_key = strdup(generate_uid());
		preferences.putString("api_key", api_key);
	}

	//APIs
	if(preferences.isKey("socials_request")) {
		free(socials_request);
		socials_request = strdup(preferences.getString("socials_request", socials_request).c_str());
	}

	if(preferences.isKey("socials_api_key")) {
		free(socials_api_key);
		socials_api_key = strdup(preferences.getString("socials_api_key", socials_api_key).c_str());
	}

	if(preferences.isKey("socials_server")) {
		free(socials_request_server);
		socials_request_server = strdup(preferences.getString("socials_server", socials_request_server).c_str());
	}

	if(preferences.isKey("current_social"))
		socials_channel_current = preferences.getInt("current_social", socials_channel_current);

	//clock
	if(preferences.isKey("clock_mode"))
		current_clock_mode = static_cast<clock_type>(preferences.getInt("clock_mode", current_clock_mode));
	
	if(preferences.isKey("clock_seconds"))
		clock_seconds = preferences.getBool("clock_seconds", clock_seconds);
		
	if(preferences.isKey("clock_blink"))
		clock_blink = preferences.getBool("clock_blink", clock_blink);
		
	if(preferences.isKey("clock_year"))
		clock_year = preferences.getBool("clock_year", clock_year);


	preferences.end();
}

// Init rtc
void time_setup() {

	setenv("TZ", timezone, 1);
	tzset();
	if(rtc_ext.begin()) {
		rtc_ext.disable32K();
		rtc_ext.disableAlarm(1);
		rtc_ext.disableAlarm(2);
		rtc_ext.clearAlarm(1);
		rtc_ext.clearAlarm(2);
		rtc_ext_enabled = true;
		rtc_internal_adjust();
	}
}

// Reset input values before going into loop
void booted_setup() {
	btn1_pressed = false;
	btn2_pressed = false;
	ms_btn3_pressed = 0;
	btn3_released = false;
	rot1_pressed = false;
	rot2_pressed = false;
	rot3_pressed = false;
	rot1_clicks = 0;
	rot2_clicks = 0;
	rot3_clicks = 0;
	requested_restart = false;

	if(sd_connected() && image_index.size() > 0) {
		if(selected_image >= image_index.size())
			selected_image = 0;
		image_loaded = sd_load_image(image_index[selected_image]);
	}

	display_change = true;
	ms_current = millis();
	ms_animation = ms_current + animation_time;
	ms_diashow = ms_current + diashow_time;
}

void setup() {
	Serial.begin(9600); //todo remove

	preferences_load();
	panel_setup(); //depends on preferences
	setup_boot_sequence(); //depends on panel
	gpio_setup();
	time_setup(); //depends on gpio and preferences
	wifi_setup(); //depends on preferences
	server_setup(); //depends on preferences and gpio
	firmware_update(); //depends on gpio and panel
	
	booted_setup();
}




/***********
**	Loop  **
************/

void loop() {

	ms_current = millis();

	//display changes

	//diashow and animation routine
	if(menu == MENU_NONE) {
		if(current_mode == MODE_IMAGES) {
			if(diashow_enabled && ms_diashow < ms_current) {
				if(sd_connected() && image_index.size() > 0) {
					if(++selected_image >= image_index.size()) {
						selected_image = 0;
						if(diashow_modes && socials_channels.size() > 0) {
							current_mode = MODE_SOCIALS;
							socials_channel_current = 0;
						}
					}
					image_loaded = sd_load_image(image_index[selected_image]);
				} else {
					image_loaded = false;
				}
				ms_diashow = ms_current + diashow_time;
				display_change = true;
			}

			if(animation_enabled && image_loaded && ms_animation < ms_current) {
				display_next_frame();
				ms_animation = ms_current + animation_time;
				display_change = true;
			}
		} else if(current_mode == MODE_SOCIALS && diashow_enabled && ms_diashow < ms_current) {
			if(socials_channels.size() > 0 && ++socials_channel_current >= socials_channels.size()) {
				socials_channel_current = 0;
				if(diashow_modes && image_index.size() > 0) {
					current_mode = MODE_IMAGES;
					selected_image = 0;
				}
			}
			ms_diashow = ms_current + diashow_time;
			display_change = true;
		}
	}

	//animation speed rot
	if(rot1_clicks != 0) {
		if(menu == MENU_NONE) {
			animation_enabled = true;
			int16_t new_animation_time = animation_time - rot1_clicks * 20;
			animation_time = new_animation_time > 500 ? 500 : new_animation_time < 20 ? 20 : new_animation_time;
			ms_animation = ms_current + animation_time;
			display_overlay(OVERLAY_ANIMATION_SPEED);

			preferences.begin(PREFERENCES_NAMESPACE);
			preferences.putUInt("animation_time", animation_time);
			preferences.putBool("animation", animation_enabled);
			preferences.end();
		} else if(menu == MENU_DATETIME) {
			if(!menu_time_changed)
				menu_copy_time();

			int8_t new_time;
			
			switch(menu_selection) {
				case 0:
					new_time = menu_year + rot1_clicks;
					if(new_time > 99)
						menu_year = 0;
					else if(new_time < 0)
						menu_year = 99;
					else
						menu_year = new_time;
					menu_correct_date();
					break;
				case 1:
					new_time = menu_second + rot1_clicks;
					if(new_time > 59)
						menu_second = 0;
					else if(new_time < 0)
						menu_second = 59;
					else
						menu_second = new_time;
					break;
			}

			menu_time_changed = true;
		}

		display_change = true;
		rot1_clicks = 0;
	}

	//diashow speed rot
	if(rot2_clicks != 0) {
		if(menu == MENU_NONE) {
			diashow_enabled = true;
			int32_t new_diashow_time = diashow_time - rot2_clicks * 1000;
			diashow_time = new_diashow_time > 60000 ? 60000 : new_diashow_time < 1000 ? 1000 : new_diashow_time;
			ms_diashow = ms_current + diashow_time;
			display_overlay(OVERLAY_DIASHOW_SPEED);

			preferences.begin(PREFERENCES_NAMESPACE);
			preferences.putUInt("diashow_time", diashow_time);
			preferences.putBool("diashow", diashow_enabled);
			preferences.end();
		} else if(menu == MENU_DATETIME) {
			if(!menu_time_changed)
				menu_copy_time();

			int8_t new_time;
			
			switch(menu_selection) {
				case 0:
					new_time = menu_month + rot2_clicks;
					if(new_time > 12)
						menu_month = 1;
					else if(new_time < 1)
						menu_month = 12;
					else
						menu_month = new_time;
					menu_correct_date();
					break;
				case 1:
					new_time = menu_minute + rot2_clicks;
					if(new_time > 59)
						menu_minute = 0;
					else if(new_time < 0)
						menu_minute = 59;
					else
						menu_minute = new_time;
					break;
			}

			menu_time_changed = true;
		}

		display_change = true;
		rot2_clicks = 0;
	}

	//brightness rot
	if(rot3_clicks != 0) {
		if(menu != MENU_DATETIME) {
			int16_t new_brightness = brightness + rot3_clicks * 8;
			brightness = new_brightness > 248 ? 248 : new_brightness < 16 ? 16 : new_brightness;
			display_overlay(OVERLAY_BRIGHTNESS);

			preferences.begin(PREFERENCES_NAMESPACE);
			preferences.putUInt("brightness", brightness);
			preferences.end();
		} else {
			if(!menu_time_changed)
				menu_copy_time();

			int8_t new_time;
			
			switch(menu_selection) {
				case 0:
					new_time = menu_day + rot3_clicks;
					if(new_time > 31)
						menu_day = 1;
					else if(new_time < 1)
						menu_day = 31;
					else
						menu_day = new_time;
					menu_correct_date();
					break;
				case 1:
					new_time = menu_hour + rot3_clicks;
					if(new_time > 23)
						menu_hour = 0;
					else if(new_time < 0)
						menu_hour = 23;
					else
						menu_hour = new_time;
					break;
			}

			menu_time_changed = true;
		}

		display_change = true;
		rot3_clicks = 0;
	}

	//next button
	if(btn1_pressed) {
		if(menu == MENU_NONE) {			
			preferences.begin(PREFERENCES_NAMESPACE);
			if(current_mode == MODE_SOCIALS) {
				socials_channel_current = socials_channel_current + 1 >= socials_channels.size() ? 0 : socials_channel_current + 1;
				preferences.putInt("current_social", socials_channel_current);
			} else if(current_mode == MODE_CLOCK) {
				current_clock_mode = static_cast<clock_type>((current_clock_mode + 1) % CLOCK_TYPE_NUMBER);
				preferences.putInt("clock_mode", current_clock_mode);
			} else if(current_mode == MODE_IMAGES) {
				if(sd_connected() && image_index.size() > 0) {
					if(++selected_image >= image_index.size())
						selected_image = 0;
					image_loaded = sd_load_image(image_index[selected_image]);
				} else {
					image_loaded = false;
				}
				preferences.putInt("selected_image", selected_image);
			}
			preferences.end();
		} else {
			menu_selection++;
			if((menu == MENU_OVERVIEW && menu_selection > 2)
			|| (menu == MENU_CLOCK && menu_selection > 3)
			|| (menu == MENU_DATETIME && menu_selection > 1)
			|| (menu == MENU_WIFI && menu_selection > 3))
				menu_selection = 0;
		}

		display_change = true;
		btn1_pressed = false;
	}

	//mode button
	if(btn2_pressed) {
		if(menu == MENU_NONE) {
			current_mode = static_cast<display_mode>((current_mode + 1) % DISPLAY_MODE_NUMBER);
			if(current_mode == MODE_SOCIALS && !wifi_connect)
				current_mode = static_cast<display_mode>((current_mode + 1) % DISPLAY_MODE_NUMBER);

			if(current_mode == MODE_IMAGES) {
				if(sd_connected() && image_index.size() > 0) {
					if(selected_image >= image_index.size())
						selected_image = 0;
					image_loaded = sd_load_image(image_index[selected_image]);
				} else {
					image_loaded = false;
				}
			} else {
				animation.clear();
			}
			
			display_overlay(OVERLAY_TEXT, current_mode == MODE_IMAGES ? "Pictures" : current_mode == MODE_CLOCK ? "Clock" : "Socials");

			preferences.begin(PREFERENCES_NAMESPACE);
			preferences.putInt("current_mode", static_cast<int32_t>(current_mode));
			preferences.end();
		} else if(menu == MENU_OVERVIEW) {
			switch(menu_selection) {
				case 0:
					menu = MENU_CLOCK;
					break;
				case 1:
					menu = MENU_DATETIME;
					break;
				case 2:
					menu = MENU_WIFI;
					break;
			}
			menu_selection = 0;
		} else if(menu == MENU_CLOCK) {
			preferences.begin(PREFERENCES_NAMESPACE);
			switch(menu_selection) {
				case 0:
					clock_seconds = !clock_seconds;
					preferences.putBool("clock_seconds", clock_seconds);
					break;
				case 1:
					clock_year = !clock_year;
					preferences.putBool("clock_year", clock_year);
					break;
				case 2:
					clock_blink = !clock_blink;
					preferences.putBool("clock_blink", clock_blink);
					break;
				case 3:
					time_format24 = !time_format24;
					preferences.putBool("time_format24", time_format24);
					break;
			}
			preferences.end();
		} else if(menu == MENU_DATETIME) {
			menu_update_time();
		} else if(menu == MENU_WIFI) {
			preferences.begin(PREFERENCES_NAMESPACE);
			switch(menu_selection) {
				case 0:
					wifi_connect = !wifi_connect;
					preferences.putBool("wifi_connect", wifi_connect);
					wifi_setup();
					break;
				case 1:
					menu = MENU_WIFI_CONNECT;
					break;
				case 2:
					wifi_host = !wifi_host;
					preferences.putBool("wifi_host", wifi_host);
					wifi_setup();
					break;
				case 3:
					menu = MENU_WIFI_HOST;
					break;
			}
			preferences.end();
		}
		
		display_change = true;
		btn2_pressed = false;
	}

	//menu button
	if(ms_btn3_pressed != 0 && ms_btn3_pressed < ms_current) {
		ms_btn3_pressed = 0;

		preferences.begin(PREFERENCES_NAMESPACE, false);
		preferences.clear();
		preferences.end();
		requested_restart = true;
	} else if(btn3_released) {
		if(menu == MENU_NONE) {
			//approve api key for server if requested else open meu
			if(ms_api_key_request > ms_current) {
				ms_api_key_approve = ms_current + 5000;
				ms_api_key_request = 0;
			} else {
				menu = MENU_OVERVIEW;
			}
		} else if(menu == MENU_WIFI || menu == MENU_CLOCK) {
			menu = MENU_OVERVIEW;
		} else if(menu == MENU_DATETIME) {
			menu_update_time();
			menu = MENU_OVERVIEW;
		} else if(menu == MENU_WIFI_CONNECT || menu == MENU_WIFI_HOST) {
			menu = MENU_WIFI;
		} else {
			menu = MENU_NONE;
		}
		
		ms_animation = ms_current + animation_time;
		ms_diashow = ms_current + diashow_time;
		menu_selection = 0;
		display_change = true;
		btn3_released = false;
	}


	//rot1 button
	if(rot1_pressed) {
		if(menu == MENU_NONE) {
			animation_enabled = !animation_enabled;
			ms_animation = ms_current + animation_time;
			preferences.begin(PREFERENCES_NAMESPACE, false);
			preferences.putBool("animation", animation_enabled);
			preferences.end();
			
			display_overlay(OVERLAY_TEXT, animation_enabled ? "Animation ON" : "Animation OFF");
		}

		rot1_pressed = false;
	}

	//rot2 button
	if(rot2_pressed) {
		if(menu == MENU_NONE) {
			diashow_enabled = !diashow_enabled;
			ms_diashow = ms_current + diashow_time;
			preferences.begin(PREFERENCES_NAMESPACE, false);
			preferences.putBool("diashow", diashow_enabled);
			preferences.end();
			
			display_overlay(OVERLAY_TEXT, diashow_enabled ? "Diashow ON" : "Diashow OFF");
		}

		rot2_pressed = false;
	}

	//rot3 button
	if(rot3_pressed) {
		rot3_pressed = false;
	}


	//Check loading animation
	if(ms_loading != 0 && ms_loading < ms_current) {
		loading_step++;
		if(loading_step >= boot_sequence.size()) {
			loading_step = 0;
			loading_cycle = !loading_cycle;
		}
		ms_loading = ms_current + 20;
		display_change = true;
	}

	//check overlay time
	if(ms_overlay != 0 && ms_overlay < ms_current) {
		overlay = OVERLAY_NONE;
		ms_overlay = 0;
		display_change = true;
	}

	//Clock refresh cycle
	if((current_mode == MODE_CLOCK || menu == MENU_DATETIME) && ms_clock < ms_current) {
		ms_clock = ms_current + 100;
		display_change = true;
	}

	//Wifi routine
	if(!wifi_setup_complete && ms_wifi_routine <= ms_current) {
		ms_wifi_routine = ms_current + 5000;
		if(WiFi.waitForConnectResult(5) == WL_CONNECTED) {
			wifi_setup_complete = true;
			wifi_on_connected();
		} else if(ms_wifi_reconnect <= ms_current) {
			wifi_setup();
		}
	}

	//DNS Routine
	if(wifi_host) {
		dns_server.processNextRequest();
	}

	//RTC adjustment after message received
	if(rtc_ext_adjust && ms_rtc_ext_adjust <= ms_current) {
		rtc_external_adjust();
	}

	//Execute reset from api request
	if(requested_restart) {
		restart();
	}

	//Routine for async http
	if(menu == MENU_NONE) {
		if(socials_response_check != 0 && socials_response_check <= ms_current) {
			if(http_socials.readyState() == readyStateDone) {
				socials_response_check = 0;
				on_socials_response();
			} else {
				socials_response_check += 250;
			}
		} else if(current_mode == MODE_SOCIALS && ms_socials_request != 0 && ms_socials_request <= ms_current) {
			ms_socials_request = ms_current + SOCIAL_REFRESH_INTERVAL;
			socials_refresh();
		}
	}

	//refresh display if needed
	if(display_change) {
		display_change = false;
		display_current();
	}

	//TODO remove tests
	// if(ms_test < ms_current) {
	// 	ms_test = ms_current + 1000;
	// 	Serial.printf("PSRAM: %d, RAM: %d\n", ESP.getFreePsram(), ESP.getFreeHeap());
	// }
}