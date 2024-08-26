
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

	uint8_t* MMU::get_memory_pointer(uint16_t addr)
	{

		return &m_Memory[addr];
	}

	void MMU::initialise_memory_mapped_io()
	{
		initialise_memory_mapped_io_dmg();
	}

	void MMU::initialise_memory_mapped_io_dmg()
	{
		// CPU IO
		write8(0xFF00, 0xCF);		// JOYP
		write8(0xFF01, 0x00);		// SB
		write8(0xFF02, 0x7E);		// SC
		write8(0xFF04, 0xAB);		// DIV
		write8(0xFF05, 0x00);		// TIMA
		write8(0xFF06, 0x00);		// TMA
		write8(0xFF07, 0xF8);		// TAC
		write8(0xFF0F, 0xE1);		// EI

		// SOUND IO
		write8(0xFF10, 0x80);		// NR10
		write8(0xFF11, 0xBF);		// NR11
		write8(0xFF12, 0xF3);		// NR12
		write8(0xFF13, 0xFF);		// NR13
		write8(0xFF14, 0xBF);		// NR14

		write8(0xFF16, 0x3F);		// NR21
		write8(0xFF17, 0x00);		// NR22
		write8(0xFF18, 0xFF);		// NR23
		write8(0xFF19, 0xBF);		// NR24

		write8(0xFF1A, 0x7F);		// NR30
		write8(0xFF1B, 0xFF);		// NR31
		write8(0xFF1C, 0x9F);		// NR32
		write8(0xFF1D, 0xFF);		// NR33
		write8(0xFF1E, 0xBF);		// NR34

		write8(0xFF20, 0xFF);		// NR41
		write8(0xFF21, 0x00);		// NR42
		write8(0xFF22, 0x00);		// NR43
		write8(0xFF23, 0xBF);		// NR44

		write8(0xFF24, 0x77);		// NR50
		write8(0xFF25, 0xF3);		// NR51
		write8(0xFF26, 0xF1);		// NR52

		// PPU IO
		write8(0xFF50, 0x91);		// LCDC
		write8(0xFF41, 0x85);		// STAT
		write8(0xFF42, 0x00);		// SCY
		write8(0xFF43, 0x00);		// SCX
		write8(0xFF44, 0x00);		// LY
		write8(0xFF45, 0x00);		// LYC
		write8(0xFF46, 0xFF);		// DMA 
		write8(0xFF47, 0xFC);		// BGP
		write8(0xFF48, 0xFF);		// OBP0
		write8(0xFF49, 0xFF);		// OBP1
		write8(0xFF4A, 0x00);		// WY
		write8(0xFF4B, 0x00);		// WX

		write8(0xFFFF, 0x00);		// IE
	}
}