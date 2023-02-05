#include <main.h>
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Preferences.h>




/**************
**	Globals  **
***************/

//slopes for mapping ranges to pixel width
//? (output_end - output_start) / (input_end - input_start)
const double slope_brightness = .196;
const double slope_diashow = 0;
const double slope_animation = 0;

//enums
enum overlay_type {
	NONE,
	BRIGHTNESS,
	ANIMATION_SPEED,
	DIASHOW_SPEED,
	DISPLAY_MODE
};

//settings
Preferences preferences;
overlay_type volatile overlay = NONE;
uint8_t volatile brightness = 0;
bool volatile brightness_change = false;

//LED Panel
MatrixPanel_I2S_DMA *panel = nullptr;
uint16_t current_image[PANEL_X][PANEL_Y] = {};
//TODO populate with real boot sequence
uint16_t testimg[PANEL_X][PANEL_Y] = {{10502,10502,10502,10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,24021,24021,10432,10432,10432,6925,6925,24021,24021,24021,24021,24021,24021,24021,6925,24021,24021,24021,24021,24021,24021,6925,6925,10432,10432,10432,6925,6925,24021,24021,24021,24021,6925,6925,10432,10432,24021,24021,6925,6925,10432,10432,24021,6925,6925,10432,10432,10502,10502,10502},{10502,10502,10502,10432,6925,6925,6925,6925,6925,6925,6925,6925,24021,24021,10432,10432,10432,6925,6925,6925,24021,24021,24021,24021,24021,24021,24021,24021,24021,24021,24021,6925,24021,24021,24021,24021,6925,10432,10432,10432,6925,6925,6925,24021,24021,24021,6925,10432,10432,24021,6925,6925,6925,6925,10432,24021,6925,6925,10432,10432,10432,10502,10502,10502},{10502,10502,10432,6925,6925,6925,6925,24021,24021,6925,6925,6925,24021,10432,10432,6925,6925,6925,24021,24021,6925,6925,6925,6925,6925,24021,24021,24021,24021,24021,24021,24021,6925,24021,24021,24021,6925,6925,10432,10432,10432,6925,6925,24021,24021,6925,6925,10432,10432,10432,6925,6925,6925,10432,10432,24021,6925,6925,10432,10432,24021,10432,10502,10502},{10502,10502,10432,6925,6925,6925,6925,6925,24021,24021,24021,24021,24021,10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,24021,24021,24021,24021,24021,24021,24021,6925,10432,10432,10432,6925,6925,24021,6925,6925,10432,10432,10432,10432,10432,6925,6925,10432,24021,6925,6925,6925,10432,10432,24021,10432,10502,10502},{10502,10432,6925,6925,6925,6925,6925,6925,24021,10432,10432,24021,10432,10432,6925,6925,10432,10432,10432,10432,10432,10432,10432,10432,6925,6925,6925,6925,6925,6925,6925,24021,24021,24021,6925,24021,24021,6925,6925,10432,10432,6925,6925,24021,6925,6925,10432,10432,10432,10432,10432,10432,10432,10432,6925,6925,6925,10432,10432,10432,24021,6925,10432,10502},{10502,10432,6925,6925,6925,6925,6925,6925,6925,24021,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,196,196,196,10432,6925,6925,6925,6925,24021,24021,24021,6925,24021,196,196,196,10432,6925,6925,6925,6925,10432,10432,24021,24021,10432,10432,10432,10432,6925,6925,10432,10432,10432,10432,10432,24021,6925,10432,10502},{10502,10432,6925,6925,6925,6925,6925,6925,6925,24021,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,196,196,36365,57306,57306,196,10432,6925,6925,6925,6925,24021,24021,6925,196,57306,57306,57306,196,196,24021,6925,10432,10432,24021,24021,6925,6925,10432,10432,10432,10432,10432,10432,24021,10432,10432,10432,10432,24021,10432,10502},{10502,10432,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,196,36365,57306,57306,196,196,196,10432,10432,10432,6925,6925,6925,24021,24021,196,196,196,36365,36365,57306,196,6925,10432,10432,10432,24021,6925,6925,10432,10432,10432,10432,6925,6925,6925,24021,10432,10432,10432,24021,10432,10502},{10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,10432,10432,10432,10432,10432,10432,24021,24021,196,36365,57306,196,196,24021,10432,10432,10432,10432,10432,10432,10432,6925,6925,24021,6925,6925,6925,196,196,36365,57306,196,10432,10432,10432,10432,6925,10432,10432,10432,6925,6925,6925,6925,6925,24021,10432,10432,10432,24021,6925,10432},{10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,10432,10432,10432,24021,24021,24021,24021,196,36365,57306,196,24021,24021,24021,24021,24021,24021,10432,10432,10432,10432,10432,6925,6925,6925,6925,6925,10432,10432,196,36365,57306,196,24021,10432,10432,10432,10432,10432,6925,6925,6925,6925,6925,6925,24021,24021,10432,10432,24021,6925,10432},{10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,10432,24021,24021,24021,24021,24021,24021,13130,57306,196,6925,6925,6925,6925,6925,6925,6925,24021,24021,24021,10432,10432,10432,6925,24021,6925,6925,10432,10432,6925,196,57306,196,6925,6925,10432,10432,10432,6925,24021,24021,24021,24021,6925,6925,6925,24021,10432,10432,24021,6925,10432},{10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,24021,24021,24021,24021,24021,6925,13130,36365,57306,13130,10432,10432,10432,10432,10432,10432,6925,6925,6925,6925,24021,10432,10432,6925,6925,6925,6925,10432,10432,10432,13130,36365,57306,196,6925,10432,10432,6925,6925,6925,6925,6925,24021,24021,24021,24021,24021,10432,10432,10432,24021,10432},{10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,6925,24021,24021,6925,6925,6925,13130,36365,57306,13130,10432,10432,10432,10432,10432,10432,10432,10432,10432,6925,6925,24021,10432,10432,6925,6925,10432,10432,10432,10432,13130,36365,57306,196,10432,10432,6925,10432,10432,10432,10432,10432,6925,6925,6925,24021,24021,6925,10432,10432,24021,10432},{10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,10432,6925,6925,6925,10432,10432,13130,36365,57306,13130,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,10432,6925,24021,10432,10432,6925,10432,10432,10432,24021,13130,36365,57306,196,10432,10432,10432,6925,6925,24021,24021,10432,10432,10432,6925,6925,6925,10432,10432,10432,24021,64814},{10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,6925,10432,10432,10432,10432,10432,13130,57306,196,10432,10432,24021,24021,24021,24021,10432,10432,10432,10432,10432,10432,24021,10432,10432,6925,10432,10432,24021,6925,196,57306,196,10432,6925,6925,6925,6925,6925,6925,6925,24021,24021,10432,10432,10432,10432,10432,10432,24021,24021,64814},{10432,6925,6925,6925,6925,6925,10432,6925,6925,6925,6925,24021,10432,10432,10432,10432,10432,10432,10432,196,36365,57306,196,24021,24021,24021,24021,24021,24021,24021,24021,10432,10432,10432,6925,24021,10432,6925,10432,10432,10432,196,36365,57306,196,6925,6925,6925,6925,6925,6925,6925,6925,24021,24021,6925,10432,10432,10432,10432,24021,6925,64814,60237},{10432,6925,6925,6925,6925,6925,10432,6925,6925,6925,6925,24021,10432,10432,10432,10432,10432,24021,24021,24021,196,36365,57306,196,196,24021,24021,24021,24021,24021,24021,24021,10432,10432,10432,24021,10432,10432,10432,196,196,36365,57306,196,6925,6925,6925,24021,6925,6925,6925,6925,24021,24021,6925,6925,10432,10432,10432,24021,6925,64814,60237,10432},{10432,6925,6925,6925,6925,6925,10432,6925,10432,6925,6925,6925,24021,10432,10432,24021,24021,24021,24021,24021,6925,196,36365,57306,36365,13130,13130,196,196,196,196,196,196,196,196,196,196,13130,13130,36365,36365,57306,196,6925,6925,6925,6925,6925,6925,24021,24021,24021,24021,6925,6925,10432,10432,10432,24021,6925,6925,64814,60237,10432},{10432,6925,6925,6925,6925,6925,6925,10432,10432,6925,6925,6925,24021,10432,6925,6925,24021,24021,6925,24021,24021,24021,196,36365,57306,57306,13130,36365,57306,57306,57306,57306,57306,57306,57306,57306,36365,13130,57306,57306,57306,196,6925,6925,6925,6925,6925,6925,6925,6925,24021,24021,6925,6925,10432,10432,10432,10432,24021,6925,6925,64814,60237,10432},{10432,6925,6925,6925,6925,6925,6925,10432,6925,6925,6925,6925,24021,10432,10432,6925,6925,6925,24021,24021,24021,6925,196,13130,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,196,6925,6925,6925,6925,6925,6925,24021,24021,24021,6925,6925,10432,10432,10432,10432,24021,6925,6925,64814,60237,6925,10432},{10432,6925,6925,6925,6925,6925,10432,10432,6925,6925,6925,6925,24021,10432,10432,10432,6925,24021,24021,24021,6925,6925,10432,196,36365,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,196,6925,10432,10432,24021,24021,6925,24021,24021,6925,6925,6925,10432,10432,10432,10432,24021,6925,6925,64814,60237,6925,6925,10432},{10432,6925,6925,6925,6925,10432,10432,6925,6925,6925,10432,10432,10432,24021,10432,10432,6925,6925,24021,6925,6925,10432,10432,196,36365,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,196,10432,6925,10432,10432,24021,24021,24021,6925,6925,10432,10432,10432,10432,10432,10432,24021,6925,6925,64814,60237,6925,6925,10432},{10502,10432,6925,6925,6925,10432,6925,6925,10432,10432,10432,6925,6925,24021,10432,10432,10432,6925,6925,6925,10432,10432,24021,196,36365,57306,57306,196,196,57306,57306,57306,57306,57306,57306,196,196,57306,57306,57306,196,6925,24021,24021,6925,10432,6925,6925,6925,10432,10432,10432,10432,10432,10432,24021,6925,6925,64814,60237,60237,6925,10432,10502},{10502,10432,6925,6925,6925,10432,6925,6925,10432,6925,6925,6925,6925,6925,24021,10432,10432,10432,6925,10432,10432,24021,24021,196,36365,57306,196,196,196,196,57306,57306,57306,57306,196,196,196,196,57306,57306,196,6925,6925,24021,24021,6925,10432,10432,10432,10432,10432,24021,24021,6925,10432,24021,6925,6925,64814,60237,6925,6925,10432,10502},{10502,10432,6925,6925,6925,10432,10432,10432,6925,6925,6925,6925,6925,6925,24021,10432,24021,10432,10432,10432,6925,6925,6925,196,13130,57306,196,196,196,196,57306,57306,57306,57306,196,196,196,196,57306,57306,196,6925,6925,6925,24021,24021,6925,10432,10432,10432,24021,6925,6925,6925,6925,10432,24021,6925,64814,60237,6925,6925,10432,10502},{10502,10432,6925,6925,10432,10432,6925,6925,6925,6925,6925,6925,6925,24021,10432,10432,6925,24021,24021,10432,10432,6925,6925,196,13130,57306,196,196,196,196,57306,57306,57306,57306,196,196,196,196,57306,57306,196,6925,6925,6925,24021,24021,6925,10432,10432,10432,24021,6925,6925,6925,6925,6925,6925,6925,64814,60237,6925,6925,10432,10502},{10502,10502,10432,6925,10432,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,6925,6925,6925,6925,24021,10432,10432,6925,6925,196,57306,196,196,196,196,57306,57306,57306,57306,196,196,196,196,57306,196,6925,6925,24021,6925,24021,24021,10432,10432,10432,10432,24021,6925,6925,6925,6925,6925,6925,6925,64814,60237,6925,10432,10502,10502},{10502,10502,10432,10432,10432,6925,6925,6925,6925,6925,6925,6925,24021,6925,6925,6925,6925,6925,6925,6925,24021,10432,6925,6925,196,36365,57306,196,196,57306,57306,57306,57306,57306,57306,196,196,57306,57306,196,6925,6925,6925,24021,24021,6925,10432,10432,10432,24021,6925,6925,6925,6925,6925,6925,6925,6925,64814,60237,6925,10432,10502,10502},{10502,10502,10432,6925,6925,6925,6925,6925,6925,6925,6925,24021,10432,6925,6925,6925,6925,6925,6925,10432,24021,10432,6925,6925,196,13130,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,196,6925,6925,24021,24021,6925,10432,10432,10432,10432,24021,6925,6925,10432,6925,6925,6925,6925,6925,64814,60237,10432,10432,10502,10502},{10502,10502,10432,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,6925,10432,10432,24021,10432,10432,6925,196,196,36365,36365,57306,57306,57306,57306,57306,57306,57306,57306,57306,57306,196,6925,10432,24021,24021,6925,10432,10432,10432,10432,10432,24021,6925,6925,6925,10432,6925,6925,6925,6925,64814,60237,10432,10432,10502,10502},{10502,10502,10502,10432,6925,6925,6925,6925,6925,6925,10432,6925,6925,6925,6925,6925,6925,6925,10432,6925,6925,24021,10432,196,196,196,196,13130,13130,36365,36365,36365,36365,36365,36365,13130,13130,196,6925,6925,10432,6925,10432,10432,10432,10432,10432,10432,24021,6925,6925,6925,6925,6925,10432,10432,6925,6925,64814,60237,10432,10432,10502,10502},{10502,10502,10502,10432,6925,6925,6925,6925,6925,10432,6925,6925,6925,6925,6925,6925,6925,10432,10432,6925,6925,24021,10432,6925,196,196,196,196,196,196,196,196,196,196,196,196,6925,6925,6925,6925,10432,10432,10432,10432,10432,24021,10432,10432,24021,6925,6925,6925,6925,6925,6925,10432,10432,64814,60237,6925,10432,10432,10502,10502},{10502,10502,10502,10502,10432,6925,6925,6925,196,196,6925,6925,6925,6925,6925,6925,6925,10432,6925,6925,6925,24021,10432,10432,6925,196,196,196,196,196,196,196,196,196,196,196,196,6925,6925,6925,10432,10432,10432,24021,24021,6925,6925,10432,24021,6925,6925,6925,6925,6925,60237,60237,60237,64814,6925,10432,10432,10502,10502,10502},{10502,10502,10502,10502,196,196,196,6925,196,57306,196,196,6925,196,196,6925,6925,10432,6925,6925,10432,6925,24021,10432,196,196,196,196,196,8775,196,8775,8775,196,196,196,196,6925,6925,10432,10432,10432,24021,6925,6925,6925,6925,10432,24021,6925,6925,6925,60237,60237,64814,64814,64814,60237,10432,10432,10432,10502,10502,10502},{10502,10502,10502,196,196,57306,57306,196,36365,196,57306,196,196,196,57306,196,10432,10432,10432,10432,6925,6925,24021,10432,10432,196,196,196,13130,8775,8775,8775,8775,196,8775,196,196,196,6925,10432,10432,10432,24021,6925,6925,6925,6925,24021,6925,6925,6925,60237,64814,64814,64814,64814,64814,64814,60237,10432,10502,10502,10502,64814},{10502,196,196,57306,57306,196,57306,57306,196,36365,196,57306,57306,57306,57306,196,10432,6925,6925,6925,6925,6925,6925,24021,10432,6925,196,8775,13130,8775,13130,8775,8775,8775,8775,196,8775,196,10432,10432,10432,24021,6925,6925,6925,6925,6925,10432,6925,6925,10432,60237,64814,64814,60237,60237,64814,64814,60237,64814,64814,64814,64814,60237},{10502,10502,13130,196,196,13130,36365,36365,196,36365,36365,36365,57306,196,57306,196,196,6925,6925,6925,6925,6925,6925,24021,10432,196,13130,8775,13130,8775,13130,13130,8775,13130,13130,196,8775,8775,196,24021,6925,6925,6925,6925,6925,6925,10432,6925,6925,6925,60237,64814,64814,60237,60237,60237,64814,64814,60237,60237,60237,60237,60237,10502},{196,13130,36365,196,57306,57306,36365,196,196,36365,36365,36365,196,57306,196,196,57306,196,6925,6925,6925,6925,6925,6925,24021,196,8775,13130,13130,8775,13130,8775,13130,13130,196,8775,8775,13130,196,24021,6925,6925,6925,6925,10432,6925,10432,6925,6925,6925,60237,64814,64814,60237,60237,64814,64814,60237,10502,10502,10502,10502,10502,10502},{10502,196,196,57306,57306,36365,36365,196,57306,36365,196,196,196,57306,196,57306,57306,196,6925,6925,6925,6925,6925,6925,6925,196,8775,13130,8775,13130,13130,8775,13130,13130,196,8775,8775,13130,196,6925,6925,6925,6925,6925,6925,10432,10432,10432,6925,60237,60237,64814,64814,64814,64814,64814,60237,60237,60237,60237,60237,10502,10502,10502},{196,57306,36365,196,57306,36365,36365,36365,196,36365,196,36365,57306,57306,57306,57306,57306,196,6925,6925,6925,6925,6925,6925,196,13130,8775,13130,8775,13130,13130,8775,13130,13130,196,8775,8775,13130,8775,196,6925,6925,6925,6925,6925,6925,6925,6925,60237,64814,60237,60237,64814,64814,60237,60237,64814,64814,64814,64814,64814,60237,60237,10502},{57306,36365,36365,196,57306,36365,36365,196,57306,36365,36365,36365,36365,36365,57306,57306,57306,57306,196,6925,6925,6925,6925,6925,196,8775,13130,13130,8775,13130,8775,13130,13130,196,8775,13130,8775,13130,13130,196,6925,6925,6925,6925,6925,6925,6925,6925,60237,60237,60237,10432,60237,60237,64814,64814,64814,64814,64814,64814,64814,64814,64814,60237},{57306,36365,36365,36365,196,36365,36365,196,196,36365,36365,36365,36365,36365,36365,57306,57306,57306,196,6925,6925,6925,6925,6925,196,8775,13130,8775,13130,13130,8775,13130,13130,196,8775,13130,13130,8775,13130,196,6925,6925,6925,6925,6925,6925,6925,60237,64814,60237,10432,10502,10502,60237,64814,64814,64814,64814,64814,64814,64814,64814,64814,64814},{196,196,196,196,57306,36365,36365,36365,196,57306,36365,36365,196,196,36365,36365,57306,196,196,6925,6925,6925,6925,6925,196,13130,13130,8775,13130,13130,8775,13130,196,8775,13130,8775,13130,8775,13130,196,6925,6925,6925,6925,6925,6925,6925,60237,64814,60237,60237,60237,60237,64814,64814,64814,60237,60237,64814,64814,64814,64814,64814,64814},{19086,196,196,196,196,196,196,196,196,57306,57306,36365,196,196,196,36365,196,196,10432,6925,6925,6925,6925,196,13130,8775,8775,13130,13130,8775,13130,13130,196,8775,13130,8775,13130,8775,13130,13130,196,6925,6925,6925,6925,10432,60237,64814,60237,60237,64814,64814,60237,64814,64814,60237,60237,60237,60237,64814,64814,64814,64814,64814},{19086,19086,19086,196,196,196,196,196,196,196,196,36365,36365,196,196,36365,196,196,19086,10432,10432,10432,6925,196,8775,13130,13130,8775,8775,13130,13130,196,196,8775,13130,8775,13130,13130,8775,8775,196,196,10432,10432,10432,60237,64814,60237,64814,64814,60237,60237,60237,64814,64814,60237,60237,60237,60237,64814,64814,64814,64814,64814},{19086,16843,16843,16843,196,196,196,196,196,196,196,196,196,36365,36365,36365,196,19086,19086,19086,19086,19086,10432,196,13130,8775,8775,13130,13130,13130,196,196,196,196,13130,13130,8775,13130,13130,13130,196,196,19086,19086,19086,60237,60237,64814,60237,60237,16843,16843,60237,64814,64814,60237,60237,60237,60237,60237,64814,64814,64814,64814},{16843,16843,16843,16843,196,16843,196,196,196,196,196,196,196,196,196,196,19086,19086,19086,19086,19086,19086,19086,196,8775,13130,13130,13130,196,196,196,196,196,196,196,196,13130,8775,8775,13130,196,196,196,19086,19086,19086,60237,60237,60237,16843,16843,16843,16843,60237,64814,64814,60237,60237,60237,60237,64814,64814,64814,64814},{16843,16843,16843,16843,16843,16843,196,16843,196,16843,196,16843,196,16843,16843,16843,19086,19086,19086,19086,19086,19086,19086,196,13130,196,196,196,196,196,196,196,196,196,196,196,196,196,196,8775,196,196,196,196,19086,19086,16843,16843,16843,16843,19086,19086,16843,60237,64814,64814,64814,64814,60237,64814,64814,64814,64814,60237},{16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,19086,19086,19086,19086,19086,19086,19086,19086,196,196,19086,196,196,196,196,196,196,196,196,196,196,196,196,19086,196,196,19086,196,196,196,19086,19086,19086,19086,19086,19086,19086,16843,60237,64814,64814,64814,64814,64814,64814,64814,64814,64814,60237},{19086,19086,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,196,196,196,196,196,16843,16843,196,196,196,196,196,16843,19086,19086,19086,19086,19086,196,196,19086,19086,19086,19086,19086,19086,19086,16843,60237,60237,64814,64814,64814,64814,64814,60237,60237,16843},{19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,16843,16843,16843,196,196,196,16843,16843,16843,16843,16843,16843,196,196,196,16843,16843,16843,16843,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,16843,16843,60237,60237,60237,60237,60237,16843,16843,16843},{19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,16843,16843,16843,16843,196,196,16843,16843,16843,16843,16843,16843,16843,16843,196,196,16843,16843,16843,16843,16843,16843,16843,16843,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,16843,16843,16843,16843,16843,16843,16843,19086},{19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086},{19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,16843,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086},{19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086},{19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086},{19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086,19086},{14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730,14730},{10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502},{10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502},{10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502},{10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502},{10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502},{10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502,10502}};

