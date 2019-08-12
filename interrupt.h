#ifndef INTERRUPT_H
#define INTERRUPT_H

extern unsigned char IME;
extern unsigned char IF;
extern unsigned char IE;

void interrupt(unsigned int);
void interrupt_disable(void);
void interrupt_enable(void);
int interrupt_get_IME(void);
int interrupt_flush(void);

enum {
	INTR_VBLANK  = 0x01,
	INTR_LCDSTAT = 0x02,
	INTR_TIMER   = 0x04,
	INTR_SERIAL  = 0x08,
	INTR_JOYPAD  = 0x10
};

#endif
