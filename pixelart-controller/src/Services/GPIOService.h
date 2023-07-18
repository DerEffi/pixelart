#pragma once

#include <Models/Config.h>
#include <SPI.h>

/*
*   Service for handling
*       - init of GPIO Pins
*       - init of hardware interfaces (spi/i2c)
*       - hardware control
*       - hardware interrupts
*/

// status for saving inputs from gpio controls (interrupts)
struct ControlsStatus {
    bool btn1 = false;
    bool btn2 = false;
    bool btn3 = false;
    uint64_t btn3Hold = 0;
    bool btnRot1 = false;
    bool btnRot2 = false;
    bool btnRot3 = false;
    int8_t rot1 = 0;
    int8_t rot2 = 0;
    int8_t rot3 = 0;
};

//set controls flag of a triggered pin, when waiting for second interrupt
struct ControlsFlags {
    bool rot1A = false;
    bool rot1B = false;
    bool rot2A = false;
    bool rot2B = false;
    bool rot3A = false;
    bool rot3B = false;
};

class GPIOService {
    private:
        static ControlsFlags flags;
        static uint8_t btn3Pin;

        static void IRAM_ATTR btn1Interrupt();
        static void IRAM_ATTR btn2Interrupt();
        static void IRAM_ATTR btn3LowInterrupt();
        static void IRAM_ATTR btn3HighInterrupt();
        static void IRAM_ATTR btnRot1Interrupt();
        static void IRAM_ATTR btnRot2Interrupt();
        static void IRAM_ATTR btnRot3Interrupt();
        static void IRAM_ATTR rot1AInterrupt();
        static void IRAM_ATTR rot1BInterrupt();
        static void IRAM_ATTR rot2AInterrupt();
        static void IRAM_ATTR rot2BInterrupt();
        static void IRAM_ATTR rot3AInterrupt();
        static void IRAM_ATTR rot3BInterrupt();
    public:
        static ControlsStatus status;
        SPIClass *spi;

        void init(const Config::GPIOConfig& config);
};