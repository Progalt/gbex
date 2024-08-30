
#include "PPU.h"
#include "CPU.h"
#include <chrono>
#include <cmath>

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
		m_LastFrame = std::make_unique<uint8_t[]>(GameboyScreenWidth * GameboyScreenHeight * 4);

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

		// Just return if PPU is off but still allow vsync 
		if (!m_LCDC.is_bit_set(7))
		{
			// IDK if this is good but reset the first 2 bits of stat to 0 
			*m_STAT.get_ptr() &= ~3;

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



					// If we have a callback call it 

					if (m_VsyncCallback)
						m_VsyncCallback();

					memcpy(m_LastFrame.get(), m_Framebuffer.get(), GameboyScreenWidth * GameboyScreenHeight * 4);
				}
				else
				{
					if (m_STAT.is_bit_set(5))
					{
						m_CPU->interrupts.set_interrupt_flag(InterruptSource::LCD);
					}

				}

				*m_STAT.get_ptr() &= ~3;
				*m_STAT.get_ptr() |= (uint8_t)m_Mode;
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

				
				}

				*m_STAT.get_ptr() &= ~3;
				*m_STAT.get_ptr() |= (uint8_t)m_Mode;


			}
			break;
		case PPUMode::OAMScan:
			if (m_Dots >= DotsPerOAMScan)
			{
				m_Dots = m_Dots % DotsPerOAMScan;

				m_Mode = PPUMode::Drawing;

				*m_STAT.get_ptr() &= ~3;
				*m_STAT.get_ptr() |= (uint8_t)m_Mode;
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

				*m_STAT.get_ptr() &= ~3;
				*m_STAT.get_ptr() |= (uint8_t)m_Mode;
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
		bool filled[GameboyScreenWidth];
		memset(filled, false, sizeof(bool) * GameboyScreenWidth);

		draw_background(filled);

		// On DMG bit 5 window enable is overriden by bit 0 
		if (m_LCDC.is_bit_set(5) && m_LCDC.is_bit_set(0))
			draw_window();

		if (m_LCDC.is_bit_set(1))
			draw_sprites(filled);

		// We now want to blend with the last frame 

		auto lerp = [](float s, float e, float t)
			{
				return (s + (e - s) * t);
			};

		// Screen ghosting
		// This is used to emulator the original DMG ghosting, which was quite severe in some cases
		// SOme games may utilise it with sprites
		if (settings.enabledScreenGhosting)
		{
			for (uint16_t i = 0; i < GameboyScreenWidth; i++)
			{
				uint32_t pixelOffset = *m_LY * (GameboyScreenWidth * 4) + (i * 4);

				uint8_t r = m_Framebuffer[pixelOffset + 0];
				uint8_t g = m_Framebuffer[pixelOffset + 1];
				uint8_t b = m_Framebuffer[pixelOffset + 2];

				uint8_t old_r = m_LastFrame[pixelOffset + 0];
				uint8_t old_g = m_LastFrame[pixelOffset + 1];
				uint8_t old_b = m_LastFrame[pixelOffset + 2];

				// This next bit isn't very thought out but it works
				// Only show ghosting if the effect is above a threshold. This is to reduce a 'burn-in' like effect. 
				int accum = r + g + b;
				int accum_old = old_r + old_g + old_b;

				if (std::abs(accum - accum_old) >= 30)
				{
					m_Framebuffer[pixelOffset + 0] = (uint8_t)lerp((float)r, (float)old_r, settings.ghostAmount);
					m_Framebuffer[pixelOffset + 1] = (uint8_t)lerp((float)g, (float)old_g, settings.ghostAmount);
					m_Framebuffer[pixelOffset + 2] = (uint8_t)lerp((float)b, (float)old_b, settings.ghostAmount);
				}
			}
		}
	}

	void PPU::draw_background(bool* filled)
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

			if (paletteIndex > 0)
				filled[screenX] = true;
			

		}
	
	}

	void PPU::draw_window()
	{
		uint8_t winY = m_MMU->read8(0xFF4A);

		// If the window is too far scrolled don't draw this scanline 
		if (winY > *m_LY)
			return;

		winY = *m_LY - winY;

		for (uint16_t x = 0; x < GameboyScreenWidth; x++)
		{
			uint8_t winX = x + m_MMU->read8(0xFF4B) - 7;

			uint16_t addr = m_LCDC.is_bit_set(6) ? 0x9C00 : 0x9800;

			uint32_t tileX = winX / 8;
			uint32_t tileY = winY / 8;

			uint32_t tilePixelX = winX % 8;
			uint32_t tilePixelY = winY % 8;

			uint32_t tileIndex = tileY * 32 + tileX;

			uint32_t tileIdAddress = addr + tileIndex;

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

			plot_pixel(x, *m_LY, m_Palette.colours[pid].col[0], m_Palette.colours[pid].col[1], m_Palette.colours[pid].col[2]);
		}
	}

	void PPU::draw_sprites(bool* filled)
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

			// Get y and flip if needed
			int yp = *m_LY - (obj->y - 16);
			yp = attributes.is_bit_set(6) ? (7 + 8 * objSizeBit) - yp : yp;

			for (uint8_t x = 0; x < 8; x++)
			{
				int xp = obj->x + x - 8;

				// Check if this pixel is offscreen
				if (xp < 0 || xp >= 160)
					continue;



				uint16_t tileIdx = obj->tileIndex & (m_LCDC.is_bit_set(2) ? 0xFE : 0xFF);

				// int pixelOffset = (*m_LY * 160 * 4) + ((xp - 8) * 4);

				if (m_LCDC.is_bit_set(2) && yp >= 8)
					tileIdx += 1;

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

				bool priority = attributes.is_bit_set(7);

				if (!filled[xp ] || !priority)
					plot_pixel(xp, *m_LY, m_Palette.colours[pid].col[0], m_Palette.colours[pid].col[1], m_Palette.colours[pid].col[2]);

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