#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "lcd.h"
#include "cpu.h"
#include "interrupt.h"
#include "sdl.h"
#include "mem.h"

#define MODE2_BOUNDS 	(204/4)
#define MODE3_BOUNDS 	(284/4)
#define MODE0_BOUNDS 	(456/4)
#define SCANLINE_CYCLES	(456/4)

#define SET_MODE(mode) (lcd_stat = (lcd_stat & 0xFC) | (mode))

static unsigned char* mem;

static byte lcd_line;
static byte lcd_stat;
static byte lcd_ly_compare;
static uint32_t lcd_cycles;

static QueueHandle_t lcdqueue;

/* LCD STAT */
static byte ly_int;
static byte mode2_oam_int;
static byte mode1_vblank_int;
static byte mode0_hblank_int;
static byte ly_int_flag;
static byte lcd_mode;
static byte lcd_stat_tracker;

/* LCD Control */
struct cLCD {
	byte lcd_enabled = 1;
	byte lcd_line;
	byte window_tilemap_select;
	byte window_enabled;
	byte tilemap_select;
	byte bg_tiledata_select;
	byte sprite_size;
	byte sprites_enabled;
	byte bg_enabled;
	byte bg_palette;
	byte spr_palette1;
	byte spr_palette2;
	byte scroll_x;
	byte scroll_y;
	byte window_x;
	byte window_y;
};

static cLCD lcdc;

struct sprite {
	int y, x, tile, flags;
};

static sprite spr[10];
static int sprcount;

static byte bgpalette[] = {3, 2, 1, 0};
static byte sprpalette1[] = {0, 1, 2, 3};
static byte sprpalette2[] = {0, 1, 2, 3};

enum {
	PRIO  = 0x80,
	VFLIP = 0x40,
	HFLIP = 0x20,
	PNUM  = 0x10
};

static inline void lcd_match_lyc()
{
	ly_int_flag = (lcd_line == lcd_ly_compare);
	if(ly_int && ly_int_flag) {
		interrupt(INTR_LCDSTAT);
	}
}

void lcd_reset(void)
{
	xQueueReset(lcdqueue);
	espeon_clear_framebuffer(palette[0x00]);
	lcd_mode = 1;
	lcd_line = 0;
	lcd_cycles = 0;
}

unsigned char lcd_get_stat(void)
{
	return lcd_stat | (ly_int_flag<<2) | lcd_mode;
}

static inline void lcd_set_palettes(const cLCD& lcdc)
{
	byte n = lcdc.bg_palette;
	bgpalette[0] = (n>>0)&3;
	bgpalette[1] = (n>>2)&3;
	bgpalette[2] = (n>>4)&3;
	bgpalette[3] = (n>>6)&3;
	n = lcdc.spr_palette1;
	sprpalette1[1] = (n>>2)&3;
	sprpalette1[2] = (n>>4)&3;
	sprpalette1[3] = (n>>6)&3;
	n = lcdc.spr_palette2;
	sprpalette2[1] = (n>>2)&3;
	sprpalette2[2] = (n>>4)&3;
	sprpalette2[3] = (n>>6)&3;
}

void lcd_write_bg_palette(unsigned char n)
{
	lcdc.bg_palette = n;
}

void lcd_write_spr_palette1(unsigned char n)
{
	lcdc.spr_palette1 = n;
}

void lcd_write_spr_palette2(unsigned char n)
{
	lcdc.spr_palette2 = n;
}

void lcd_write_scroll_x(unsigned char n)
{
	lcdc.scroll_x = n;
}

void lcd_write_scroll_y(unsigned char n)
{
	lcdc.scroll_y = n;
}

int lcd_get_line(void)
{
	return lcd_line;
}

void lcd_write_stat(unsigned char c)
{
	ly_int                = !!(c & 0x40);
	mode2_oam_int         = !!(c & 0x20);
	mode1_vblank_int      = !!(c & 0x10);
	mode0_hblank_int      = !!(c & 0x08);
	
	lcd_stat = (c & 0xF8);
}

void lcd_write_control(unsigned char c)
{
	lcdc.bg_enabled            = !!(c & 0x01);
	lcdc.sprites_enabled       = !!(c & 0x02);
	lcdc.sprite_size           = !!(c & 0x04);
	lcdc.tilemap_select        = !!(c & 0x08);
	lcdc.bg_tiledata_select    = !!(c & 0x10);
	lcdc.window_enabled        = !!(c & 0x20);
	lcdc.window_tilemap_select = !!(c & 0x40);
	lcdc.lcd_enabled           = !!(c & 0x80);
	
	if(!lcdc.lcd_enabled) {
		lcd_reset();
	} else {
		lcd_match_lyc();
	}
}

