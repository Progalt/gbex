
#ifndef GBEX_H
#define GBEX_H

#include "CPU.h"
#include "MMU.h"

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


		CPU m_CPU;
		MMU m_MMU;

	private:

		uint16_t m_Breakpoint = 0xFFFF;
		bool m_HitBreakpoint = false;

		DeviceType m_DeviceType = DeviceType::DMG;

	};
}

#endif // GBEX_H