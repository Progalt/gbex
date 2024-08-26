
#ifndef GBEX_CARTRIDGE_H
#define GBEX_CARTRIDGE_H

#include <cstdint>

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
		Writes are ignored, not that a game should write to them if its an official game.
	*/

	class Cartridge
	{
	public:


		virtual uint8_t read8(uint16_t addr) = 0;

		virtual uint16_t read16(uint16_t addr) = 0;

		const uint8_t get_mbc_type() const { return m_Header.cartridge_type; }

		const CartridgeHeader& get_header() const { return m_Header; }

	protected:

		CartridgeHeader m_Header;

		uint8_t* m_ROM;

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

	private:
	};
}

#endif // GBEX_CARTRIDGE_H