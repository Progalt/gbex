#ifndef REGISTER_H
#define REGISTER_H

#include <cstdint>

namespace gbex
{

	inline uint8_t clear_bit(uint8_t v, uint8_t bit)
	{
		return (v &= ~(1 << bit));
	}

	inline uint8_t set_bit_to(uint8_t v, uint8_t bit, bool value)
	{
		uint8_t t = clear_bit(v, bit);
		if (!value)
			return t;

		return (t | (1 << bit));
	}

	inline bool is_bit_set(uint8_t v, uint8_t bit)
	{
		return (bool)((v >> bit) & 1);
	}

	/*
		BitValue is like any regular value but allows the individual bits to be manipulated
	*/
	class BitValue 
	{
	public:

		BitValue() : m_Byte(0x0) { }

		BitValue(uint8_t b) : m_Byte(b) { }

		void set_bit(uint8_t idx, bool value)
		{
			m_Byte = set_bit_to(m_Byte, idx, value);
		}

		bool is_bit_set(uint8_t idx)
		{
			return (bool)((m_Byte >> idx) & 1);
		}

		void clear_bit(uint8_t idx)
		{
			m_Byte &= ~(1 << idx);
		}

		operator uint8_t() const { return m_Byte; }

		bool operator==(BitValue& value)
		{
			return m_Byte = value.m_Byte;
		}

	private:

		uint8_t m_Byte;
	};
}

#endif // REGISTER_H