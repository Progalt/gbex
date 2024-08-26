
#ifndef GBEX_H
#define GBEX_H

#include "CPU.h"
#include "MMU.h"

namespace gbex
{
	class gbex
	{
	public:

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

	};
}

#endif // GBEX_H