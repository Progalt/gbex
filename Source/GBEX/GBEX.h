
#ifndef GBEX_H
#define GBEX_H

#include "CPU.h"
#include "MMU.h"
#include "PPU.h"
#include <functional>

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

		void set_vsync_callback(std::function<void()> callback)
		{
			m_VsyncCallback = callback;
		}

		CPU m_CPU;
		MMU m_MMU;
		PPU m_PPU;

	private:

		std::function<void()> m_VsyncCallback;

		uint16_t m_Breakpoint = 0xFFFF;
		bool m_HitBreakpoint = false;

		DeviceType m_DeviceType = DeviceType::DMG;

	};
}

#endif // GBEX_H