#include "timer.h"
#include "interrupt.h"
#include "cpu.h"
#include "mem.h"

static unsigned int prev_time;
static unsigned int elapsed;
static unsigned int ticks;
static unsigned int divticks;

static unsigned char tac;
static unsigned int started;
static unsigned int speed = 1024;
static unsigned int counter;
static unsigned char divider;
static unsigned char modulo;

void timer_set_div(unsigned char v)
{
	(void) v;
	divticks = divider = 0;
	//divider = 0;
}

unsigned char timer_get_div(void)
{
	return divider;
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

static void timer_tick(void)
{
	/* 1/262144Hz has elapsed */
	++ticks;

	/* Divider updates at 16384Hz */
	if(!(ticks & 15))
	{
		++divider;
		//ticks = 0;
	}

	if(!started)
		return;

	if(!(ticks & (speed-1)))
	{
		if(++counter >= 0x100)
		{
			interrupt(INTR_TIMER);
			counter = modulo;
		}
		//ticks = 0;
	}
}

void timer_cycle(unsigned int delta)
{
	//unsigned char* mem = mem_get_bytes();
	delta = delta << 2;
	
	divticks += delta;
	if(divticks >= 256) {
		//divticks -= 256;
		divticks = 0;
		++divider;
	}
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

	// elapsed += delta * 4; /* 4 cycles to a timer tick */
	// while(elapsed >= 16)
	// {
		// timer_tick();
		// elapsed -= 16;	/* keep track of the time overflow */
	// }
}