//Interrupt flags
bool volatile rot1_a_flag = false;
bool volatile rot1_b_flag = false;
bool volatile rot2_a_flag = false;
bool volatile rot2_b_flag = false;
bool volatile rot3_a_flag = false;
bool volatile rot3_b_flag = false;

//Timers
hw_timer_t * timer_overlay = NULL;




/**************
**	Display  **
**************/

//Display overlay menu for io switches
void display_overlay() {
	
	if(overlay != NONE) {
		//Background
		panel->fillRect(0, 0, 64, 11, 0);
		
		if(overlay == BRIGHTNESS) {
			//Progress Bar
			panel->drawRect(2, 2, 49, 7, 0xFFFF);

			//maps brightness range to progress bar width
			//? floor(output_start + (slope * (input - input_start)) + 0.5)
			panel->fillRect(4, 4, floor((1 + slope_brightness * (brightness - 24)) + .5), 3, 0xFFFF);
		
			//Sun icon
			panel->drawRect(56, 4, 3, 3, 0xFFFF);

			panel->drawPixel(54, 5, 0xFFFF);
			panel->drawPixel(60, 5, 0xFFFF);
			panel->drawPixel(57, 2, 0xFFFF);
			panel->drawPixel(57, 8, 0xFFFF);

			panel->drawPixel(55, 3, 0xFFFF);
			panel->drawPixel(55, 7, 0xFFFF);
			panel->drawPixel(59, 3, 0xFFFF);
			panel->drawPixel(59, 7, 0xFFFF);

			panel->drawPixel(57, 5, 0xFFFF);
		} else if(overlay == ANIMATION_SPEED) {

		} else if(overlay == DIASHOW_SPEED) {

		} else if(overlay == DISPLAY_MODE) {

		}
	}
}

