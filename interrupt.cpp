#include "interrupt.h"
#include "cpu.h"

unsigned char IME;
unsigned char IF;
unsigned char IE;

static int pending;

/* Returns true if the cpu should be unhalted */
int interrupt_flush(void)
{
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

	if (!IME)
		return !!(IF & IE);

	/* Interrupts are enabled - Check if any need to fire */
	for (int i = 0; i < 5; ++i) {
		if (IE & IF & (1 << i)) {
			IME = 0;
			IF &= ~(1 << i);
			cpu_interrupt(0x40 + i*0x08);
			break;
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

int interrupt_get_IME(void)
{
	return (IME && !pending);
}
