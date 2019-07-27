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

  rom_init(gb_rom);
  Serial.println("ROM OK!");
  memm_init();
  Serial.println("Mem OK!");
  cpu_init();
  Serial.println("CPU OK!");
}

void loop() {
  // put your main code here, to run repeatedly:
  sdl_update();
  
  cpu_cycle();
  lcd_cycle();
  timer_cycle();
}
