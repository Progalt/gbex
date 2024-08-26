
#include "CPU.h"

namespace gbex
{
	void CPU::opcode_inc(Register& reg)
	{
		reg.increment();

		set_flag_zero(reg.get() == 0);
		set_flag_subtract(false);
		set_flag_half_carry((reg.get() & 0x0F) == 0x00);
	}

	void CPU::opcode_inc(uint8_t* value)
	{
		*value += 1;

		set_flag_zero(*value == 0);
		set_flag_subtract(false);
		set_flag_half_carry((*value & 0x0F) == 0x00);
	}

	void CPU::opcode_dec(Register& reg)
	{
		reg.decrement();

		set_flag_zero(reg.get() == 0);
		set_flag_subtract(true);
		set_flag_half_carry((reg.get() & 0x0F) == 0x0F);
	}

	void CPU::opcode_dec(uint8_t* value)
	{
		*value -= 1;

		set_flag_zero(*value == 0);
		set_flag_subtract(true);
		set_flag_half_carry((*value & 0x0F) == 0x0F);
	}

	void CPU::opcode_add(CombinedRegister& a, uint16_t v)
	{
		uint16_t t = a.get();
		uint32_t r = t + v;

		set_flag_subtract(false);
		set_flag_half_carry((t & 0x0FFF) + (v & 0x0FFF) > 0x0FFF);
		set_flag_carry((r & 0x10000) != 0);

		a.set((uint16_t)r);
	}

	void CPU::opcode_add(uint8_t v)
	{
		uint8_t i = a.get();
		uint32_t result = a.get() + v;

		a.set((uint8_t)result);

		set_flag_zero(a.get() == 0);
		set_flag_subtract(false);
		set_flag_half_carry((i & 0xf) + (v & 0xf) > 0xf);
		set_flag_carry((result & 0x100) != 0);
	}

	void CPU::opcode_adc(uint8_t v)
	{
		uint8_t r = a.get();
		uint8_t carry = flag_carry();

		uint32_t result32 = r + v + carry;
		uint8_t result = static_cast<uint8_t>(result32);

		set_flag_zero(result == 0);
		set_flag_subtract(false);
		set_flag_half_carry(((r & 0xf) + (v & 0xf) + carry) > 0xf);
		set_flag_carry(result32 > 0xff);

		a.set(result);
	}

	void CPU::opcode_sbc(uint8_t v)
	{
		uint8_t carry = flag_carry();
		uint8_t reg = a.get();

		int result32 = reg - v - carry;
		uint8_t result = (uint8_t)result32;

		set_flag_zero(result == 0);
		set_flag_subtract(true);
		set_flag_carry(result32 < 0);
		set_flag_half_carry(((reg & 0xf) - (v & 0xf) - carry) < 0);

		a.set(result);
	}

	void CPU::opcode_jr(bool condition)
	{
		if (condition)
		{
			
			int8_t jump = (int8_t)mmu->read8(pc++);

			pc += jump;

			tcycles += 12;
		}
		else
		{
			pc++;
			tcycles += 8;
		}
	}

	void CPU::opcode_jp(bool condition)
	{
		if (condition)
		{
			pc = mmu->read16(pc);
			tcycles += 16;
		}
		else
		{
			pc += 2;
			tcycles += 12;
		}
	}

	void CPU::opcode_call(bool condition)
	{
		tcycles += 12;
		uint16_t op = mmu->read16(pc);
		pc += 2;

		if (condition)
		{
			write_stack(pc);
			pc = op;
			tcycles += 12;
		}
	}

	void CPU::opcode_ret(bool condition)
	{
		if (condition)
		{
			pc = read_stack();
			tcycles += 20;
		}
		else
		{
			tcycles += 8;
		}
	}

	void CPU::opcode_xor(uint8_t val)
	{
		uint8_t r = a.get();

		uint8_t result = r ^ val;

		a.set(result);

		set_flag_zero(result == 0);
		set_flag_subtract(false);
		set_flag_half_carry(false);
		set_flag_carry(false);
	}

	void CPU::opcode_or(uint8_t val)
	{
		uint8_t r = a.get() | val;

		a.set(r);

		set_flag_zero(a.get() == 0);
		set_flag_half_carry(false);
		set_flag_carry(false);
		set_flag_subtract(false);
	}

	void CPU::opcode_cp(uint8_t val)
	{
		uint8_t result = (a.get() - val);

		set_flag_zero(result == 0);
		set_flag_subtract(true);
		set_flag_half_carry(((a.get() & 0xf) - (val & 0xf)) < 0);
		set_flag_carry(a.get() < val);
	}

