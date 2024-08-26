
#include "PPU.h"
#include "CPU.h"

namespace gbex
{
	void PPU::step()
	{

		// 1 dot is equivalent to 4 tcycles or 1 mcycle
		m_Dots += m_CPU->tcycles;


	}
}