void lcd_set_ly_compare(unsigned char c)
{
	lcd_ly_compare = c;
	if(lcdc.lcd_enabled)
		lcd_match_lyc();
}

void lcd_set_window_y(unsigned char n) {
	lcdc.window_y = n;
}

void lcd_set_window_x(unsigned char n) {
	lcdc.window_x = n;
}

static inline void sort_sprites(struct sprite *s, int n)
{
	int swapped, i;

	do
	{
		swapped = 0;
		for(i = 0; i < n-1; i++)
		{
			if(s[i].x < s[i+1].x)
			{
				sprite c = s[i];
				s[i] = s[i+1];
				s[i+1] = c;
				swapped = 1;
			}
		}
	}
	while(swapped);
}

static inline int scan_sprites(struct sprite *s, int line, int size)
{
	int i, c = 0;
	for(i = 0; i<40; i++)
	{
		int y, offs = i * 4;
	
		y = mem[0xFE00 + offs++] - 16;
		if(line < y || line >= y + 8+(size*8))
			continue;
	
		s[c].y     = y;
		s[c].x     = mem[0xFE00 + offs++]-8;
		s[c].tile  = mem[0xFE00 + offs++];
		s[c].flags = mem[0xFE00 + offs++];
		c++;
	
		if(c == 10)
			break;
	}
	return c;
}

static void draw_bg_and_window(fbuffer_t *b, int line, struct cLCD& lcdc)
{
	int x, offset = line * 160;
	bool windowVisible = line >= lcdc.window_y && lcdc.window_enabled && line - lcdc.window_y < 144;

	for(x = 0; x < 160; x++, offset++)
	{
		unsigned int map_select, map_offset, tile_num, tile_addr, xm, ym;
		unsigned char b1, b2, mask, colour;

		/* Convert LCD x,y into full 256*256 style internal coords */
		if(windowVisible && x + 7 >= lcdc.window_x)
		{
			xm = x + 7 - lcdc.window_x;
			ym = line - lcdc.window_y;
			map_select = lcdc.window_tilemap_select;
		}
		else {
			if(!lcdc.bg_enabled)
			{
				//b[offset] = 0;
				b[offset] = palette[bgpalette[0]];
				continue;
			}
			xm = (x + lcdc.scroll_x) & 0xFF;
			ym = (line + lcdc.scroll_y) & 0xFF;
			map_select = lcdc.tilemap_select;
		}

		/* Which pixel is this tile on? Find its offset. */
		/* (y/8)*32 calculates the offset of the row the y coordinate is on.
		 * As 256/32 is 8, divide by 8 to map one to the other, this is the row number.
		 * Then multiply the row number by the width of a row, 32, to find the offset.
		 * Finally, add x/(256/32) to find the offset within that row. 
		 */
		map_offset = (ym/8)*32 + xm/8;

		tile_num = mem[0x9800 + map_select*0x400 + map_offset];
		if(lcdc.bg_tiledata_select)
			tile_addr = 0x8000 + tile_num*16;
		else
			tile_addr = 0x9000 + ((signed char)tile_num)*16;

		b1 = mem[tile_addr+(ym&7)*2];
		b2 = mem[tile_addr+(ym&7)*2+1];
		mask = 128>>(xm&7);
		colour = (!!(b2&mask)<<1) | !!(b1&mask);
		
		//b[offset] = bgpalette[colour];
		b[offset] = palette[bgpalette[colour]];
	}
}

static void draw_sprites(fbuffer_t *b, int line, int nsprites, struct sprite *s, struct cLCD& lcdc)
{
	int i;
	for(i = 0; i < nsprites; i++)
	{
		unsigned int b1, b2, tile_addr, sprite_line, x, offset;

		/* Sprite is offscreen */
		if(s[i].x < -7)
			continue;

		/* Which line of the sprite (0-7) are we rendering */
		sprite_line = s[i].flags & VFLIP ? (lcdc.sprite_size ? 15 : 7)-(line - s[i].y) : line - s[i].y;

		/* Address of the tile data for this sprite line */
		tile_addr = 0x8000 + (s[i].tile*16) + sprite_line*2;

		/* The two bytes of data holding the palette entries */
		b1 = mem[tile_addr];
		b2 = mem[tile_addr+1];

		/* For each pixel in the line, draw it */
		offset = s[i].x + line * 160;
		for(x = 0; x < 8; x++, offset++)
		{
			unsigned char mask, colour;
			byte *pal;

			if((s[i].x + x) >= 160)
				continue;

			mask = s[i].flags & HFLIP ? 128>>(7-x) : 128>>x;
			colour = ((!!(b2&mask))<<1) | !!(b1&mask);
			if(colour == 0)
				continue;

			pal = (s[i].flags & PNUM) ? sprpalette2 : sprpalette1;
			
			/* Sprite is behind BG, only render over palette entry 0 */
			if(s[i].flags & PRIO)
			{
				if(b[offset] != palette[bgpalette[0]])
					continue;
			}
			
			//b[offset] = pal[colour];
			b[offset] = palette[pal[colour]];
		}
	}
}