	void CPU::opcode_and(uint8_t val)
	{
		uint8_t result = a.get() & val;

		a.set(result);

		set_flag_zero(a.get() == 0);
		set_flag_half_carry(true);
		set_flag_carry(false);
		set_flag_subtract(false);
	}

	void CPU::opcode_sub(uint8_t v)
	{
		uint8_t r = a.get();
		uint8_t result = r - v;

		a.set(result);

		set_flag_zero(a.get() == 0);
		set_flag_subtract(true);
		set_flag_half_carry(((r & 0xf) - (v & 0xf)) < 0);
		set_flag_carry(r < v);
	}

	uint8_t CPU::opcode_srl(uint8_t val)
	{
		bool set = is_bit_set(val, 0);

		uint8_t result = val >> 1;

		set_flag_carry(set);
		set_flag_zero(result == 0);
		set_flag_half_carry(false);
		set_flag_subtract(false);

		return result;
	}

	uint8_t CPU::opcode_rr(uint8_t val)
	{
		uint8_t carry = flag_carry();

		bool willCarry = is_bit_set(val, 0);
		set_flag_carry(willCarry);

		uint8_t result = val >> 1;
		result |= (carry << 7);

		set_flag_zero(result == 0);
		set_flag_subtract(false);
		set_flag_half_carry(false);

		return result;
	}

	uint8_t CPU::opcode_rlc(uint8_t val)
	{
		uint8_t carry = is_bit_set(val, 7);
		uint8_t trunc = is_bit_set(val, 7);
		uint8_t result = (uint8_t)((val << 1) | trunc);

		set_flag_carry(carry);
		set_flag_zero(result == 0);
		set_flag_half_carry(false);
		set_flag_subtract(false);

		return result;
	}

	uint8_t CPU::opcode_rrc(uint8_t val)
	{
		uint8_t carry_flag = is_bit_set(val, 0);
		uint8_t truncated_bit = is_bit_set(val, 0);
		uint8_t result = (uint8_t)((val >> 1) | (truncated_bit << 7));

		set_flag_carry(carry_flag);
		set_flag_zero(result == 0);
		set_flag_half_carry(false);
		set_flag_subtract(false);

		return result;
	}

	uint8_t CPU::opcode_rl(uint8_t val)
	{
		uint8_t carry = flag_carry();

		bool willCarry = is_bit_set(val, 7);
		set_flag_carry(willCarry);

		uint8_t result = (uint8_t)(val << 1);
		result |= carry;

		set_flag_zero(result == 0);

		set_flag_subtract(false);
		set_flag_half_carry(false);

		return result;
	}

	void CPU::opcode_bit(uint8_t bit, uint8_t val)
	{
		set_flag_zero(!is_bit_set(val, bit));
		set_flag_subtract(false);
		set_flag_half_carry(true);
	}

	uint8_t CPU::opcode_sla(uint8_t val)
	{
		uint8_t carry_bit = is_bit_set(val, 7);

		uint8_t result = (uint8_t)(val << 1);

		set_flag_zero(result == 0);
		set_flag_carry(carry_bit);
		set_flag_half_carry(false);
		set_flag_subtract(false);

		return result;
	}

	uint8_t CPU::opcode_sra(uint8_t val)
	{
		uint8_t carry_bit = is_bit_set(val, 0);
		uint8_t top_bit = is_bit_set(val, 7);

		uint8_t result = (uint8_t)(val >> 1);
		result = set_bit_to(result, 7, top_bit);

		set_flag_zero(result == 0);
		set_flag_carry(carry_bit);
		set_flag_half_carry(false);
		set_flag_subtract(false);

		return result;
	}

	uint8_t CPU::opcode_swap(uint8_t val)
	{
		uint8_t lower_nibble = val & 0x0F;
		uint8_t upper_nibble = (val & 0xF0) >> 4;

		uint8_t result = (lower_nibble << 4) | upper_nibble;
		
		set_flag_zero(result == 0);
		set_flag_subtract(false);
		set_flag_half_carry(false);
		set_flag_carry(false);

		return result;
	}

	uint8_t CPU::opcode_set(uint8_t bit, uint8_t v)
	{
		return set_bit_to(v, bit, true);
	}

	void CPU::opcode_set(uint8_t bit, Register& reg)
	{
		reg.set(opcode_set(bit, reg.get()));
	}

	uint8_t CPU::opcode_res(uint8_t bit, uint8_t v)
	{
		return clear_bit(v, bit);
	}

	void CPU::opcode_res(uint8_t bit, Register& reg)
	{
		reg.set(opcode_res(bit, reg.get()));
	}
}