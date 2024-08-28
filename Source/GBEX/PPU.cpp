
#include "PPU.h"
#include "CPU.h"
#include <chrono>

namespace gbex
{
	void PPU::initialise(MMU* mmu)
	{
		m_MMU = mmu;

		m_LCDC = BitAddress(mmu->get_memory_pointer(0xFF40));
		m_STAT = BitAddress(mmu->get_memory_pointer(0xFF41));
		m_LY = mmu->get_memory_pointer(0xFF44);
		m_LYC = mmu->get_memory_pointer(0xFF45);
		m_Mode = PPUMode::OAMScan;

		m_Framebuffer = std::make_unique<uint8_t[]>(GameboyScreenWidth * GameboyScreenHeight * 4);

		m_Palette.colours[0] = Colour(255, 255, 255);
		m_Palette.colours[1] = Colour(190, 190, 190);
		m_Palette.colours[2] = Colour(90, 90, 90);
		m_Palette.colours[3] = Colour(0, 0, 0);
	}

	void PPU::step()
	{


		// 1 dot is equivalent to 4 tcycles or 1 mcycle
		// If we want to emulate CGB, should probably divide by 2 
		m_Dots += m_CPU->tcycles;

		// Just return if PPU is off
		if (!m_LCDC.is_bit_set(7))
		{
			if (m_Dots >= 70224)
			{
				m_Dots = 0;

				if (m_VsyncCallback)
					m_VsyncCallback();
			}

			return;
		}


		switch (m_Mode)
		{
		case PPUMode::HBlank:
			if (m_Dots >= DotsPerHBlank)
			{
				m_Dots = m_Dots % DotsPerHBlank;
				m_Mode = PPUMode::OAMScan;

				*m_LY += 1;

				compare_ly_lyc();
				
				if (*m_LY == 144)
				{
					m_Mode = PPUMode::VBlank;

					m_CPU->interrupts.set_interrupt_flag(InterruptSource::VBlank);

					if (m_STAT.is_bit_set(4))
					{
						m_CPU->interrupts.set_interrupt_flag(InterruptSource::LCD);
					}


					m_STAT.set_bit(0, true);
					m_STAT.set_bit(1, false);

					// If we have a callback call it 

					if (m_VsyncCallback)
						m_VsyncCallback();
				}
				else
				{
					if (m_STAT.is_bit_set(5))
					{
						m_CPU->interrupts.set_interrupt_flag(InterruptSource::LCD);
					}

					m_STAT.set_bit(0, false);
					m_STAT.set_bit(1, true);
				}
			}
			break;
		case PPUMode::VBlank:
			if (m_Dots >= DotsPerVBlankScanline)
			{
				m_Dots = m_Dots % DotsPerVBlankScanline;

				*m_LY += 1;

				compare_ly_lyc();
				

				if (*m_LY == 153)
				{


					*m_LY = 0;
					m_Mode = PPUMode::OAMScan;

					if (m_STAT.is_bit_set(5))
					{
						m_CPU->interrupts.set_interrupt_flag(InterruptSource::LCD);
					}

					m_STAT.set_bit(0, false);
					m_STAT.set_bit(1, true);
				}


			}
			break;
		case PPUMode::OAMScan:
			if (m_Dots >= DotsPerOAMScan)
			{
				m_Dots = m_Dots % DotsPerOAMScan;

				m_Mode = PPUMode::Drawing;

				m_STAT.set_bit(0, true);
				m_STAT.set_bit(1, true);
			}
			break;
		case PPUMode::Drawing:
			if (m_Dots >= DotsPerDrawing)
			{
				m_Dots = m_Dots % DotsPerDrawing;

				m_Mode = PPUMode::HBlank;
				draw_scanline();

				if (m_STAT.is_bit_set(3))
				{
					m_CPU->interrupts.set_interrupt_flag(InterruptSource::LCD);
				}

				m_STAT.set_bit(0, false);
				m_STAT.set_bit(1, false);
			}
			break;
		}

	}

	void PPU::compare_ly_lyc()
	{
		uint8_t equal = *m_LYC == *m_LY;

		m_STAT.set_bit(2, equal);

		if (equal && m_STAT.is_bit_set(6))
			m_CPU->interrupts.set_interrupt_flag(InterruptSource::LCD);
	}

	void PPU::draw_scanline()
	{
		draw_background();

		if (m_LCDC.is_bit_set(1))
			draw_sprites();
	}

