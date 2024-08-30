
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

	void Interrupts::set_interrupt_flag(InterruptSource source)
	{
		m_IF.set_bit((uint8_t)source, true);
	}

	void Interrupts::handle_interrupts()
	{
		uint8_t pending = m_IE.get() & m_IF.get();

		// TODO: Make sure halt bug is handled correctly
		if (!m_IME && pending)
		{
			m_ParentCPU->is_halted = false;
		}

		// If interrupts are disabled just return 
		if (!m_IME)
			return;

	

		if (m_ParentCPU->is_halted && pending)
			m_ParentCPU->is_halted = false;

		if (!pending)
			return;


		if (is_interrupt_requested(InterruptSource::VBlank) && is_interrupt_enabled(InterruptSource::VBlank))
		{
			jump_to_interrupt(InterruptSource::VBlank);
		}

		if (is_interrupt_requested(InterruptSource::LCD) && is_interrupt_enabled(InterruptSource::LCD))
		{
			jump_to_interrupt(InterruptSource::LCD);
		}

		if (is_interrupt_requested(InterruptSource::Timer) && is_interrupt_enabled(InterruptSource::Timer))
		{
			jump_to_interrupt(InterruptSource::Timer);
		}

		if (is_interrupt_requested(InterruptSource::Serial) && is_interrupt_enabled(InterruptSource::Serial))
		{
			jump_to_interrupt(InterruptSource::Serial);
		}

		if (is_interrupt_requested(InterruptSource::Joypad) && is_interrupt_enabled(InterruptSource::Joypad))
		{
			jump_to_interrupt(InterruptSource::Joypad);
		}
	}

	void Interrupts::jump_to_interrupt(InterruptSource source)
	{
		set_master_flag(false);
		m_IF.set_bit((uint8_t)source, false);
		
		m_ParentCPU->write_stack(m_ParentCPU->pc);

		switch (source)
		{
		case InterruptSource::VBlank:
			m_ParentCPU->pc = 0x40;
			break;
		case InterruptSource::LCD:
			m_ParentCPU->pc = 0x48;
			break;
		case InterruptSource::Timer:
			m_ParentCPU->pc = 0x50;
			break;
		case InterruptSource::Serial:
			m_ParentCPU->pc = 0x58;
			break;
		case InterruptSource::Joypad:
			m_ParentCPU->pc = 0x60;
			break;
		}
	}
}