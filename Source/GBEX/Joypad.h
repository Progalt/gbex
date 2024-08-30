
#ifndef GBEX_JOYPAD_H
#define GBEX_JOYPAD_H

#include <cstdint>
#include "BitValue.h"

namespace gbex
{
	class MMU;
	class CPU;

	enum class JoypadButton
	{
		A,
		B,
		SELECT,
		START,

		RIGHT,
		LEFT,
		UP,
		DOWN
	};

	class Joypad
	{
	public:

		void initialise(CPU* _cpu, MMU* mmu);

		void set_button_down(JoypadButton button);

		void set_button_released(JoypadButton button);

		uint8_t get_button_readings(uint8_t ff00);

	private:

		BitValue m_State;
		MMU* m_MMU;
		CPU* m_CPU;

	};
}

#endif // GBEX_JOYPAD_H