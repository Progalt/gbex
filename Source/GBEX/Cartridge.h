
#ifndef GBEX_CARTRIDGE_H
#define GBEX_CARTRIDGE_H

#include <cstdint>
#include <memory>
#include <stdexcept>

namespace gbex
{
	// Read from address 0x0104
	// https://gbdev.io/pandocs/The_Cartridge_Header.html
	struct CartridgeHeader
	{
		uint8_t logo[48];

		// NOTE: these next 3 fields can be combined for a title 
		uint8_t title[16];

		uint8_t manufacturer_code[4];

		uint8_t cgb_flag;

		uint8_t new_licensee[2];

		uint8_t sgb_flag;

		uint8_t cartridge_type;

		uint8_t rom_size;

		uint8_t ram_size;

		uint8_t destination_code;

		uint8_t old_licensee;

		uint8_t mask_rom_version;

		uint8_t checksum;

		uint8_t global_checksum[2];
	};

	/*
		A cartridge is the game. 

		Its loaded to address 0x000 0x7FFF

		With 0x0000 -> 0x3FFF Being Bank 0
		And 0x4000 -> 0x7FFF Being Bank N controlled by the MBC 

		Reads to these addresses are routed through the virtual cartridge functions.
	*/

	class Cartridge
	{
	public:


		virtual uint8_t read8(uint16_t addr) = 0;

		virtual uint16_t read16(uint16_t addr) = 0;

		virtual void write8(uint16_t addr, uint8_t data) = 0;

		virtual void write16(uint16_t addr, uint16_t data) = 0;

		const uint8_t get_mbc_type() const { return m_Header.cartridge_type; }

		const CartridgeHeader& get_header() const { return m_Header; }

	protected:

		CartridgeHeader m_Header;

		uint8_t* m_ROM;

		uint16_t m_ROMBank = 0x01;
		uint16_t m_RAMBank = 0x0;

		uint8_t m_BankingMode = 0x0;

	};

	/*
		This Cartridge has no MBC, so its max 32kb -> Tetris and Dr. Mario for instance 
	*/
	class NoMBC : public Cartridge
	{
	public:

		NoMBC(CartridgeHeader header, uint8_t* rom)
		{
			m_Header = header;
			m_ROM = rom;
		}

		uint8_t read8(uint16_t addr);

		uint16_t read16(uint16_t addr);

		void write8(uint16_t addr, uint8_t data);

		void write16(uint16_t addr, uint16_t data);

	private:
	};

	class MBC1 : public Cartridge
	{
	public:

		MBC1(CartridgeHeader header, uint8_t* rom, bool ram, bool battery)
		{
			m_Header = header;
			m_ROM = rom;

			m_HasRAM = ram;
			m_HasBattery = battery;

			uint32_t m_RAMSize = 0;

			// MBC1 chips support up to 32kb of usable RAM anything any bigger required a different MBC
			switch (header.ram_size)
			{
			case 0:
			case 1:
				break;
			case 2:
				m_RAMSize = 0x2000;
				break;
			case 3:
				m_RAMSize = 0x8000;
				break;
			default:
				throw std::runtime_error("Invalid MBC1 Ram configuration");
				break;
			}
			
			m_RAM = std::make_unique<uint8_t[]>(m_RAMSize);

			for (uint32_t i = 0; i < m_RAMSize; i++)
				m_RAM[i] = 0xFF;
		}

		uint8_t read8(uint16_t addr);

		uint16_t read16(uint16_t addr);

		void write8(uint16_t addr, uint8_t data);

		void write16(uint16_t addr, uint16_t data);

	private:

		bool m_HasRAM;
		bool m_HasBattery;

		bool m_RAMEnabled = false;

		std::unique_ptr<uint8_t[]> m_RAM;
	};
}

#endif // GBEX_CARTRIDGE_H