// Display static images on led matrix by pixelarray
void display_full(uint16_t pixels[PANEL_X][PANEL_Y], bool save_image = true) {
	for(int y = 0; y < PANEL_X; y++) {
		for(int x = 0; x < PANEL_Y; x++) {
			if(save_image)
				current_image[y][x] = pixels[y][x];
			panel->drawPixel(x, y, pixels[y][x]);
		}
	}

	display_overlay();

	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}

//! Not working
//Display only changes of pixels for the next frame
void display_frame(int size, uint16_t *pixels[3]) {

	//TODO display current image

	for(int i = 0; i < size / 3; i++) {
		panel->drawPixel(pixels[i][0], pixels[i][1], pixels[i][2]);
	}

	display_overlay();

	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();
}




/***********
**	Wifi  **
************/

// Try connect to existing wifi network
void wifi_connect() {
	//TODO TBD
}

// Host own wifi network
void wifi_host() {
	//TODO TBD
}




/***************
**	Settings  **
****************/

void IRAM_ATTR enable_overlay(overlay_type type) {
	overlay = type;
	timerRestart(timer_overlay);
	timerAlarmEnable(timer_overlay);
}

void IRAM_ATTR disable_overlay() {
	overlay = NONE;
	timerAlarmDisable(timer_overlay);
	display_full(current_image, false);
}

