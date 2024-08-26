
#include "MMU.h"
#include <cstdio>

namespace gbex
{
	MMU::MMU() : m_Cartridge(nullptr)
	{
		m_Memory = new uint8_t[0xFFFF + 1];
	}

	MMU::~MMU()
	{
		delete[] m_Memory;
	}

	void MMU::write8(uint16_t addr, uint8_t value)
	{

		if (addr <= 0x7FFF)
		{
			m_Cartridge->write8(addr, value);
			return;
		}

		// We want to intercept some writes because they are memory mapped io 
		switch (addr)
		{
		case 0xFF00:	

			break;
		case 0xFF46:
			do_dma_transfer(value);
			break;
		case 0xFF02:

			if (value == 0x81)
			{
				printf("%c", m_Memory[0xFF01]);
			}

			break;
		}

		m_Memory[addr] = value;
	}

	void MMU::write16(uint16_t addr, uint16_t value)
	{
		if (addr <= 0x7FFF)
		{
			m_Cartridge->write16(addr, value);
			return;
		}

		uint16_t* p = ((uint16_t*)(m_Memory + addr));
		*p = value;
	}

	uint8_t MMU::read8(uint16_t addr)
	{
		// If we try to read ROM space route it to a ROM read function 
		if (addr <= 0x7FFF)
			return m_Cartridge->read8(addr);

		// HARDCODED FOR NOW
		if (addr == 0xFF44)
			return 0x90;

		return m_Memory[addr];
	}

	uint16_t MMU::read16(uint16_t addr)
	{
		if (addr < 0x7FFF)
			return m_Cartridge->read16(addr);


		return *((uint16_t*)(m_Memory + addr));
	}

	void MMU::do_dma_transfer(uint8_t data)
	{
		uint16_t addr = data << 8;
		for (uint16_t i = 0; i < 0xA0; i++)
		{
			write8(0xFE00 + i, read8(addr + i));
		}
	}
}