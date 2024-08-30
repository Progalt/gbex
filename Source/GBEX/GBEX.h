
#ifndef GBEX_H
#define GBEX_H

#include "CPU.h"
#include "MMU.h"
#include "PPU.h"
#include "Joypad.h"
#include <functional>
#include "Timer.h"

namespace gbex
{
	enum class DeviceType
	{
		DMG, 
		CGB
	};

	class gbex
	{
	public:

		void set_device_type(DeviceType type)
		{
			m_DeviceType = type;
		}

		bool load_cartridge(uint8_t* rom, const size_t size);

		void unload_cartridge();

		void step();

		Cartridge* get_cartridge() const { return m_MMU.m_Cartridge; }

		void set_breakpoint(uint16_t target_pc);

		void reset_breakpoint() { m_HitBreakpoint = false; }

		void set_vsync_callback(std::function<void()> callback)
		{
			m_VsyncCallback = callback;
		}

		void play()
		{
			m_Halted = false;
			m_Step = false;
			// m_HitBreakpoint = false;
		}

		void pause()
		{
			m_Halted = true;
		}

		void step_intruction()
		{
			m_Step = true;
			m_Halted = false;
		}

		bool is_halted() const 
		{
			return m_Halted;
		}

		uint8_t* get_framebuffer() const
		{
			return m_PPU.m_Framebuffer.get();
		}

		Joypad* get_joypad()
		{
			return &m_Joypad;
		}

		void set_dmg_colour_palette(Palette palette)
		{
			m_PPU.m_Palette = palette;
		}

		CPU m_CPU;
		MMU m_MMU;
		PPU m_PPU;
		Joypad m_Joypad;
		Timer m_Timer;

	private:

		std::function<void()> m_VsyncCallback;

		uint16_t m_Breakpoint = 0xFFFF;
		bool m_HitBreakpoint = false;

		bool m_Step = false;
		bool m_Halted = false;

		DeviceType m_DeviceType = DeviceType::DMG;

	};
}

#endif // GBEX_H