
#ifndef GBEX_INTERRUPTS_H
#define GBEX_INTERRUPTS_H

#include "BitValue.h"

namespace gbex
{
	class Interrupts
	{
	public:

		Interrupts();

		void set_master_flag(bool value);

	private:

		bool m_IME;


	};
}

#endif GBEX_INTERRUPTS_H