
#ifndef GBEX_TIMER_H
#define GBEX_TIMER_H
#include <cstdint>
#include "BitValue.h"

namespace gbex
{
	class MMU;
	class CPU;

	class Timer
	{
	public:

		void initialise(CPU* _cpu, MMU* mmu);

		void step(uint16_t tcycles);

		void reset_div();

		uint8_t get_div();

	private:

		uint16_t m_InternalDIV = 0xAB << 8;

		uint8_t* m_TIMA;
		uint8_t* m_TMA;
		BitAddress m_TAC;

		CPU* m_CPU;

		uint16_t m_Cycles = 0;

		bool m_Overflow = false;

		void increment_tima();
	};
}

#endif // GBEX_TIMER_H