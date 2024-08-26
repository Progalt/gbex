
#ifndef GBEX_MMU_H
#define GBEX_MMU_H

#include <cstdint>
#include "Cartridge.h"
#include "BitValue.h"
#include "Interrupts.h"

namespace gbex
{
	namespace SpriteAttribute
	{
		constexpr uint8_t Priority = 7;
		constexpr uint8_t YFlip = 6;
		constexpr uint8_t XFlip = 5;
		constexpr uint8_t DMGPalette = 4;
		constexpr uint8_t Bank = 3;

		// The last 3 Bits are the CGB Palette 
	}

	struct Sprite
	{
		uint8_t y;
		uint8_t x;
		uint8_t tile_index;
		BitValue attributes;
	};

	class MMU
	{
	public:

		MMU();
		~MMU();

		void write8(uint16_t addr, uint8_t value);

		void write16(uint16_t addr, uint16_t value);

		uint8_t read8(uint16_t addr);

		uint16_t read16(uint16_t addr);
		
		uint8_t* get_memory_pointer(uint16_t addr);

		void do_dma_transfer(uint8_t data);

		void initialise_memory_mapped_io();

	private:

		void initialise_memory_mapped_io_dmg();

		friend class gbex;

		/*
			We will allocate all 16 bit address space so we don't have to shift addresses around but we won't use it all for the emulator

			The first 32kb will come from the cartridge ROM and the MBCs
		*/

		uint8_t* m_Memory;

		Cartridge* m_Cartridge;

		Interrupts* interrupts;


	};
}

#endif // GBEX_MMU_H