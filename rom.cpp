#include <stdio.h>
#include <string.h>
#include "rom.h"

const unsigned char *bytes;

static s_rominfo rominfo;

static uint16_t banks[256] = {
	2, 4, 8, 16, 32, 64, 128, 256, 512,
	/* 0x52 */
	72, 80, 96,
	0
};

static uint8_t rams[256] = {
	0, 1, 1, 4, 16, 8,
	0
};

// static char *regions[] = {
	// "Japan",
	// "Non-Japan",
	// "Unknown"
// };

static unsigned char header[] = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

int rom_init(const unsigned char *rombytes)
{
	int i, pass;
	unsigned char checksum = 0;

	/* Check Nintendo logo on ROM header */
	if(memcmp(&rombytes[0x104], header, sizeof(header)) != 0)
		return 0;

	uint8_t cart_type  = rombytes[0x147];
	rominfo.rom_banks  = rombytes[0x148]>=0x52 ? banks[rombytes[0x148] - 0x52] : banks[rombytes[0x148]];
	rominfo.ram_banks  = rams[rombytes[0x149]];
	//rominfo.region    = rombytes[0x14A];
	//rominfo.version   = rombytes[0x14C];

	for(i = 0x134; i <= 0x14C; i++)
		checksum = checksum - rombytes[i] - 1;

	pass = rombytes[0x14D] == checksum;
	if(!pass)
		return 0;

	bytes = rombytes;

	switch(cart_type)
	{
		case 0x09:
			rominfo.has_battery = true;
		case 0x08:
		case 0x00:
			rominfo.rom_mapper = NROM;
			break;
		case 0x03:
			rominfo.has_battery = true;
		case 0x02:
		case 0x01:
			rominfo.rom_mapper = MBC1;
			break;
		case 0x06:
			rominfo.has_battery = true;
		case 0x05:
			rominfo.rom_mapper = MBC2;
			break;
		case 0x0D:
			rominfo.has_battery = true;
		case 0x0C:
		case 0x0B:
			rominfo.rom_mapper = MMM01;
			break;
		case 0x10:
		case 0x0F:
			rominfo.has_rtc = true;
		case 0x13:
			rominfo.has_battery = true;
		case 0x12:
		case 0x11:
			rominfo.rom_mapper = MBC3;
			break;
		case 0x1E:
		case 0x1B:
			rominfo.has_battery = true;
		case 0x1D:
		case 0x1C:
		case 0x1A:
		case 0x19:
			rominfo.rom_mapper = MBC5;
			break;
	}

	return 1;
}

const s_rominfo *rom_get_info(void)
{
	return &rominfo;
}

unsigned int rom_get_ram_size()
{
	if (rominfo.rom_mapper == MBC2)
		return 512;
	return rominfo.ram_banks * 1024 * 8;
}

int rom_load(const char *filename)
{
	return 0;
}

const unsigned char *rom_getbytes(void)
{
	return bytes;
}
