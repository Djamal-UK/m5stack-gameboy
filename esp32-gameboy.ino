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
	cpu_init();
	mmu_init();
	lcd_init();

	while(true) {
		unsigned int cycles = cpu_cycle();
		sdl_update();
		lcd_cycle(cycles);
		timer_cycle(cycles);
	}
}

void loop() {
	// put your main code here, to run repeatedly:
	unsigned int cycles = cpu_cycle();
	sdl_update();
	lcd_cycle(cycles);
	timer_cycle(cycles);
}
