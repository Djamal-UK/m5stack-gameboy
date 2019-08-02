#ifndef MEM_H
#define MEM_H

#include "rom.h"
void memm_init(void);
unsigned char mem_get_byte(unsigned short);
inline unsigned short mem_get_word(unsigned short i) { return mem_get_byte(i) | (mem_get_byte(i+1)<<8); }
void mem_write_byte(unsigned short, unsigned char);
inline void mem_write_word(unsigned short d, unsigned short i) { mem_write_byte(d, i&0xFF); mem_write_byte(d+1, i>>8); }
void mem_bank_switch(unsigned int);
void mem_bank_switch_ram(unsigned int);
void mem_enable_ram(bool);
unsigned char mem_get_raw(unsigned short);
unsigned char* mem_get_bytes(void);
unsigned char* mem_get_ram(void);

#endif
