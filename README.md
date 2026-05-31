# Mp3-player

A custom-built, open-source portable music player inspired by the classic Apple iPod design.
Features:
Play MP3 files from a MicroSD card.
Intuitive navigation with a capacitive touch click wheel and center button.
Clear display on a 2.42-inch OLED screen.
High-quality audio output via a PCM5102A DAC and 3.5mm headphone jack.
USB-C charging and robust power management.
Bluetooth streaming capabilities (planned/future).
Long battery life: over 30 hours.

Hardware Components:
ESP32-WROOM-32E (Main MCU)
PCM5102A DAC Module (Audio Codec)
SSD1309 2.42 inch OLED Display
MicroSD Card Breakout Module
TP4056 USB-C LiPo Charger Module
MT3608 DC-DC Step Up Converter
MPR121 Capacitive Touch Sensor Breakout (for Click Wheel)
Tactile Push Button Switch (Center Button)
PJ-307 3.5mm Audio Jack
LiPo Battery 3.7V 4000mAh
Custom 3D Printed Enclosure and Internal Mounts

Software/Firmware:
Developed using ESP-IDF or Arduino framework.
Custom UI for navigation and music playback.
I2S audio driver for DAC.
SPI driver for MicroSD card.
I2C driver for OLED and MPR121 touch IC.
Bluetooth audio profile (A2DP sink) for future streaming.

Build Instructions:
1. 3D print all mechanical enclosure parts.
2. Assemble internal frame and mount all electronic modules using M2 screws and standoffs.
3. Wire all electrical connections as per the provided schematics.
4. Integrate the OLED display, click wheel components, and headphone jack into the enclosure.
5. Secure the LiPo battery and connect to the power management circuit.
6. Flash the ESP32 with the firmware.

Usage:
Load MP3 files onto a MicroSD card.
Insert MicroSD card into the player.
Navigate menus and select tracks using the click wheel and center button.
Connect headphones to the 3.5mm jack.
Charge via USB-C.

License: MIT License