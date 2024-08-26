
#ifndef GBEX_INTERRUPTS_H
#define GBEX_INTERRUPTS_H

#include "BitValue.h"

namespace gbex
{
	enum class InterruptSource
	{
		VBlank = (1 << 0),
		LCD = (1 << 1),
		Timer = (1 << 2),
		Serial = (1 << 3),
		Joypad = (1 << 4),
	};

	class Interrupts
	{
	public:

		Interrupts();

		void set_master_flag(bool value);

		bool interrupts_enabled() const { return m_IME; }

		bool is_interrupt_enabled(InterruptSource source);

	private:

		bool m_IME;


	};
}

#endif GBEX_INTERRUPTS_H