#include <M5Stack.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <freertos/queue.h>

//#include "SPI.h"
#include "spi_lcd.h"
#include "i2c_keyboard.h"
#include "sdl.h"

#define JOYPAD_INPUT 5
#define JOYPAD_ADDR  0x88

#define BIT(x, b) (((x)>>(b)) & 0x01)

#define GAMEBOY_WIDTH 160
#define GAMEBOY_HEIGHT 144
//byte pixels[GAMEBOY_HEIGHT * GAMEBOY_WIDTH / 4];
static byte* pixels = NULL;

static uint8_t btn_directions, btn_faces;

uint16_t palette[] = { 0xFFFF, 0xAAAA, 0x5555, 0x2222 };

QueueHandle_t fbqueue;

static void videoTask(void *arg) {
	byte* fb = NULL;
	int x = (320 - GAMEBOY_WIDTH)  >> 1;
	int y = (240 - GAMEBOY_HEIGHT) >> 1;
	while(true) {
		xQueueReceive(fbqueue, &fb, portMAX_DELAY);
		ili9341_write_frame(x, y, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, fb, true);
	}
}


byte getColorIndexFromFrameBuffer(int x, int y)
{
	int offset = x + y * 160;
	return (pixels[offset >> 2] >> ((offset & 3) << 1)) & 3;
}

void sdl_init(void)
{
	// LCDEnable, SDEnable, SerialEnable, I2CEnable
	//M5.begin(false, true, true, true);
	//M5.Speaker.end();
	//Serial.begin(115200);
	//dacWrite(SPEAKER_PIN, 0);
	ledcDetachPin(SPEAKER_PIN);
	pinMode(JOYPAD_INPUT, INPUT_PULLUP);
	i2c_keyboard_master_init();
	pixels = (byte*)calloc(GAMEBOY_HEIGHT, GAMEBOY_WIDTH);
	const unsigned int pal[] = {0xEFFFDE, 0xADD794, 0x525F73, 0x183442}; // Default greenscale palette
	sdl_set_palette(pal);
	ili9341_init();
	fbqueue = xQueueCreate(1, sizeof(byte*));
	//xTaskCreatePinnedToCore(&videoTask, "videoTask", 2048, NULL, 5, NULL, 1);
}

int sdl_update(void)
{
	if(digitalRead(JOYPAD_INPUT) == LOW) {
		uint8_t btns = ~(i2c_keyboard_read());
		btn_faces = (btns >> 4);
		btn_directions = (BIT(btns, 1) << 3) | (BIT(btns, 0) << 2) | (BIT(btns, 2) << 1) | (BIT(btns, 3));
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

uint16_t* sdl_get_palette(void)
{
	return palette;
}

void sdl_set_palette(const unsigned int* col)
{
	// RGB888 -> RGB565
	for (int i = 0; i < 4; ++i) {
		palette[i] = ((col[i]&0xFF)>>3)+((((col[i]>>8)&0xFF)>>2)<<5)+((((col[i]>>16)&0xFF)>>3)<<11);
	}
}

void sdl_render_framebuffer(void)
{
	int x = (320 - GAMEBOY_WIDTH)  >> 1;
	int y = (240 - GAMEBOY_HEIGHT) >> 1;
	ili9341_write_frame(x, y, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, pixels, true);
}

void sdl_frame(void)
{
	byte* screen = pixels;
	xQueueSend(fbqueue, &screen, 0);
}