void IRAM_ATTR set_brightness(uint8_t value, bool show_overlay) {
	brightness = value;
	brightness_change = true;
	if(show_overlay)
		enable_overlay(BRIGHTNESS);
}




/*****************
**	Interrupts  **
******************/

void IRAM_ATTR trigger_btn1() {
}

void IRAM_ATTR trigger_btn2() {
}


void IRAM_ATTR trigger_rot1_a() {
	if(rot1_a_flag && digitalRead(GPIO_ROT1_B) == LOW) {
		rot1_a_flag = false;
		rot1_b_flag = false;
		//TODO Function Code turned left
	} else {
		rot1_b_flag = true;
	}
}

void IRAM_ATTR trigger_rot1_b() {
	if(rot1_b_flag && digitalRead(GPIO_ROT1_A) == LOW) {
		rot1_a_flag = false;
		rot1_b_flag = false;
		//TODO Function Code turned right
	} else {
		rot1_a_flag = true;
	}
}

void IRAM_ATTR trigger_rot1_btn() {
}


void IRAM_ATTR trigger_rot2_a() {
	if(rot2_a_flag && digitalRead(GPIO_ROT2_B) == LOW) {
		rot2_a_flag = false;
		rot2_b_flag = false;
		//TODO Function Code turned left
	} else {
		rot2_b_flag = true;
	}
}

