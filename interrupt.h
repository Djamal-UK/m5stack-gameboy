#ifndef INTERRUPT_H
#define INTERRUPT_H

extern unsigned char IME;
extern unsigned char IF;
extern unsigned char IE;

void interrupt(unsigned int);
void interrupt_disable(void);
void interrupt_enable(void);
unsigned char interrupt_get_IF(void);
void interrupt_set_IF(unsigned char);
unsigned char interrupt_get_IE(void);
void interrupt_set_mask(unsigned char);
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
