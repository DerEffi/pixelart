# <p align="center">Pixelart - Controller</p>

## About

This is the firmware for the controller of the led matrix.

## Hardware compatibility

#### Microcontroller
For the firmware you need an ESP32-S3 specifically. For other versions either RAM or GPIO Pins are missing. I used a DevKit with the Onboard LED on Pin 48 for this project (DevkitC V1), by swapping RGB-LED Pin on your devboard with pin 48 in [main.h](./src/main.h) and your hardware you should be able to use different devkits as well.

#### SD Card and Real-Time-Clock
The project should work with any SPI SD-Card reader connected to the according pins and any I2C RTC Module. The device should work without both of these, but can't use the webinterface and displaying images if you are missing an sd card and you would need to set the clock manually every time you start the device or have it connected to wifi to update itself on start if you don't use any RTC module.

#### Button Inputs
The inputs consists of 3 Push buttons and 3 Rotary encoders with additional push functionality. If you want to use the firmware as is, you will need at least the 3 push buttons to enable wifi and authorize your webinterface connection. The rest can be configured via webinterface. If you plan to customize your firmware you can enable wifi by default (either host it or connect directly to your home network) and remove the menu button press for authorization of the webinterface to get rid of all inputs.

#### LED Matrix
For the display, I used a 64x64 LED Matrix connected via HUB75E interface. Although you should be able to use any HUB75 display, you will have to customize a big part of the code when using other dimensions, since I hardcoded most of the bitmaps and positions to that resolution.

## Wiring

If you have the components described above, you can wire them according to the GPIO Pin numbers in the `src/main.h` file. See [Customization](#customization) for more information on that below.

The SD Card is wired using SPI interface

The RTC Module uses I2C interface

The LED Matrix is connected via HUB75E (Note the additional E pin since this is a 64x64 panel with 1/32 scan)

The buttons/rotary encoders are wired with a denoising circuit with a schmitt-trigger-inverter (like the 74HC14) and therefore on PULLUP mode, so you would at least need to invert the buttons before wiring to the according pins. I don't have a circuit diagram for my exact components, but I used a similar approach as in [this article](https://mansfield-devine.com/speculatrix/2018/04/debouncing-fun-with-schmitt-triggers-and-capacitors/) just with swapped vcc and ground connections

If I ever do a hardware revision of this project I would probably get rid of the buttons directly wired to the microcontroller and rather use an IOExpander (like the MCP23017), which would only use 1 additional wire for an interrupt signal (and a CE wire in case of SPI) and would connect to one of the existing SPI or I2C interfaces instead of using 12 GPIO Pins. I am aware that this would make older versions incompatible with future updates!

I would recommend not soldering these wires directly to the ESP32, since there are some issues simultaneously using the USB/Serial interface and uploading new firmware to it as I had to use reserved and strapping Pins. More on that [here](http://wiki.fluidnc.com/en/hardware/ESP32-S3_Pin_Reference).

## Installation

Development is done using [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO extension](https://platformio.org/install/ide?install=vscode). The dependencies should be installed on your first build.

Under `src/` copy the `main.h.example` and rename it to `main.h`.

## Customization

Be aware that any changes you make might prevent you from using [Updates](#updates). For more information see below.

In the `src/main.h` file you can set the Pins used by the controller for the attached devices (SPI / I2C / HUB75E and Interrupts) as well as the factory settings for wifi, time and the socials API. If you plan to use the socials functions, make sure to deploy the socials-api app and setting the server and api key. The social channels can be configured by the webinterface later.

In the `platform.ini` file you can change the build option `DPIXEL_COLOUR_DEPTH_BITS` for the devkit-c profile. Even though the colors might not be optimal, the sweetspot for me was to set it to `5`. 6 or more requires the panel to run at a lower refresh rate, with induces the typical LED flickering on cameras and I think you can even see it by eye.

You can override the socials API Key and hosted Wifi password on your device by copying the two files in the `data` folder and removing `.example` from the filename. The configuration from these files are prioritised over everything else and therefore the only ones not changing with any updates you perform. If you want to update these values, edit the files and repeat the first step under [Deployment](#deployment).

## Deployment

1. Deploy the configuration files to your ESP32-S3 by selecting the build profile (`devkit-c` if you are using my provided configuration) in the PlatformIO menu. Under `Platform` first Build the Filesystem Image and then Upload it to your device connected via Serial/USB. This uploads the data folder to the devices filesystem.
1. Deploy the firmware under `General > Upload` the same way.

## Updates

! Updates will override any customization except from files of the `data` folder. User settings made via webinterface will be kept until factory reset !

Although I plan to revisit this project, since there are some issues I still want to resolve. I can't make any promises on when or if updates will be made or if the hardware configuration will stay the same, since there are some "janky" solutions in place right now, but if you plan to deploy this project yourself, you might be able to use the update functionalities either via OTA or SD-Card on your own by changing the update server.

When using your own update server, make sure to place the `version.json` file for the version metadata alongside your `firmware.bin` inside `www.your-update.server/firmware/`.

On more information on how to perform updates visit the webinterface startpage or [http://pixelart.dereffi.de/](http://pixelart.dereffi.de/#/#updates)