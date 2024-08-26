
#include "Interrupts.h"

namespace gbex
{
	Interrupts::Interrupts() : m_IME(false)
	{

	}

	void Interrupts::set_master_flag(bool value)
	{
		m_IME = value;
	}
}