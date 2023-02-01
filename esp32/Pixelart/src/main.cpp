#include <main.h>
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

MatrixPanel_I2S_DMA *panel = nullptr;

//TODO populate with real boot sequence
uint8_t bootimg[12288] = {};

// Display static images on led matrix by pixelarray
void panel_display_static(uint8_t *pixels, bool overwrite = true) {

	if(overwrite)
		panel->clearScreen();

	int i = 0;
	while(i + 2 < sizeof(pixels)) {
		
		int y = (i/3) % PANEL_X;
		int x = (i/3 - y) / PANEL_X;
		panel->drawPixelRGB888(x, y, pixels[i], pixels[i+1], pixels[i+2]);
		i+=3;

	}
}

// Initialize LED Matrix
void panel_setup() {

	HUB75_I2S_CFG mxconfig(
		PANEL_X,
		PANEL_Y,
		PANEL_CHAIN,
		{
			PANEL_GPIO_R1,
			PANEL_GPIO_G1,
			PANEL_GPIO_B1,
			PANEL_GPIO_R2,
			PANEL_GPIO_G2,
			PANEL_GPIO_B2,
			PANEL_GPIO_A,
			PANEL_GPIO_B,
			PANEL_GPIO_C,
			PANEL_GPIO_D,
			PANEL_GPIO_E,
			PANEL_GPIO_LAT,
			PANEL_GPIO_OE,
			PANEL_GPIO_CLK
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
	panel->setPanelBrightness(128); //TODO Add hardware and save in eeprom

}

// Try connect to existing wifi network
void wifi_connect() {

}

// Host own wifi network
void wifi_host() {

}

// Initialize Wifi if enabled
void wifi_setup() {

}

void setup() {

	Serial.begin(9600); //TODO remove serial connection

	panel_setup();
	panel_display_static(bootimg);

}

void loop() {

}