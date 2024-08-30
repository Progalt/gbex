
#include "Cartridge.h"
#include <cstdio>
#include <stdexcept>

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

	void NoMBC::write8(uint16_t addr, uint8_t data)
	{
		// throw std::runtime_error("A Cartridge cannot be written to if it doesn't have an MBC");
	}

	void NoMBC::write16(uint16_t addr, uint16_t data)
	{
		// throw std::runtime_error("A Cartridge cannot be written to if it doesn't have an MBC");
	}

	uint8_t MBC1::read8(uint16_t addr)
	{
		if (addr >= 0x0000 && addr <= 0x3FFF)
			return *(m_ROM + addr);

		if (addr >= 0x4000 && addr <= 0x7FFF)
		{
			uint16_t bankAddr = addr - 0x4000;
			uint32_t bankOffset = 0x4000 * m_ROMBank;

			return *(m_ROM + (bankOffset + bankAddr));
		}

		if (addr >= 0xA000 && addr <= 0xBFFF)
		{
			if (!m_RAMEnabled)
				throw std::runtime_error("Attempting to read from cartridge RAM when not enabled");

			uint16_t bankAddr = addr - 0xA000;
			uint32_t bankOffset = 0x2000 * m_RAMBank;

			return m_RAM[bankOffset + bankAddr];
		}

		throw std::runtime_error("Attempting to read from unmapped cartridge Memory");
	}

	uint16_t MBC1::read16(uint16_t addr)
	{
		// Check its within range
		if (addr >= 0x0000 && addr < 0x7FFF)
		{
			uint8_t upper = read8(addr);
			uint8_t lower = read8(addr + 1);

			return (lower << 8) | upper;
		}

		throw std::runtime_error("Unimplemend MBC1 Logic or its not allowed: read16");
	}

	void MBC1::write8(uint16_t addr, uint8_t data)
	{
		if (addr >= 0x2000 && addr <= 0x3FFF)
		{
			switch (data)
			{
			case 0x0:
				m_ROMBank = 0x1;
				break;
			case 0x20:
				m_ROMBank = 0x21;
				break;
			case 0x40:
				m_ROMBank = 0x41;
				break;
			case 0x60:
				m_ROMBank = 0x61;
				break;
			default:
				m_ROMBank = data & 0x1F;
				break;
			}

			//printf("ROM Bank switch: %02x\n", m_ROMBank);
			return;
		}

		// Enable RAM
		if (addr >= 0x000 && addr <= 0x1FFF)
		{
			if ((data & 0x0F) == 0x0A)
			{
				m_RAMEnabled = true;
				// printf("RAM Enabled\n");
			}
			else
				m_RAMEnabled = false;

			return;
		}

		if (addr >= 0x4000 && addr <= 0x5FFF)
		{
			m_RAMBank = data & 3;
			return;
		}

		if (addr >= 0xA000 && addr <= 0xBFFF)
		{
			if (!m_RAMEnabled)
			{
				return;
			}

			uint16_t bankAddr = addr - 0xA000;
			uint32_t bankOffset = 0x2000 * m_RAMBank;

			m_RAM[bankOffset + bankAddr] = data;
			return;
		}

		if (addr >= 0x6000 && addr <= 0x7FFF)
		{
			m_BankingMode = data & 1;
		}
	}

	void MBC1::write16(uint16_t addr, uint16_t data)
	{
		uint8_t upper = data & 0xFF;
		uint8_t lower = (data >> 8) & 0xFF;

		write8(addr, upper);
		write8(addr + 1, lower);
	}
}