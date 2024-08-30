
#ifndef DEBUGUI_H
#define DEBUGUI_H

#include "../GBEX/GBEX.h"
#include <imgui.h>
#include <SDL3/SDL.h>

inline void debugui_cpu(gbex::CPU* _cpu, gbex::MMU* mmu)
{
	ImGui::Begin("CPU");

	ImGui::Text("Registers");
	{
		ImGui::Text("BC: %04x", _cpu->bc.get());
		ImGui::Text("DE: %04x", _cpu->de.get());
		ImGui::Text("HL: %04x", _cpu->hl.get());
		ImGui::Text("AF: %04x", _cpu->af.get());

		ImGui::Text("Flags: %c %c %c %c",
			_cpu->flag_zero() ? 'Z' : '-',
			_cpu->flag_subtract() ? 'N' : '-',
			_cpu->flag_half_carry() ? 'H' : '-',
			_cpu->flag_carry() ? 'C' : '-'
		);
	}

	ImGui::Text("SP: %04x", _cpu->sp);
	ImGui::Text("PC: %04x", _cpu->pc);

	ImGui::Separator();

	ImGui::Text("IME: %d", _cpu->interrupts.interrupts_enabled());

	ImGui::Separator();

	ImGui::Text("JOYP (0xFF00): %02x", mmu->read8(0xFF00));

	ImGui::Text("DIV (0xFF04): %02x", mmu->read8(0xFF04));
	ImGui::Text("TIMA (0xFF05): %02x", mmu->read8(0xFF05));
	ImGui::Text("TMA (0xFF06): %02x", mmu->read8(0xFF06));
	ImGui::Text("TAC (0xFF07): %02x", mmu->read8(0xFF07));


	ImGui::End();	
}

void renderVramTilesToTexture(SDL_Texture* texture, gbex::MMU* mmu)
{

	uint8_t* pixels = NULL;
	int pitch;
	SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

	memset(pixels, 0xFFFFFF, pitch * 8 * 24);

	uint8_t* tilesStart = mmu->get_memory_pointer(0x8000);

	SDL_PixelFormatDetails pixelFormat;
	pixelFormat.format = SDL_PIXELFORMAT_RGBA8888;



	uint8_t palette[4][4] = {
		{ 255, 255, 255, 255 },
		{ 190, 190, 190, 255 },
		{ 90, 90, 90, 255 },
		{ 0, 0, 0, 255 },
	};


	for (uint32_t i = 0; i < 384; i++)
	{
		uint32_t vramAddr = (i * 16 + 0x8000);
		for (uint8_t y = 0; y < 16; y += 2)
		{
			for (uint8_t x = 0; x < 8; x++)
			{
				uint8_t b1 = *mmu->get_memory_pointer(vramAddr + y);
				uint8_t b2 = *mmu->get_memory_pointer(vramAddr + y + 1);

				b1 = b1 >> (7 - x) & 1;
				b2 = b2 >> (7 - x) & 1;

				uint8_t paletteIndex = b1 | (b2 << 1);

				const uint32_t w = 16 * 8;

				int offsetX = ((i * 8 + x) % w);
				int offsetY = (y / 2) + (int(i / 16)) * 8;
				int offset = offsetY * (pitch / sizeof(unsigned int)) + offsetX;
				offset *= 4;

				pixels[offset + 3] = palette[paletteIndex][0];
				pixels[offset + 2] = palette[paletteIndex][1];
				pixels[offset + 1] = palette[paletteIndex][2];
				pixels[offset + 0] = palette[paletteIndex][3];

			}
		}
	}

	SDL_UnlockTexture(texture);

}

inline void debugui_ppu(gbex::PPU* ppu, gbex::MMU* mmu, SDL_Texture* vramTilesDebug)
{
	ImGui::Begin("PPU");

	ImGui::Checkbox("Enable Ghosting", &ppu->settings.enabledScreenGhosting);
	ImGui::SliderFloat("Ghost Amount", &ppu->settings.ghostAmount, 0.0f, 1.0f);

	ImGui::Separator();

	ImGui::Text("LCDC: %02x", mmu->read8(0xFF40));

	ImGui::Text("LY: %02x", mmu->read8(0xFF44));

	ImGui::Text("Mode: %d", (int)ppu->m_Mode);
	ImGui::Text("SCY (0xFF42): %02x", mmu->read8(0xFF42));
	ImGui::Text("SCX (0xFF43): %02x", mmu->read8(0xFF43));

	ImGui::Text("WY (0xFF42): %02x", mmu->read8(0xFF4A));
	ImGui::Text("WX (0xFF43): %02x", mmu->read8(0xFF4B));

	renderVramTilesToTexture(vramTilesDebug, mmu);

	ImGui::Image((void*)vramTilesDebug, ImVec2(8 * 16 * 2, 8 * 24 * 2));

	ImGui::End();
}

inline void debugui_debugger(gbex::gbex* emulator)
{
	ImGui::Begin("Debugger");

	if (ImGui::Button("Play"))
	{
		emulator->play();
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause"))
	{
		emulator->pause();
	}
	if (ImGui::Button("Step"))
	{
		emulator->step_intruction();
	}

	ImGui::End();
}

inline void debugui_output(gbex::gbex* emulator, SDL_Texture* output)
{
	ImGui::Begin("Output");

	ImGui::Image((void*)output, ImVec2(gbex::GameboyScreenWidth * 2, gbex::GameboyScreenHeight * 2));

	ImGui::End();
}

#endif // DEBUGUI_H