static void render_line(void *arg)
{
	struct cLCD cline;
	fbuffer_t* b = sdl_get_framebuffer();
	//unsigned char* mem = mem_get_bytes();
	
	while(true) {
		if(!xQueueReceive(lcdqueue, &cline, portMAX_DELAY))
			continue;
		
		if(lcd_mode==1)
			continue;
		
		int line = cline.lcd_line;
		struct sprite s[10];
		
		lcd_set_palettes(cline);
		
		/* Draw the background layer */
		draw_bg_and_window(b, line, cline);
		
		/* Draw sprites */
		if(cline.sprites_enabled) {
			int sprcnt = scan_sprites(s, line, cline.sprite_size);
			if(sprcnt) sort_sprites(s, sprcnt);
			draw_sprites(b, line, sprcnt, s, cline);
		}

		if(line == 143) {
			sdl_end_frame();
		}
	}
}

void lcd_cycle(unsigned int cycles)
{	
	if(!lcdc.lcd_enabled)
		return;
	
	lcd_cycles += cycles;
  
	if(lcd_line >= 144) {
		if (lcd_mode != 1) {
			/* Mode 1: Vertical blanking */
			if (mode1_vblank_int) {
				interrupt(INTR_LCDSTAT);
			}
			interrupt(INTR_VBLANK);
			lcd_stat_tracker = 0;
			lcd_mode = 1;
			
			//sdl_end_frame();
			//sdl_frame();
		}
		if (lcd_cycles >= SCANLINE_CYCLES) {
			lcd_cycles -= SCANLINE_CYCLES;
			//lcd_cycles = 0;
			++lcd_line;
			if (lcd_line > 153) {
				lcd_line = 0;
			}
			lcd_match_lyc();
		}
	}
	else if(lcd_cycles < MODE2_BOUNDS) {
		if (lcd_mode != 2) {
			if (mode2_oam_int) {
				interrupt(INTR_LCDSTAT);
			}
			lcd_stat_tracker = 1;
			lcd_mode = 2;
			
			/* Mode 2: Scanning OAM for (X, Y) coordinates of sprites that overlap this line */
			// sprcount = scan_sprites(spr, lcd_line, lcdc.sprite_size);
			// if (sprcount) sort_sprites(spr, sprcount);
		}
	}
	else if(lcd_cycles < MODE3_BOUNDS) {
		if (lcd_mode != 3) {
			lcd_stat_tracker = 1;
			lcd_mode = 3;
			
			// send scanline early
			lcdc.lcd_line = lcd_line;
			xQueueSend(lcdqueue, &lcdc, 0);
			
			/* Mode 3: Reading OAM and VRAM to generate the picture */
			// fbuffer_t* b = sdl_get_framebuffer();
			// lcd_set_palettes(lcdc);
			// draw_bg_and_window(b, lcd_line, lcdc);
			// draw_sprites(b, lcd_line, sprcount, spr, lcdc);
		}
	}
	else if(lcd_cycles < MODE0_BOUNDS) {
		if (lcd_mode != 0) {
			/* Mode 0: Horizontal blanking */
			if (mode0_hblank_int) {
				interrupt(INTR_LCDSTAT);
			}
			lcd_stat_tracker = 3;
			lcd_mode = 0;
		}
	}
	else {
		++lcd_line;
		lcd_match_lyc();
		lcd_mode = 0;
		lcd_cycles -= SCANLINE_CYCLES;
		//lcd_cycles = 0;
	}
}

void lcd_init()
{
	mem = mem_get_bytes();
	lcdqueue = xQueueCreate(143, sizeof(cLCD));
	lcd_write_control(mem[0xFF40]);
	xTaskCreatePinnedToCore(&render_line, "renderScanline", 4096, NULL, 5, NULL, 0);
}
