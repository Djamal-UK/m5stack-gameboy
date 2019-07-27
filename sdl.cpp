//#include <M5Stack.h>

#include "SPI.h"
#include "spi_lcd.h"
#include "sdl.h"

#define JOYPAD_INPUT 5
#define JOYPAD_ADDR  0x88

#define BIT(x, b) (((x)>>(b)) & 0x01)

// void backlighting(bool state) {
	// if (state) {
		// M5.Lcd.setBrightness(0xF0);
	// } else {
		// M5.Lcd.setBrightness(0x20);
	// }
// }

#define GAMEBOY_WIDTH 160
#define GAMEBOY_HEIGHT 144
byte pixels[GAMEBOY_HEIGHT * GAMEBOY_WIDTH / 4];

static uint8_t btn_directions, btn_faces;

unsigned int palette[] = {0x183442, 0x525F73, 0xADD794, 0xEFFFDE};

byte getColorIndexFromFrameBuffer(int x, int y)
{
	int offset = x + y * 160;
	return (pixels[offset >> 2] >> ((offset & 3) << 1)) & 3;
}

void SDL_Flip(byte *screen)
{
	// Too slow
	//int i,j;
	//M5.Lcd.fillScreen(BLACK);
	/*for(i = 0;i<GAMEBOY_HEIGHT;i++){
		for(j = 0;j<GAMEBOY_WIDTH;j++){
			M5.Lcd.drawPixel(j, i, palette[getColorIndexFromFrameBuffer(j, i)]);
		}
	}*/
	ili9341_write_frame(0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, screen);
}

void sdl_init(void)
{
	// LCDEnable, SDEnable, SerialEnable, I2CEnable
	//M5.begin(false, true, true, true);
	//M5.Speaker.end();
	Serial.begin(115200);
	Wire.begin(21, 22);
	pinMode(JOYPAD_INPUT, INPUT_PULLUP);
	const unsigned int pal[] = {0xEFFFDE, 0xADD794, 0x525F73, 0x183442}; // Default greenscale palette
	sdl_set_palette(pal);
	ili9341_init();
	//backlighting(true);
}

int sdl_update(void)
{
	if(digitalRead(JOYPAD_INPUT) == LOW) {
		Wire.requestFrom(JOYPAD_ADDR, 1);
		while(Wire.available()) {
			uint8_t btns = ~(Wire.read());
			btn_faces = (btns >> 4);
			btn_directions = (BIT(btns, 1) << 3) | (BIT(btns, 0) << 2) | (BIT(btns, 2) << 1) | (BIT(btns, 3));
			//Serial.println(btn_faces);
			//Serial.println(btn_directions);
		}
	}
	return 0;
}

unsigned int sdl_get_buttons(void)
{
	return btn_faces;
}

unsigned int sdl_get_directions(void)
{
	return btn_directions;
}

byte* sdl_get_framebuffer(void)
{
	return pixels;
}

void sdl_clear_framebuffer(byte col)
{
	memset(pixels, col, sizeof(pixels));
}

void sdl_clear_screen(byte col)
{
	//M5.Lcd.fillScreen(palette[col]);
	//M5.Lcd.clear();
}

unsigned int* sdl_get_palette(void)
{
	return palette;
}

void sdl_set_palette(const unsigned int* col)
{
	// RGB888 -> RGB565
	for (int i = 0; i < 4; ++i) {
		unsigned int c = ((col[i]&0xFF)>>3)+((((col[i]>>8)&0xFF)>>2)<<5)+((((col[i]>>16)&0xFF)>>3)<<11);
		palette[i] = c;
	}
}

void sdl_frame(void)
{
	SDL_Flip(pixels);
}
