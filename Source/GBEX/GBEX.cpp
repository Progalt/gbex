
#include "GBEX.h"
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cstdio>

namespace gbex
{
	bool gbex::load_cartridge(uint8_t* rom, const size_t size)
	{

		unload_cartridge();

		CartridgeHeader header{};

		memcpy(&header.logo, &rom[0x0104], 48);
		memcpy(&header.title, &rom[0x0134], 16);
		memcpy(&header.manufacturer_code, &rom[0x013F], 4);
		header.cgb_flag = rom[0x0143];
		memcpy(&header.new_licensee, &rom[0x0144], 2);
		header.sgb_flag = rom[0x0146];
		header.cartridge_type = rom[0x0147];
		header.rom_size = rom[0x0148];
		header.ram_size = rom[0x0149];
		header.destination_code = rom[0x014A];
		header.old_licensee = rom[0x014B];
		header.mask_rom_version = rom[0x014C];
		header.checksum = rom[0x014D];

		switch (header.cartridge_type)
		{
		case 0x00:
			m_MMU.m_Cartridge = new NoMBC(header, rom);
			break;
		case 0x01:
			m_MMU.m_Cartridge = new MBC1(header, rom, false, false);
			break;
		case 0x02:
			m_MMU.m_Cartridge = new MBC1(header, rom, true, false);
			break;
		case 0x03:
			m_MMU.m_Cartridge = new MBC1(header, rom, true, true);
			break;
		default:
			throw std::runtime_error("Unsupported Mapper Chip");
			break;
		}

		m_MMU.m_Timer = &m_Timer;
		m_MMU.initialise_memory_mapped_io();
		m_Timer.initialise(&m_CPU, &m_MMU);

		m_CPU.initialise_registers(header);
		m_CPU.mmu = &m_MMU;
		m_CPU.interrupts = Interrupts(&m_CPU, &m_MMU);
		m_MMU.interrupts = &m_CPU.interrupts;
		m_PPU.m_VsyncCallback = m_VsyncCallback;
		m_PPU.m_CPU = &m_CPU;
		m_PPU.initialise(&m_MMU);
		m_Joypad.initialise(&m_CPU, &m_MMU);
		m_MMU.m_Joypad = &m_Joypad;
		

		return true;
	}

	void gbex::unload_cartridge()
	{
		if (m_MMU.m_Cartridge)
		{
			delete m_MMU.m_Cartridge;
			m_MMU.m_Cartridge = nullptr;
		}


	}

	void gbex::set_breakpoint(uint16_t target_pc)
	{
		m_Breakpoint = target_pc;
	}

	void gbex::step()
	{

		if (m_CPU.pc == m_Breakpoint && !m_HitBreakpoint)
		{
			m_HitBreakpoint = true;
			m_Halted = true;
		}

		if (!m_Halted)
		{
			
			m_CPU.interrupts.handle_interrupts();
			m_CPU.step();

			m_PPU.step();
			m_Timer.step(m_CPU.tcycles);
			

			if (m_Step)
				m_Halted = true;
		}
		else
		{
			if (m_PPU.m_VsyncCallback)
				m_PPU.m_VsyncCallback();
		}



	}
}