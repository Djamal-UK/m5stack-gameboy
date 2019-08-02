#include <stdio.h>
#include "timer.h"
#include "rom.h"
#include "mem.h"
#include "cpu.h"
#include "lcd.h"
#include "sdl.h"
#include "gbrom.h"


void setup() {
	// put your setup code here, to run once:
	sdl_init();
	rom_init((const unsigned char*)gb_rom);
	memm_init();
	cpu_init();

	lcd_thread_setup();

	while(true) {
		cpu_cycle();
		sdl_update();
		lcd_cycle();
		timer_cycle();
	}
}

void loop() {
	// put your main code here, to run repeatedly:
	cpu_cycle();
	sdl_update();
	lcd_cycle();
	timer_cycle();
}
