
#ifndef GBEX_CPU_H
#define GBEX_CPU_H

#include <cstdint>
#include "BitValue.h"
#include "Cartridge.h"
#include "Interrupts.h"
#include "MMU.h"
#include <cstdio>

namespace gbex
{

	class Register
	{
	public:

		Register() : m_Value(0x0) { }
		Register(uint8_t v) : m_Value(v) { }

		void set(uint8_t v) { m_Value = v; }
		uint8_t get() { return m_Value; }

		void set_bit(uint8_t bit, bool value)
		{
			m_Value = set_bit_to(m_Value, bit, value);
		}

		bool is_bit_set(uint8_t bit)
		{
			return (bool)((m_Value >> bit) & 1);
		}

		void increment()
		{
			m_Value++;
		}

		void decrement()
		{
			m_Value--;
		}

	protected:

		uint8_t m_Value;
	};


	class CombinedRegister
	{
	public:

		CombinedRegister(Register& high, Register& low) : m_High(high), m_Low(low) { }

		void set(uint16_t v)
		{
			m_Low.set((uint8_t)(v & 0xFF));
			m_High.set((uint8_t)((v >> 8) & 0xFF));
		}

		uint16_t get() const
		{
			return (((uint16_t)m_High.get()) << 8) | ((uint16_t)m_Low.get());
		}

		void increment()
		{
			set(get() + 1);
		}

		void decrement()
		{
			set(get() - 1);
		}

	private:

		Register& m_Low;
		Register& m_High;
	};

	class CPU
	{
	public:

		CPU();
		~CPU();

		Register a, f, b, c, d, e, h, l;


		CombinedRegister af, bc, de, hl;

		uint16_t sp;		// Stack Pointer
		uint16_t pc;		// Program Counter

		Interrupts interrupts;

		uint64_t tcycles;

		bool is_halted;

		void initialise_registers_dmg(CartridgeHeader& header);

		void initialise_registers_cgb(CartridgeHeader& header);

		void step();

		void set_flag_zero(bool value);
		void set_flag_carry(bool value);
		void set_flag_half_carry(bool value);
		void set_flag_subtract(bool value);


		bool flag_zero();
		bool flag_carry();
		bool flag_half_carry();
		bool flag_subtract();

		void write_stack(uint16_t value);

		uint16_t read_stack();

	private:

		friend class gbex;

		void execute_next_instruction();

		void execute_extended_instructions();

		MMU* mmu;

		void opcode_inc(Register& reg);
		void opcode_inc(uint8_t* value);

		void opcode_dec(Register& reg);
		void opcode_dec(uint8_t* value);

		void opcode_add(CombinedRegister& a, uint16_t v);
		void opcode_add(uint8_t v);

		void opcode_adc(uint8_t v);
		void opcode_sbc(uint8_t v);

		void opcode_sub(uint8_t v);

		void opcode_jr(bool condition);

		void opcode_jp(bool condition);

		void opcode_call(bool condition);

		void opcode_ret(bool condition);

		void opcode_xor(uint8_t val);
		void opcode_or(uint8_t val);
		void opcode_cp(uint8_t val);
		void opcode_and(uint8_t val);

		uint8_t opcode_srl(uint8_t val);
		uint8_t opcode_rr(uint8_t val);
		uint8_t opcode_rlc(uint8_t val);
		uint8_t opcode_rrc(uint8_t val);
		uint8_t opcode_rl(uint8_t val);
		uint8_t opcode_sla(uint8_t val);
		uint8_t opcode_sra(uint8_t val);
		uint8_t opcode_swap(uint8_t val);
		uint8_t opcode_set(uint8_t bit, uint8_t v);
		void opcode_set(uint8_t bit, Register& reg);
		uint8_t opcode_res(uint8_t bit, uint8_t v);
		void opcode_res(uint8_t bit, Register& reg);

		void opcode_bit(uint8_t bit, uint8_t val);

	};
}

#endif // GBEX_CPU_H