
#include "Timer.h"
#include "MMU.h"
#include "CPU.h"

namespace gbex
{
	void Timer::initialise(CPU* _cpu, MMU* mmu)
	{
		m_TIMA = mmu->get_memory_pointer(0xFF05);
		m_TMA = mmu->get_memory_pointer(0xFF06);
		m_TAC = BitAddress(mmu->get_memory_pointer(0xFF07));

		m_CPU = _cpu;
	}

	void Timer::step(uint16_t tcycles)
	{
		m_InternalDIV += tcycles;


		if (m_Overflow)
		{
			m_Overflow = false;
			m_CPU->interrupts.set_interrupt_flag(InterruptSource::Timer);
			*m_TIMA = *m_TMA;
		}

		// Get the first 2 bits of tac
		uint8_t clockSelect = uint8_t(m_TAC) & 0x3;

		m_Cycles += tcycles;

		uint16_t limit = 0;

		switch (clockSelect)
		{
		case 0:
			limit = (256 * 4);
			break;
		case 1:
			limit = (4 * 4);
			break;
		case 2:
			limit = (16 * 4);
			break;
		case 3:
			limit = (64 * 4);
			break;
		}

		while (m_Cycles >= limit && uint8_t(m_TAC) & 0x4)
		{
			m_Cycles = m_Cycles - limit;

			increment_tima();
		}
	}

	void Timer::reset_div()
	{
		m_InternalDIV = 0;
		*m_TIMA = 0;
	}

	uint8_t Timer::get_div()
	{
		// Only the upper 8 bits are mapped
		return (m_InternalDIV >> 8);
	}

	void Timer::increment_tima()
	{
		// If it equals 0xFF we set overflow
		// But for one cycle it is 0x00 because it has overflowed
		if (*m_TIMA == 0xFF)
		{
			m_Overflow = true;
		}
		
		*m_TIMA += 1;
		
	}
}