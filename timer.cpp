#include "timer.h"
#include "interrupt.h"
#include "cpu.h"
#include "mem.h"

static unsigned int ticks;
static unsigned char tac;
static unsigned char started;
static unsigned int speed = 1024;
static unsigned int counter;
static uint16_t divider;
static unsigned char modulo;

void timer_reset_div(void)
{
	divider = 0;
}

unsigned char timer_get_div(void)
{
	return (divider >> 8);
}

void timer_set_counter(unsigned char v)
{
	counter = v;
}

unsigned char timer_get_counter(void)
{
	return counter;
}

void timer_set_modulo(unsigned char v)
{
	modulo = v;
}

unsigned char timer_get_modulo(void)
{
	return modulo;
}

void timer_set_tac(unsigned char v)
{
	const int speeds[] = {1024, 16, 64, 256};
	tac = v;
	started = v&4;
	if (speed != speeds[v&3])
		ticks = 0;
	speed = speeds[v&3];
}

unsigned char timer_get_tac(void)
{
	return tac;
}

void timer_cycle(unsigned int delta)
{
	delta *= 4;
	divider += delta;
	if (started) {
		ticks += delta;
		if(ticks >= speed) {
			//ticks -= speed;
			ticks = 0;
			if(++counter >= 0x100) {
				interrupt(INTR_TIMER);
				counter = modulo;
			}
		}
	}
}