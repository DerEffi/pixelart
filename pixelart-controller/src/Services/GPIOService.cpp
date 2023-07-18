#include <main.h>
#include <Services/GPIOService.h>

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <Models/Config.h>

ControlsStatus GPIOService::status;
ControlsFlags GPIOService::flags;
SPIClass GPIOService::*spi = NULL;
uint8_t GPIOService::btn3Pin = -1;

void IRAM_ATTR GPIOService::btn1Interrupt() { status.btn1 = true; }
void IRAM_ATTR GPIOService::btn2Interrupt() { status.btn2 = true; }
// start counting on press - stop and set status on release
void IRAM_ATTR GPIOService::btn3LowInterrupt() { bool current = digitalRead(btn3Pin); status.btn3 = current; status.btn3Hold = !current * millis(); }
void IRAM_ATTR GPIOService::btn3HighInterrupt() { bool current = digitalRead(btn3Pin); status.btn3 = !current; status.btn3Hold = current * millis(); }
void IRAM_ATTR GPIOService::btnRot1Interrupt() { status.btnRot1 = true; }
void IRAM_ATTR GPIOService::btnRot2Interrupt() { status.btnRot2 = true; }
void IRAM_ATTR GPIOService::btnRot3Interrupt() { status.btnRot3 = true; }
// rot interrupts: set flag on first interrupt - set status on second interrupt - reset flag of first interrupt
void IRAM_ATTR GPIOService::rot1AInterrupt() { flags.rot1A = !flags.rot1B; status.rot1 -= flags.rot1B; flags.rot1B = false; }
void IRAM_ATTR GPIOService::rot1BInterrupt() { flags.rot1B = !flags.rot1A; status.rot1 += flags.rot1A; flags.rot1A = false; }
void IRAM_ATTR GPIOService::rot2AInterrupt() { flags.rot2A = !flags.rot2B; status.rot2 -= flags.rot2B; flags.rot2B = false; }
void IRAM_ATTR GPIOService::rot2BInterrupt() { flags.rot2B = !flags.rot2A; status.rot2 += flags.rot2A; flags.rot2A = false; }
void IRAM_ATTR GPIOService::rot3AInterrupt() { flags.rot3A = !flags.rot3B; status.rot3 -= flags.rot3B; flags.rot3B = false; }
void IRAM_ATTR GPIOService::rot3BInterrupt() { flags.rot3B = !flags.rot3A; status.rot3 += flags.rot3A; flags.rot3A = false; }

void GPIOService::init(const Config::GPIOConfig& config) {
    LOG("Info", "Configuring GPIO interfaces");

    btn3Pin = config.controls.btn3;

    if(config.controls.expander.enabled) {
        //todo ioexpander
    } else {
        //todo software debounce
        if(config.controls.btn1 >= 0) {
            pinMode(config.controls.btn1, config.controls.pullup ? PULLUP : PULLDOWN);
            attachInterrupt(config.controls.btn1, btn1Interrupt, config.controls.pullup ? FALLING : RISING);
        }
        if(config.controls.btn2 >= 0) {
            pinMode(config.controls.btn2, config.controls.pullup ? PULLUP : PULLDOWN);
            attachInterrupt(config.controls.btn2, btn2Interrupt, config.controls.pullup ? FALLING : RISING);
        }
        if(config.controls.btn3 >= 0) {
            pinMode(config.controls.btn3, config.controls.pullup ? PULLUP : PULLDOWN);
            attachInterrupt(config.controls.btn3, config.controls.pullup ? btn3LowInterrupt : btn3HighInterrupt, CHANGE);
        }
        if(config.controls.rot1.btn >= 0) {
            pinMode(config.controls.rot1.btn, config.controls.pullup ? PULLUP : PULLDOWN);
            attachInterrupt(config.controls.rot1.btn, btnRot1Interrupt, config.controls.pullup ? FALLING : RISING);
        }
        if(config.controls.rot2.btn >= 0) {
            pinMode(config.controls.rot2.btn, config.controls.pullup ? PULLUP : PULLDOWN);
            attachInterrupt(config.controls.rot2.btn, btnRot2Interrupt, config.controls.pullup ? FALLING : RISING);
        }
        if(config.controls.rot3.btn >= 0) {
            pinMode(config.controls.rot3.btn, config.controls.pullup ? PULLUP : PULLDOWN);
            attachInterrupt(config.controls.rot3.btn, btnRot3Interrupt, config.controls.pullup ? FALLING : RISING);
        }
    }

    //I2C
    if(config.i2c.clk >= 0 && config.i2c.sda >= 0) {
        Wire.setPins(config.i2c.clk, config.i2c.sda);
        Wire.begin();
    }

	//SPI
    if(config.spi.clk >= 0 && config.spi.miso >= 0 && config.spi.mosi >= 0) {
        spi = new SPIClass(HSPI);
        spi->begin(config.spi.clk, config.spi.miso, config.spi.mosi);
    }
}