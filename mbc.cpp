#include "mbc.h"
#include "mem.h"
#include "rom.h"

enum {
	NO_FILTER_WRITE,
	FILTER_WRITE
};

static unsigned int curr_rom_bank = 0;
static unsigned int curr_ram_bank = 0;
static unsigned int bank_upper_bits = 0;
static unsigned int ram_select = 0;

/* Unfinished, no clock etc */
unsigned int MBC3_write_byte(unsigned short d, unsigned char i)
{
	if(d < 0x2000)
	{
		mem_enable_ram((i & 0x0F) == 0x0A);
		return FILTER_WRITE;
	}

	if(d < 0x4000)
	{
		curr_rom_bank = i & 0x7F;

		if(curr_rom_bank == 0)
			curr_rom_bank++;

		mem_bank_switch_rom(curr_rom_bank);

		return FILTER_WRITE;
	}
	
	if(d < 0x6000)
	{
		// TODO: RTC
		curr_ram_bank = i & 0x07;
		mem_bank_switch_ram(curr_ram_bank);
	}

	if(d < 0x8000)
		return FILTER_WRITE;

	return NO_FILTER_WRITE;
}
unsigned int MBC1_write_byte(unsigned short d, unsigned char i)
{
	if(d < 0x2000)
	{
		mem_enable_ram((i & 0x0F) == 0x0A);
		return FILTER_WRITE;
	}

	/* Switch rom bank at 4000-7fff */
	if(d < 0x4000)
	{
		/* Bits 0-4 come from the value written to memory here,
		 * bits 5-6 come from a seperate write to 4000-5fff if
		 * RAM select is 1.
		 */
		curr_rom_bank = i & 0x1F;
		if(!ram_select)
			curr_rom_bank |= bank_upper_bits;

		/* "Writing to this address space selects the lower 5 bits of the
		 * ROM Bank Number (in range 01-1Fh). When 00h is written, the MBC
		 * translates that to bank 01h also."
		 * http://nocash.emubase.de/pandocs.htm#mbc1max2mbyteromandor32kbyteram
		 */

		if(curr_rom_bank == 0 || curr_rom_bank == 0x20 || curr_rom_bank == 0x40 || curr_rom_bank == 0x60)
			curr_rom_bank++;

		mem_bank_switch_rom(curr_rom_bank);

		return FILTER_WRITE;
	}

	/* Bit 5 and 6 of the bank selection */
	if(d < 0x6000)
	{
		if (ram_select) {
			curr_ram_bank = i&3;
			mem_bank_switch_ram(curr_ram_bank);
		}
		else {
			bank_upper_bits = (i & 0x3)<<5;
			curr_rom_bank = bank_upper_bits | (curr_rom_bank & 0x1F);
			mem_bank_switch_rom(curr_rom_bank);
		}
		return FILTER_WRITE;
	}

	if(d < 0x8000)
	{
		ram_select = i&1;
		if (ram_select) {
			curr_rom_bank &= 0x1F;
			mem_bank_switch_rom(curr_rom_bank);
		}
		else {
			curr_ram_bank = 0;
			mem_bank_switch_ram(curr_ram_bank);
		}
		return FILTER_WRITE;
	}
	
	return NO_FILTER_WRITE;
}