void IRAM_ATTR trigger_rot2_b() {
	if(rot2_b_flag && digitalRead(GPIO_ROT2_A) == LOW) {
		rot2_a_flag = false;
		rot2_b_flag = false;
		//TODO Function Code turned right
	} else {
		rot2_a_flag = true;
	}
}

void IRAM_ATTR trigger_rot2_btn() {
}


void IRAM_ATTR trigger_rot3_a() {
	if(rot3_a_flag && digitalRead(GPIO_ROT3_B) == LOW) {
		rot3_a_flag = false;
		rot3_b_flag = false;

		set_brightness(brightness = (brightness & 0B11100000) == 0 ? 24 : brightness - 8, true);
	} else {
		rot3_b_flag = true;
	}
}

void IRAM_ATTR trigger_rot3_b() {
	if(rot3_b_flag && digitalRead(GPIO_ROT3_A) == LOW) {
		rot3_a_flag = false;
		rot3_b_flag = false;

		set_brightness((brightness & 0B11111000) == 248 ? 248 : brightness + 8, true);
	} else {
		rot3_a_flag = true;
	}
}

void IRAM_ATTR trigger_rot3_btn() {
}




/************
**	Setup  **
*************/

//Initialize GPIO Pins
void gpio_setup() {

	//Turning off onboard led
	pinMode(48, PULLDOWN);

	//Setting PinModes
	pinMode(GPIO_BTN1, PULLUP);
	pinMode(GPIO_BTN2, PULLUP);
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
	attachInterrupt(GPIO_ROT1_A, trigger_rot1_a, FALLING);
	attachInterrupt(GPIO_ROT1_B, trigger_rot1_b, FALLING);
	attachInterrupt(GPIO_ROT1_BTN, trigger_rot1_btn, FALLING);
	attachInterrupt(GPIO_ROT2_A, trigger_rot2_a, FALLING);
	attachInterrupt(GPIO_ROT2_B, trigger_rot2_b, FALLING);
	attachInterrupt(GPIO_ROT2_BTN, trigger_rot2_btn, FALLING);
	attachInterrupt(GPIO_ROT3_A, trigger_rot3_a, FALLING);
	attachInterrupt(GPIO_ROT3_B, trigger_rot3_b, FALLING);
	attachInterrupt(GPIO_ROT3_BTN, trigger_rot3_btn, FALLING);

}

