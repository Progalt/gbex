
#ifndef GBEX_PPU_H
#define GBEX_PPU_H

#include <functional>

namespace gbex
{
	class CPU;

	class PPU
	{
	public:

		void step();

	private:

		friend class gbex;

		CPU* m_CPU;

		std::function<void()> m_VsyncCallback;

		uint32_t m_Dots = 0;

		BitAddress m_LCDC;
	};
}

#endif // GBEX_PPU_H