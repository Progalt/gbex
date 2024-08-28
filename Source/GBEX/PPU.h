
#ifndef GBEX_PPU_H
#define GBEX_PPU_H

#include <functional>
#include "BitValue.h"
#include <memory>
#include "Defs.h"

namespace gbex
{
	class CPU;

	enum class PPUMode
	{
		HBlank,			/* Mode 0 */
		VBlank,			/* Mode 1 */
		OAMScan,		/* Mode 2 */
		Drawing,		/* Mode 3 */
	};

	constexpr uint16_t DotsPerHBlank = 204;
	constexpr uint16_t DotsPerVBlankScanline = 456;
	constexpr uint16_t DotsPerOAMScan = 80;
	constexpr uint16_t DotsPerDrawing = 172;
	constexpr uint16_t DotsPerScanline = DotsPerHBlank + DotsPerOAMScan + DotsPerDrawing;

	class MMU;

	struct Colour
	{
		
		Colour() { }
		Colour(uint8_t r, uint8_t g, uint8_t b)
		{
			col[0] = r;
			col[1] = g;
			col[2] = b;
		}

		uint8_t col[3];
	};

	struct Palette
	{
		Colour colours[4];
	};

	struct OAM
	{
		uint8_t y;
		uint8_t x;

		uint8_t tileIndex;
		uint8_t attributes;
	};

	class PPU
	{
	public:
		
		void initialise(MMU* mmu);

		void step();


		CPU* m_CPU;
		MMU* m_MMU;

		std::function<void()> m_VsyncCallback;

		uint32_t m_Dots = 0;
		PPUMode m_Mode = PPUMode::HBlank;

		BitAddress m_LCDC;
		BitAddress m_STAT;
		uint8_t* m_LY;
		uint8_t* m_LYC;

		Palette m_Palette;

		std::unique_ptr<uint8_t[]> m_Framebuffer;

	private:

		void draw_scanline();

		void compare_ly_lyc();

		void plot_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);

		void draw_background();

		void draw_sprites();
	};
}

#endif // GBEX_PPU_H