	void PPU::draw_background()
	{

		uint32_t screenY = *m_LY;
		for (uint32_t screenX = 0; screenX < GameboyScreenWidth; screenX++)
		{
			uint8_t scrollX = screenX + m_MMU->read8(0xFF43);
			uint8_t scrollY = screenY + m_MMU->read8(0xFF42);

			uint32_t bgMapX = scrollX % (32 * 8);
			uint32_t bgMapY = scrollY % (32 * 8);

			uint32_t tileX = bgMapX / 8;
			uint32_t tileY = bgMapY / 8;

			uint32_t tilePixelX = bgMapX % 8;
			uint32_t tilePixelY = bgMapY % 8;

			uint32_t tileIndex = tileY * 32 + tileX;

			uint32_t tileIdAddress = (m_LCDC.is_bit_set(3) ? 0x9C00 : 0x9800) + tileIndex;

			uint32_t tileId = m_MMU->read8(tileIdAddress);

			if (!m_LCDC.is_bit_set(4) && tileId < 128)
				tileId += 256;

			uint32_t tileMemAddr = 0x8000 + (tileId * 16);
			tileMemAddr += (tilePixelY * 2);

			uint8_t b1 = m_MMU->read8(tileMemAddr);
			uint8_t b2 = m_MMU->read8(tileMemAddr + 1);

			b1 = b1 >> (7 - tilePixelX) & 1;
			b2 = b2 >> (7 - tilePixelX) & 1;

			uint8_t paletteIndex = b1 | (b2 << 1);

			uint8_t bgp = m_MMU->read8(0xFF47);

			uint8_t pid = (bgp >> (paletteIndex * 2)) & 3;

			plot_pixel(screenX, screenY, m_Palette.colours[pid].col[0], m_Palette.colours[pid].col[1], m_Palette.colours[pid].col[2]);


		}
	
	}

	void PPU::draw_sprites()
	{
		uint8_t objSizeBit = m_LCDC.is_bit_set(2);
		uint8_t objSize = objSizeBit ? 16 : 8;

		for (int i = 39; i >= 0; i--)
		{
			OAM* obj = (OAM*)m_MMU->get_memory_pointer(0xFE00 + (sizeof(OAM) * i));

			BitValue attributes(obj->attributes);

			// Its not on screen 
			if (obj->y < 16)
				continue;

			// Sprite does not overlap scanline
			if (obj->y - 16 > *m_LY || ((obj->y - 16) + objSize) <= *m_LY)
				continue;

			// Sprite isn't on screen
			// The first X position on screen is 9
			if (obj->x < 8 || obj->x >= 168)
				continue;

			int yp = *m_LY - (obj->y - 16);

			for (uint8_t x = 0; x < 8; x++)
			{
				int xp = obj->x + x;

				// Check if this pixel is offscreen
				if (xp < 0 || xp >= 160)
					continue;



				uint16_t tileIdx = obj->tileIndex & (m_LCDC.is_bit_set(2) ? 0xFE : 0xFF);

				// int pixelOffset = (*m_LY * 160 * 4) + ((xp - 8) * 4);

				if (m_LCDC.is_bit_set(2) && yp >= 8)
					tileIdx += 1;

				// TODO: Y Flip

				uint32_t tileMemAddr = 0x8000 + (tileIdx * 16);
				tileMemAddr += (yp * 2);

				uint8_t b1 = m_MMU->read8(tileMemAddr);
				uint8_t b2 = m_MMU->read8(tileMemAddr + 1);

				uint8_t xShift = 7 - x;

				// X flip 
				if (attributes.is_bit_set(5))
					xShift = x;

				b1 = b1 >> xShift & 1;
				b2 = b2 >> xShift & 1;


				uint8_t paletteIndex = b1 | (b2 << 1);

				uint8_t dmgPalette = attributes.is_bit_set(4);

				uint8_t obp = m_MMU->read8(0xFF48 + dmgPalette);

				// Skip transparent 
				if (paletteIndex == 0)
					continue;

				uint8_t pid = (obp >> (paletteIndex * 2)) & 3;

				plot_pixel(xp - 8, *m_LY, m_Palette.colours[pid].col[0], m_Palette.colours[pid].col[1], m_Palette.colours[pid].col[2]);

			}
		}
	}

	void PPU::plot_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
	{
		uint32_t pixelOffset = y * (GameboyScreenWidth * 4) + (x * 4);
		m_Framebuffer[pixelOffset + 0] = r;
		m_Framebuffer[pixelOffset + 1] = g;
		m_Framebuffer[pixelOffset + 2] = b;
		m_Framebuffer[pixelOffset + 3] = 255;
	}
}