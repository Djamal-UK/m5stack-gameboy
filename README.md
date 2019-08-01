# esp32-gameboy

This is a port of https://github.com/zid/gameboy to the Espressif ESP32 chip.

# What do I need to use this?

You will need:
* A board containg an ESP32 chip and at least 4MB (32Mbit) of SPI flash, plus the tools to program it.
* A backup of a GameBoy ROM game cartridge
* A 320x240 ILI9341 display, controllable by a 4-wire SPI interface. You can find modules with this LCD by
looking for '2.2 inch SPI 320 240 ILI9341' on eBay or other shopping sites.
* A GPIO button

This fork is being developed with the M5Stack device in mind. You can find more info about it here: https://m5stack.com/
M5Stack Arduino API and M5Stack FACES are required.

# How do I program the chip?

* Set up your Arduino IDE for the M5Stack (https://docs.m5stack.com/)
* Drag and drop your rom to rom2h.bat
* Run esp32-gameboy.ino
* Compile the sketch and upload it to the board
