
#include "Joypad.h"
#include "CPU.h"
#include "MMU.h"

namespace gbex
{
	void Joypad::initialise(CPU* _cpu, MMU* mmu)
	{
		m_MMU = mmu;
		m_CPU = _cpu;

		m_State = BitValue(0xFF);
	}

	void Joypad::set_button_down(JoypadButton button)
	{
		uint8_t joyp = m_MMU->read8(0xFF00);

		// Note: Joypad inputs are pulled high by default so 1... 0 is when we want to signal a button being pressed or to read from the lower nibble 

		// If the button is at 1 we are dropping it to 0
		// This calls an interrupt if enabled 

		// DPAD
		if (!is_bit_set(joyp, 4) && (uint8_t)button >= 4)
		{
			if (m_State.is_bit_set((uint8_t)button) && m_CPU->interrupts.is_interrupt_enabled(InterruptSource::Joypad))
			{
				m_CPU->interrupts.set_interrupt_flag(InterruptSource::Joypad);
			}
		}

		if (!is_bit_set(joyp, 5) && (uint8_t)button < 4)
		{
			if (m_State.is_bit_set((uint8_t)button) && m_CPU->interrupts.is_interrupt_enabled(InterruptSource::Joypad))
			{
				m_CPU->interrupts.set_interrupt_flag(InterruptSource::Joypad);
			}
		}

		// 
		m_State.set_bit((uint8_t)button, false);
	}

	void Joypad::set_button_released(JoypadButton button)
	{
		m_State.set_bit((uint8_t)button, true);
	}

	uint8_t Joypad::get_button_readings(uint8_t ff00)
	{
		// Select dpad
		if (!is_bit_set(ff00, 4) && is_bit_set(ff00, 5))
		{
			return 0xC0 | ff00 | (((uint8_t)m_State) >> 4);
		}
		else if (!is_bit_set(ff00, 5) && is_bit_set(ff00, 4))
		{
			return  0xC0 | ff00 | (((uint8_t)m_State) & 0x0F);
		}
		else
		{
			// Return all buttons released 
			return  0xC0 | ff00 | 0x0F;
		}
	}
}