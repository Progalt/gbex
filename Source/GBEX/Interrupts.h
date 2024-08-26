
#ifndef GBEX_INTERRUPTS_H
#define GBEX_INTERRUPTS_H

#include "BitValue.h"

namespace gbex
{
	enum class InterruptSource
	{
		VBlank = 0,
		LCD = 1,
		Timer = 2,
		Serial = 3,
		Joypad = 4
	};

	class MMU;
	class CPU;

	class Interrupts
	{
	public:

		Interrupts() : m_IME(false) { }

		Interrupts(CPU* _cpu, MMU* mmu);

		void set_master_flag(bool value);

		bool interrupts_enabled() const { return m_IME; }

		bool is_interrupt_enabled(InterruptSource source);

		bool is_interrupt_requested(InterruptSource source);

		void handle_interrupts();

	private:

		bool m_IME;

		BitAddress m_IE;
		BitAddress m_IF;

		
		CPU* m_ParentCPU;
	};
}

#endif GBEX_INTERRUPTS_H