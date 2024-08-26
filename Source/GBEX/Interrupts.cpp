
#include "Interrupts.h"
#include "MMU.h"
#include "CPU.h"

namespace gbex
{
	Interrupts::Interrupts(CPU* _cpu, MMU* mmu) : m_IME(false)
	{
		m_IE = BitAddress(mmu->get_memory_pointer(0xFFFF));
		m_IF = BitAddress(mmu->get_memory_pointer(0xFF0F));

		m_ParentCPU = _cpu;
	}

	void Interrupts::set_master_flag(bool value)
	{
		m_IME = value;
	}

	bool Interrupts::is_interrupt_enabled(InterruptSource source)
	{
		return m_IE.is_bit_set((uint8_t)source);
	}

	bool Interrupts::is_interrupt_requested(InterruptSource source)
	{
		return m_IF.is_bit_set((uint8_t)source);
	}

	void Interrupts::handle_interrupts()
	{
		// If interrupts are disabled just return 
		if (!m_IME)
			return;

		uint8_t pending = m_IE.get() & m_IF.get();

		if (m_ParentCPU->is_halted && pending)
			m_ParentCPU->is_halted = false;

		if (!pending)
			return;
	}
}