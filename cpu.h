#ifndef CPU_H
#define CPU_H
#include "rom.h"

extern int halted;

void cpu_init(bool bootrom = true);
unsigned int cpu_cycle(void);
unsigned int cpu_get_cycles(void);
void cpu_interrupt(unsigned short);

#endif
