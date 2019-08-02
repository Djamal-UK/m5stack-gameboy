#ifndef ROM_H
#define ROM_H

#include <stdint.h>

struct s_rominfo {
	uint16_t rom_banks;
	uint8_t ram_banks;
	uint8_t rom_mapper;
	uint8_t has_battery;
	uint8_t has_rtc;
};

int rom_load(const char *);
int rom_init(const unsigned char *);
const unsigned char *rom_getbytes(void);
const s_rominfo *rom_get_info(void);
unsigned int rom_get_ram_size();

enum {
	NROM,
	MBC1,
	MBC2,
	MMM01,
	MBC3,
	MBC4,
	MBC5,
};

#endif
