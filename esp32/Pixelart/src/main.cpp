#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define RES_X 64
#define RES_Y 64
#define COLOR_DEPTH 5

//MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA *dma_display = nullptr;

void setup() {

	Serial.begin(9600);

  // Module configuration
  HUB75_I2S_CFG mxconfig(
    RES_X,
    RES_Y,
    1,
	{
		25,		//R1
		26,		//G1
		27,		//B1
		14,		//R2
		12,		//G2
		13,		//B2
		23,		//A
		19,		//B
		5,		//C
		17,		//D
		18,		//E
		4,		//LAT
		15,		//OE
		16		//CLK
	}
  );

  mxconfig.latch_blanking = 1;
  mxconfig.clkphase = false;
  mxconfig.driver = HUB75_I2S_CFG::SHIFTREG;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness(128); //0-255
  dma_display->clearScreen();
  dma_display->fillScreen(dma_display->color565(255, 255, 255));

}

void loop() {
}