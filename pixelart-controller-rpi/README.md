# <p align="center">Pixelart - Controller (Raspberry PI)</p>

## About

This is the kernel for the controller of the led matrix using a Raspberry Pi.

! I'm still in testing phase, so this is nowhere near ready to be used. I'm also not certain at this point if this is feasible and if I will continue working on it !

## Hardware compatibility

#### Hardware
For the kernel you need a Raspberry PI. This kernel was developed for the Pi Zero 2W specifically, but with modifications it should be working with other models, though not tested. Be aware of the performance of other models, when using less cores than the Pi Zero 2W (it has 4 Cores) that you might need to adjust the scheduler in this project.

#### SD Card and Real-Time-Clock
The project should work with any SPI SD-Card reader connected to the according pins and any I2C RTC Module. The device should work without both of these, but can't use the webinterface and displaying images if you are missing an sd card and you would need to set the clock manually every time you start the device or have it connected to wifi to update itself on start if you don't use any RTC module.

#### Button Inputs
The inputs consists of 3 Push buttons and 3 Rotary encoders with additional push functionality. If you want to use the firmware as is, you will need at least the 3 push buttons to enable wifi and authorize your webinterface connection. The rest can be configured via webinterface. If you plan to customize your firmware you can enable wifi by default (either host it or connect directly to your home network) and remove the menu button press for authorization of the webinterface to get rid of all inputs.

#### LED Matrix
For the display, I used a 64x64 LED Matrix connected via HUB75E interface. Although you should be able to use any HUB75 display, you will have to customize a big part of the code when using other dimensions, since I hardcoded most of the bitmaps and positions to that resolution.

## Wiring

//TODO Pinout

The SD Card is wired using SPI interface

The RTC Module uses I2C interface

The LED Matrix is connected via HUB75E (Note the additional E pin since this is a 64x64 panel with 1/32 scan)

## Installation

//TODO Installation

## Deployment

1. Copy the generated files under to the Pi's SD Card (FAT32 Filesystem)
1. Rename the `config32.txt`, or `config64.txt` if you are using 64bit, to `config.txt`
1. Copy the kernel*.img to the SD-Card as well