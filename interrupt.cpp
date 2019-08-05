#include "interrupt.h"
#include "cpu.h"

unsigned char IME;
static int pending;

/* Pending interrupt flags */
static unsigned int vblank;
static unsigned int lcdstat;
static unsigned int timer;
static unsigned int serial;
static unsigned int joypad;
unsigned char IF;

/* Interrupt masks */
static unsigned int vblank_masked;
static unsigned int lcdstat_masked;
static unsigned int timer_masked;
static unsigned int serial_masked;
static unsigned int joypad_masked;
unsigned char IE;

/* Returns true if the cpu should be unhalted */
int interrupt_flush(void)
{
	/* Flush the highest priority interrupt and/or resume the cpu */
	if(pending == 2)
	{
		pending--;
		return 0;
	}
	if (pending == 1)
	{
		IME = 1;
		pending = 0;
	}

	/* There's a pending interrupt but interrupts are disabled, just resume the cpu */
	if (!IME)
		return !!(IF & IE);

	/* Interrupts are enabled - Check if any need to fire */
	for (int i = 0; i < 5; ++i) {
		if (IE & IF & (1 << i)) {
			IME = 0;
			IF &= ~(1 << i);
			cpu_interrupt(0x40 + i*0x08);
		}
	}

	return 0;
}

void interrupt_enable(void)
{
	//IME = 1;
	pending = 2;
}

void interrupt_disable(void)
{
	IME = 0;
}

void interrupt(unsigned int n)
{
	/* Add this interrupt to pending queue */
	IF |= n;

	/* If interrupts are already enabled, flush one now, otherwise wait for
	 * interrupts to be re-enabled.
	 */
	if(interrupt_flush())
		halted = 0;
}

unsigned char interrupt_get_IF(void)
{
	return IF;
}

void interrupt_set_IF(unsigned char mask)
{
	IF = mask;

	// if(IME && mask)
		// pending = 1;
}

unsigned char interrupt_get_IE(void)
{
	return IE;
}

void interrupt_set_mask(unsigned char mask)
{
	IE = mask;
}

int interrupt_get_IME(void)
{
	return (IME && !pending);
}