// Initialize LED Matrix
void panel_setup() {

	HUB75_I2S_CFG mxconfig(
		PANEL_X,
		PANEL_Y,
		PANEL_CHAIN,
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
	panel->setPanelBrightness(brightness); //TODO Add hardware and save in eeprom

	if(PANEL_DOUBLE_BUFFER)
		panel->flipDMABuffer();

}

// Initialize Wifi if enabled
void wifi_setup() {
	//TODO TBD
}

//Load preferences from flash
void preferences_load() {
	preferences.begin(PREFERENCES_NAMESPACE, true);
	brightness = preferences.getShort("brightness", 0);
	preferences.end();
}

//Init timers
void timer_setup() {

	//Overlay Timer
	timer_overlay = timerBegin(3, 80, true);
	timerAttachInterrupt(timer_overlay, disable_overlay, true);
	timerAlarmWrite(timer_overlay, 3000000, false);

}

void setup() {
	Serial.begin(9600); //TODO remove after testing

	preferences_load();
	gpio_setup();
	timer_setup();
	panel_setup();
	wifi_setup();

	display_full(testimg);
}




/***********
**	Loop  **
************/

void loop() {
	if(brightness_change) {
		brightness_change = false;

		panel->setPanelBrightness(brightness);
		display_full(current_image, false);

		preferences.begin(PREFERENCES_NAMESPACE);
		preferences.putShort("brightness", brightness);
		preferences.end();
	}
	delay(50);
}