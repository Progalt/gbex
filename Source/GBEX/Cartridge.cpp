
#include "Cartridge.h"
#include <cstdio>

namespace gbex
{
	/*
		With NoMBC we can just read from it
	*/

	uint8_t NoMBC::read8(uint16_t addr)
	{
		// Just return a garbage value if its out of range
		// TODO: Make this check better -> Same for read16
		if (addr > 0x7FFF)
			return 0xFF;


		return *(m_ROM + addr);
	}

	uint16_t NoMBC::read16(uint16_t addr)
	{
		if (addr > 0x7FFF)
			return 0xFFFF;

		return *((uint16_t*)(m_ROM + addr));
	}
}