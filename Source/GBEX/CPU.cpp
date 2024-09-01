
#include "CPU.h"
#include "OpcodeTable.h"
#include <cstdio>
#include <stdexcept>

namespace gbex
{
	CPU::CPU() : af(a, f), bc(b, c), de(d, e), hl(h, l), pc(0x0), sp(0x0), is_halted(false), tcycles(0x0)
	{
	}

	CPU::~CPU()
	{
	}

	void CPU::initialise_registers_dmg(CartridgeHeader& header)
	{
		// We initialise the registers to set values

		pc = 0x100;
		sp = 0xFFFE;

		a.set(0x01);

		b.set(0x00);
		c.set(0x13);
		d.set(0x00);
		e.set(0xD8);
		h.set(0x01);
		l.set(0x4D);

		set_flag_zero(true);
		set_flag_subtract(false);
		set_flag_carry(true);
		set_flag_half_carry(true);
	}

	void CPU::initialise_registers_cgb(CartridgeHeader& header)
	{
		pc = 0x100;
		sp = 0xFFFE;

		a.set(0x11);

		b.set(0x00);
		c.set(0x00);
		d.set(0xFF);
		e.set(0x56);
		h.set(0x00);
		l.set(0x0D);

		set_flag_zero(true);
		set_flag_subtract(false);
		set_flag_carry(false);
		set_flag_half_carry(false);
	}

	void CPU::step()
	{
		// Reset the number of cycles executed
		tcycles = 0;

		// If we are halted don't step 
		if (is_halted)
		{
			tcycles += 4;
			return;
		}

		execute_next_instruction();

	}


	void CPU::set_flag_zero(bool value)
	{
		f.set_bit(7, value);
	}

	void CPU::set_flag_carry(bool value)
	{
		f.set_bit(4, value);
	}

	void CPU::set_flag_half_carry(bool value)
	{
		f.set_bit(5, value);
	}

	void CPU::set_flag_subtract(bool value)
	{
		f.set_bit(6, value);
	}

	bool CPU::flag_zero()
	{
		return f.is_bit_set(7);
	}

	bool CPU::flag_carry()
	{
		return f.is_bit_set(4);
	}

	bool CPU::flag_half_carry()
	{
		return f.is_bit_set(5);
	}

	bool CPU::flag_subtract()
	{
		return f.is_bit_set(6);
	}


	void CPU::write_stack(uint16_t value)
	{
		sp -= 2;
		mmu->write16(sp, value);

	}

	uint16_t CPU::read_stack()
	{
		uint16_t v = mmu->read16(sp);
		sp += 2;

		return v;
	}

	void CPU::execute_next_instruction()
	{
		// Read the Opcode from the ROM
		uint8_t opcode = mmu->read8(pc);

		Instruction& instr = instructions[opcode];

		tcycles += instr.cycles;

		/*fprintf(logFile, "A:%02x F:%02x B:%02x C:%02x D:%02x E:%02x H:%02x L:%02x SP:%04x PC:%04x PCMEM:%02x,%02x,%02x,%02x\n",
			a.get(), f.get(), b.get(), c.get(), d.get(), e.get(), h.get(), l.get(), sp, pc,
			mmu->read8(pc), mmu->read8(pc + 1), mmu->read8(pc + 2), mmu->read8(pc + 3)
		);*/

		// We increment here to go past the opcode 
		pc++;

		switch (opcode)
		{
		case 0x00:			// NOP
			// We do nothing
			break;
		case 0x01:			// LD BC, n16
			bc.set(mmu->read16(pc));
			pc += 2;
			break;
		case 0x02:			// LD [BC], A
			mmu->write8(bc.get(), a.get());
			break;
		case 0x03:			// INC BC
			bc.increment();
			break;
		case 0x04:			// INC B
			opcode_inc(b);
			break;
		case 0x05:			// DEC B
			opcode_dec(b);
			break;
		case 0x06:			// LD B, n8
			b.set(mmu->read8(pc++));
			break;
		case 0x07:			// RLCA
			a.set(opcode_rlc(a.get()));
			set_flag_zero(false);
			break;
		case 0x08:			// LD [a16], SP
			mmu->write16(mmu->read16(pc), sp);
			pc += 2;
			break;
		case 0x09:			// ADD HL, BC
			opcode_add(hl, bc.get());
			break;
		case 0x0A:			// LD A, [BC]
			a.set(mmu->read8(bc.get()));
			break;
		case 0x0B:			// DEC BC
			bc.decrement();
			break;
		case 0x0C:			// INC C
			opcode_inc(c);
			break;
		case 0x0D:			// DEC C
			opcode_dec(c);
			break;
		case 0x0E:			// LD C, n8
			c.set(mmu->read8(pc++));
			break;
		case 0x0F:			// RRCA
			a.set(opcode_rrc(a.get()));
			set_flag_zero(false);
			break;
		case 0x10:			// STOP n8
			pc++;			// Just increment the pc
			break;
		case 0x11:			// LD DE, n16
			de.set(mmu->read16(pc));
			pc += 2;
			break;
		case 0x12:			// LD [DE], A
			mmu->write8(de.get(), a.get());
			break;
		case 0x13:			// INC DE
			de.increment();
			break;
		case 0x14:			// INC D
			opcode_inc(d);
			break;
		case 0x15:			// DEC D
			opcode_dec(d);
			break;
		case 0x16:			// LD D, n8
			d.set(mmu->read8(pc++));
			break;
		case 0x17:			// RLA
			a.set(opcode_rl(a.get()));
			set_flag_zero(false);
			break;
		case 0x18:			// JR e8
		{
			int8_t offset = (int8_t)mmu->read8(pc++);
			pc += offset;
		}
		break;
		case 0x19:			// ADD HL, DE
			opcode_add(hl, de.get());
			break;
		case 0x1A:			// LD A, DE
			a.set(mmu->read8(de.get()));
			break;
		case 0x1B:			// DEC DE
			de.decrement();
			break;
		case 0x1C:			// INC E
			opcode_inc(e);
			break;
		case 0x1D:			// DEC E
			opcode_dec(e);
			break;
		case 0x1E:			// LD E, n8
			e.set(mmu->read8(pc++));
			break;
		case 0x1F:			// RRA
			a.set(opcode_rr(a.get()));
			set_flag_zero(false);
			break;
		case 0x20:			// JR NZ, e8
			opcode_jr(!flag_zero());
			break;
		case 0x21:			// LD HL, n16
			hl.set(mmu->read16(pc));
			pc += 2;
			break;
		case 0x22:			// LD [HL+], A
			mmu->write8(hl.get(), a.get());
			hl.increment();
			break;
		case 0x23:			// INC HL
			hl.increment();
			break;
		case 0x24:			// INC H
			opcode_inc(h);
			break;
		case 0x25:			// DEC H
			opcode_dec(h);
			break;
		case 0x26:			// LD H, n8
			h.set(mmu->read8(pc++));
			break;
		case 0x27:			// DAA
		{
			uint8_t r = a.get();

			uint16_t correct = flag_carry() ? 0x60 : 0x00;

			if (flag_half_carry() || (!flag_subtract() && ((r & 0x0F) > 9)))
			{
				correct |= 0x06;
			}

			if (flag_carry() || (!flag_subtract() && (r > 0x99)))
			{
				correct |= 0x60;
			}

			if (flag_subtract())
			{
				r = (uint8_t)(r - correct);
			}
			else
			{
				r = (uint8_t)(r + correct);
			}

			if (((correct << 2) & 0x100) != 0)
			{
				set_flag_carry(true);
			}

			set_flag_half_carry(false);
			set_flag_zero(r == 0);

			a.set(r);
		}
			break;
		case 0x28:			// JR Z, e8
			opcode_jr(flag_zero());
			break;
		case 0x29:			// ADD HL, HL
			opcode_add(hl, hl.get());
			break;
		case 0x2A:			// LD A, [HL+]
			a.set(mmu->read8(hl.get()));
			hl.increment();
			break;
		case 0x2B:			// DEC HL
			hl.decrement();
			break;
		case 0x2C:			// INC L
			opcode_inc(l);
			break;
		case 0x2D:			// DEC L
			opcode_dec(l);
			break;
		case 0x2E:			// LD L, n8
			l.set(mmu->read8(pc++));
			break;
		case 0x2F:			// CPL
		{
			uint8_t r = a.get();
			uint8_t result = ~r;
			a.set(result);

			set_flag_subtract(true);
			set_flag_half_carry(true);
		}
			break;
		case 0x30:			// JR NC, e8
			opcode_jr(!flag_carry());
			break;
		case 0x31:			// LD SP, n16
			sp = mmu->read16(pc);
			pc += 2;
			break;
		case 0x32:			// LD [HL-], A
			mmu->write8(hl.get(), a.get());
			hl.decrement();
			break;
		case 0x33:			// INC SP
			sp++;
			break;
		case 0x34:			// INC [HL]
		{
			uint8_t data = mmu->read8(hl.get());
			opcode_inc(&data);
			mmu->write8(hl.get(), data);
		}
			break;
		case 0x35:			// DEC [HL]
		{
			uint8_t data = mmu->read8(hl.get());
			opcode_dec(&data);
			mmu->write8(hl.get(), data);
		}
			break;
		case 0x36:			// LD [HL], n8
			mmu->write8(hl.get(), mmu->read8(pc++));
			break;
		case 0x37:			// SCF
			set_flag_carry(true);
			set_flag_half_carry(false);
			set_flag_subtract(false);
			break;
		case 0x38:			// JR C, e8
			opcode_jr(flag_carry());
			break;
		case 0x39:			// ADD HL, SP
			opcode_add(hl, sp);
			break;
		case 0x3A:			// LD A, [HL-]
			a.set(mmu->read8(hl.get()));
			hl.decrement();
			break;
		case 0x3B:			// DEC SP
			sp--;
			break;
		case 0x3C:			// INC A
			opcode_inc(a);
			break;
		case 0x3D:			// DEC A
			opcode_dec(a);
			break;
		case 0x3E:			// LD A, n8
			a.set(mmu->read8(pc++));
			break;
		case 0x3F:			// CCF
			set_flag_subtract(false);
			set_flag_half_carry(false);
			set_flag_carry(!flag_carry());
			break;
		case 0x40:			// LD B, B
			break;
		case 0x41:			// LD B, C
			b.set(c.get());
			break;
		case 0x42:			// LD B, D
			b.set(d.get());
			break;
		case 0x43:			// LD B, E
			b.set(e.get());
			break;
		case 0x44:			// LD B, H
			b.set(h.get());
			break;
		case 0x45:			// LD B, L
			b.set(l.get());
			break;
		case 0x46:			// LD B, [HL]
			b.set(mmu->read8(hl.get()));
			break;
		case 0x47:			// LD B, A
			b.set(a.get());
			break;
		case 0x48:			// LD C, B
			c.set(b.get());
			break;
		case 0x49:			// LD C, C
			break;
		case 0x4A:			// LD C, D
			c.set(d.get());
			break;
		case 0x4B:			// LD C, E
			c.set(e.get());
			break;
		case 0x4C:			// LD C, H
			c.set(h.get());
			break;
		case 0x4D:			// LD C, L
			c.set(l.get());
			break;
		case 0x4E:			// LD C, [HL]
			c.set(mmu->read8(hl.get()));
			break;
		case 0x4F:			// LD C, A
			c.set(a.get());
			break;
		case 0x50:			// LD D, B
			d.set(b.get());
			break;
		case 0x51:			// LD D, C
			d.set(c.get());
			break;
		case 0x52:			// LD D, D
			break;
		case 0x53:			// LD D, E
			d.set(e.get());
			break;
		case 0x54:			// LD D, H
			d.set(h.get());
			break;
		case 0x55:			// LD D, L
			d.set(l.get());
			break;
		case 0x56:			// LD D, [HL]
			d.set(mmu->read8(hl.get()));
			break;
		case 0x57:			// LD D, A
			d.set(a.get());
			break;
		case 0x58:			// LD E, B
			e.set(b.get());
			break;
		case 0x59:			// LD E, C
			e.set(c.get());
			break;
		case 0x5A:			// LD E, D
			e.set(d.get());
			break;
		case 0x5B:			// LD E, E
			break;
		case 0x5C:			// LD E, H
			e.set(h.get());
			break;
		case 0x5D:			// LD E, L
			e.set(l.get());
			break;
		case 0x5E:			// LD E, [HL]
			e.set(mmu->read8(hl.get()));
			break;
		case 0x5F:			// LD E, A
			e.set(a.get());
			break;
		case 0x60:			// LD H, B
			h.set(b.get());
			break;
		case 0x61:			// LD H, C
			h.set(c.get());
			break;
		case 0x62:			// LD H, D
			h.set(d.get());
			break;
		case 0x63:			// LD H, E
			h.set(e.get());
			break;
		case 0x64:			// LD H, H
			break;
		case 0x65:			// LD H, L
			h.set(l.get());
			break;
		case 0x66:			// LD H, [HL]
			h.set(mmu->read8(hl.get()));
			break;
		case 0x67:			// LD H, A
			h.set(a.get());
			break;
		case 0x68:			// LD L, B
			l.set(b.get());
			break;
		case 0x69:			// LD L, C
			l.set(c.get());
			break;
		case 0x6A:			// LD L, D
			l.set(d.get());
			break;
		case 0x6B:			// LD L, E
			l.set(e.get());
			break;
		case 0x6C:			// LD L, H
			l.set(h.get());
			break;
		case 0x6D:			// LD L, L
			break;
		case 0x6E:			// LD L, [HL]
			l.set(mmu->read8(hl.get()));
			break;
		case 0x6F:			// LD L, A
			l.set(a.get());
			break;
		case 0x70:			// LD [HL], B
			mmu->write8(hl.get(), b.get());
			break;
		case 0x71:			// LD [HL], C
			mmu->write8(hl.get(), c.get());
			break;
		case 0x72:			// LD [HL], D
			mmu->write8(hl.get(), d.get());
			break;
		case 0x73:			// LD [HL], E
			mmu->write8(hl.get(), e.get());
			break;
		case 0x74:			// LD [HL], H
			mmu->write8(hl.get(), h.get());
			break;
		case 0x75:			// LD [HL], L
			mmu->write8(hl.get(), l.get());
			break;
		case 0x76:			// HALT
		{
			is_halted = true;
		}
			break;
		case 0x77:			// LD [HL], A
			mmu->write8(hl.get(), a.get());
			break;
		case 0x78:			// LD A, B
			a.set(b.get());
			break;
		case 0x79:			// LD A, C
			a.set(c.get());
			break;
		case 0x7A:			// LD A, D
			a.set(d.get());
			break;
		case 0x7B:			// LD A, E
			a.set(e.get());
			break;
		case 0x7C:			// LD A, H
			a.set(h.get());
			break;
		case 0x7D:			// LD A, L
			a.set(l.get());
			break;
		case 0x7E:			// LD A, [HL]
			a.set(mmu->read8(hl.get()));
			break;
		case 0x7F:			// LD A, A
			break;
		case 0x80:			// ADD A, B
			opcode_add(b.get());
			break;
		case 0x81:			// ADD A, C
			opcode_add(c.get());
			break;
		case 0x82:			// ADD A, D
			opcode_add(d.get());
			break;
		case 0x83:			// ADD A, E
			opcode_add(e.get());
			break;
		case 0x84:			// ADD A, H
			opcode_add(h.get());
			break;
		case 0x85:			// ADD A, L
			opcode_add(l.get());
			break;
		case 0x86:			// ADD A, HL
			opcode_add(mmu->read8(hl.get()));
			break;
		case 0x87:			// ADD A, A
			opcode_add(a.get());
			break;
		case 0x88:			// ADC A, B
			opcode_adc(b.get());
			break;
		case 0x89:			// ADC A, C
			opcode_adc(c.get());
			break;
		case 0x8A:			// ADC A, D
			opcode_adc(d.get());
			break;
		case 0x8B:			// ADC A, E
			opcode_adc(e.get());
			break;
		case 0x8C:			// ADC A, H
			opcode_adc(h.get());
			break;
		case 0x8D:			// ADC A, L
			opcode_adc(l.get());
			break;
		case 0x8E:			// ADC A, HL
			opcode_adc(mmu->read8(hl.get()));
			break;
		case 0x8F:			// ADC A, A
			opcode_adc(a.get());
			break;
		case 0x90:			// SUB A, B
			opcode_sub(b.get());
			break;
		case 0x91:			// SUB A, C
			opcode_sub(c.get());
			break;
		case 0x92:			// SUB A, D
			opcode_sub(d.get());
			break;
		case 0x93:			// SUB A, E
			opcode_sub(e.get());
			break;
		case 0x94:			// SUB A, H
			opcode_sub(h.get());
			break;
		case 0x95:			// SUB A, L
			opcode_sub(l.get());
			break;
		case 0x96:			// SUB A, [HL]
			opcode_sub(mmu->read8(hl.get()));
			break;
		case 0x97:			// SUB A, A
			opcode_sub(a.get());
			break;
		case 0x98:			// SBC A, B
			opcode_sbc(b.get());
			break;
		case 0x99:			// SBC A, C
			opcode_sbc(c.get());
			break;
		case 0x9A:			// SBC A, D
			opcode_sbc(d.get());
			break;
		case 0x9B:			// SBC A, E
			opcode_sbc(e.get());
			break;
		case 0x9C:			// SBC A, H
			opcode_sbc(h.get());
			break;
		case 0x9D:			// SBC A, L
			opcode_sbc(l.get());
			break;
		case 0x9E:			// SBC A, [HL]
			opcode_sbc(mmu->read8(hl.get()));
			break;
		case 0x9F:			// SBC A, A
			opcode_sbc(a.get());
			break;
		case 0xA0:			// AND A, B
			opcode_and(b.get());
			break;
		case 0xA1:			// AND A, C
			opcode_and(c.get());
			break;
		case 0xA2:			// AND A, D
			opcode_and(d.get());
			break;
		case 0xA3:			// AND A, E
			opcode_and(e.get());
			break;
		case 0xA4:			// AND A, H
			opcode_and(h.get());
			break;
		case 0xA5:			// AND A, L
			opcode_and(l.get());
			break;
		case 0xA6:			// AND A, HL
			opcode_and(mmu->read8(hl.get()));
			break;
		case 0xA7:			// AND A, A
			opcode_and(a.get());
			break;
		case 0xA8:			// XOR A, B
			opcode_xor(b.get());
			break;
		case 0xA9:			// XOR A, C
			opcode_xor(c.get());
			break;
		case 0xAA:			// XOR A, D
			opcode_xor(d.get());
			break;
		case 0xAB:			// XOR A, E
			opcode_xor(e.get());
			break;
		case 0xAC:			// XOR A, H
			opcode_xor(h.get());
			break;
		case 0xAD:			// XOR A, L
			opcode_xor(l.get());
			break;
		case 0xAE:			// XOR A, [HL]
			opcode_xor(mmu->read8(hl.get()));
			break;
		case 0xAF:			// XOR A, A
			opcode_xor(a.get());
			break;
		case 0xB0:			// OR A, B
			opcode_or(b.get());
			break;
		case 0xB1:			// OR A, C
			opcode_or(c.get());
			break;
		case 0xB2:			// OR A, D
			opcode_or(d.get());
			break;
		case 0xB3:			// OR A, E
			opcode_or(e.get());
			break;
		case 0xB4:			// OR A, H
			opcode_or(h.get());
			break;
		case 0xB5:			// OR A, L
			opcode_or(l.get());
			break;
		case 0xB6:			// OR A, [HL]
			opcode_or(mmu->read8(hl.get()));
			break;
		case 0xB7:			// OR A, A
			opcode_or(a.get());
			break;
		case 0xB8:			// CP A, B
			opcode_cp(b.get());
			break;
		case 0xB9:			// CP A, C
			opcode_cp(c.get());
			break;
		case 0xBA:			// CP A, D
			opcode_cp(d.get());
			break;
		case 0xBB:			// CP A, E
			opcode_cp(e.get());
			break;
		case 0xBC:			// CP A, H
			opcode_cp(h.get());
			break;
		case 0xBD:			// CP A, L
			opcode_cp(l.get());
			break;
		case 0xBE:			// CP A, [HL]
			opcode_cp(mmu->read8(hl.get()));
			break;
		case 0xBF:			// CP A, A
			opcode_cp(a.get());
			break;
		case 0xC0:			// RET NZ
			opcode_ret(!flag_zero());
			break;
		case 0xC1:			// POP BC
			bc.set(read_stack());
			break;
		case 0xC2:			// JP NZ, a16
			opcode_jp(!flag_zero());
			break;
		case 0xC3:			// JP a16
		{
			uint16_t op = mmu->read16(pc);
			pc = op;
		}
			break;
		case 0xC4:			// CALL NZ, a16
			opcode_call(!flag_zero());
			break;
		case 0xC5:			// PUSH BC
			write_stack(bc.get());
			break;
		case 0xC6:			// ADD A, n8
			opcode_add(mmu->read8(pc++));
			break;
		case 0xC7:			// RST $00
			write_stack(pc);
			pc = 0x00;
			break;
		case 0xC8:			// RET Z
			opcode_ret(flag_zero());
			break;
		case 0xC9:			// RET
			pc = read_stack();
			break;
		case 0xCA:			// JP Z, a16
			opcode_jp(flag_zero());
			break;
		case 0xCB:			// PREFIX
			execute_extended_instructions();
			break;
		case 0xCC:			// CALL Z, a16
			opcode_call(flag_zero());
			break;
		case 0xCD:			// CALL a16
		{
			uint16_t addr = mmu->read16(pc);
			pc += 2;
			write_stack(pc);
			pc = addr;
		}
			break;
		case 0xCE:			// ADC A, n8
			opcode_adc(mmu->read8(pc++));
			break;
		case 0xCF:			// RST $08
			write_stack(pc);
			pc = 0x08;
			break;
		case 0xD0:			// RET NC
			opcode_ret(!flag_carry());
			break;
		case 0xD1:			// POP DE
			de.set(read_stack());
			break;
		case 0xD2:			// JP NC, a16
			opcode_jp(!flag_carry());
			break;
		case 0xD3:			// ILLEGAL_D3
			throw std::runtime_error("Unimplemented Instruction: 0xD3");
			break;
		case 0xD4:			// CALL NC, a16
			opcode_call(!flag_carry());
			break;
		case 0xD5:			// PUSH DE
			write_stack(de.get());
			break;
		case 0xD6:			// SUB A, n8
			opcode_sub(mmu->read8(pc++));
			break;
		case 0xD7:			// RST $10
			write_stack(pc);
			pc = 0x10;
			break;
		case 0xD8:			// RET C
			opcode_ret(flag_carry());
			break;
		case 0xD9:			// RETI
			pc = read_stack();
			interrupts.set_master_flag(true);
			break;
		case 0xDA:			// JP C, a16
			opcode_jp(flag_carry());
			break;
		case 0xDB:			// ILLEGAL_DB
			throw std::runtime_error("Unimplemented Instruction: 0xDB");
			break;
		case 0xDC:			// CALL C, a16
			opcode_call(flag_carry());
			break;
		case 0xDD:			// ILLEGAL_DD
			throw std::runtime_error("Unimplemented Instruction: 0xDD");
			break;
		case 0xDE:			// SBC A, n8
			opcode_sbc(mmu->read8(pc++));
			break;
		case 0xDF:			// RST $18
			write_stack(pc);
			pc = 0x18;
			break;
		case 0xE0:			// LDH a8, A
			mmu->write8(0xFF00 + mmu->read8(pc++), a.get());
			break;
		case 0xE1:			// POP HL
			hl.set(read_stack());
			break;
		case 0xE2:			// LD [C], A
			mmu->write8(0xFF00 + c.get(), a.get());
			break;
		case 0xE3:			// ILLEGAL_E3
			throw std::runtime_error("Unimplemented Instruction: 0xE3");
			break;
		case 0xE4:			// ILLEGAL_E4
			throw std::runtime_error("Unimplemented Instruction: 0xE4");
			break;
		case 0xE5:			// PUSH HL
			write_stack(hl.get());
			break;
		case 0xE6:			// AND A, n8
			opcode_and(mmu->read8(pc++));
			break;
		case 0xE7:			// RST $20
			write_stack(pc);
			pc = 0x20;
			break;
		case 0xE8:			// ADD SP, e8
		{
			uint16_t reg = sp;
			int8_t v = mmu->read8(pc++);

			int result = (int)(reg + v);

			set_flag_zero(false);
			set_flag_subtract(false);
			set_flag_half_carry(((reg ^ v ^ (result & 0xFFFF)) & 0x10) == 0x10);
			set_flag_carry(((reg ^ v ^ (result & 0xFFFF)) & 0x100) == 0x100);

			sp = (uint16_t)result;
		}
			break;
		case 0xE9:			// JP HL
			pc = hl.get();
			break;
		case 0xEA:			// LD [a16], A
		{
			uint16_t addr = mmu->read16(pc);
			mmu->write8(addr, a.get());
			pc += 2;
		}
			break;
		case 0xEB:			// ILLEGAL_EB
			throw std::runtime_error("Unimplemented Instruction: 0xEB");
			break;
		case 0xEC:			// ILLEGAL_EC
			throw std::runtime_error("Unimplemented Instruction: 0xEC");
			break;
		case 0xED:			// ILLEGAL_ED
			throw std::runtime_error("Unimplemented Instruction: 0xED");
			break;
		case 0xEE:			// XOR A, n8
			opcode_xor(mmu->read8(pc++));
			break;
		case 0xEF:			// RST $28
			write_stack(pc);
			pc = 0x28;
			break;
		case 0xF0:			// LDH A, [a8]
			a.set(mmu->read8(0xFF00 + mmu->read8(pc++)));
			break;
		case 0xF1:			// POP AF
			af.set(read_stack());
			// some bits are reserved on the flag register 
			f.set(f.get() & 0xF0);
			break;
		case 0xF2:			// LDH A, [C]
			a.set(mmu->read8(0xFF00 + c.get()));
			break;
		case 0xF3:			// DI
			interrupts.set_master_flag(false);
			break;
		case 0xF4:			// ILLEGAL_F4
			throw std::runtime_error("Unimplemented Instruction: 0xF4");
			break;
		case 0xF5:			// PUSH AF
			write_stack(af.get());
			break;
		case 0xF6:			// OR A, n8
			opcode_or(mmu->read8(pc++));
			break;
		case 0xF7:			// RST $30
			write_stack(pc);
			pc = 0x30;
			break;
		case 0xF8:			// LD HL, SP, e8
		{
			uint16_t v = sp;
			int8_t offset = (int8_t)mmu->read8(pc++);

			int result = (int)(v + offset);

			set_flag_zero(false);
			set_flag_subtract(false);
			set_flag_half_carry(((v ^ offset ^ (result & 0xFFFF)) & 0x10) == 0x10);
			set_flag_carry(((v ^ offset ^ (result & 0xFFFF)) & 0x100) == 0x100);

			hl.set((uint16_t)result);
		}
			break;
		case 0xF9:			// LD SP, HL
			sp = hl.get();
			break;
		case 0xFA:			// LD A, [a16]
			a.set(mmu->read8(mmu->read16(pc)));
			pc += 2;
			break;
		case 0xFB:			// EI
			interrupts.set_master_flag(true);
			break;
		case 0xFC:			// ILLEGAL_FC
			throw std::runtime_error("Unimplemented Instruction: 0xFC");
			break;
		case 0xFD:			// ILLEGAL_FD
			throw std::runtime_error("Unimplemented Instruction: 0xFD");
			break;
		case 0xFE:			// CP A, n8
			opcode_cp(mmu->read8(pc++));
			break;
		case 0xFF:			// RST $38
			write_stack(pc);
			pc = 0x38;
			break;

		}

		//printf("%s\t\t[PC]:%04x\t\n", instr.mnemonic, pc);

	}

	void CPU::execute_extended_instructions()
	{
		uint8_t opcode = mmu->read8(pc++);

		switch (opcode)
		{
		case 0x00:			// RLC B
			b.set(opcode_rlc(b.get()));
			break;
		case 0x01:			// RLC C
			c.set(opcode_rlc(c.get()));
			break;
		case 0x02:			// RLC D
			d.set(opcode_rlc(d.get()));
			break;
		case 0x03:			// RLC E
			e.set(opcode_rlc(e.get()));
			break;
		case 0x04:			// RLC H
			h.set(opcode_rlc(h.get()));
			break;
		case 0x05:			// RLC L
			l.set(opcode_rlc(l.get()));
			break;
		case 0x06:			// RLC HL
			mmu->write8(hl.get(), opcode_rlc(mmu->read8(hl.get())));
			break;
		case 0x07:			// RLC A
			a.set(opcode_rlc(a.get()));
			break;
		case 0x08:			// RRC B
			b.set(opcode_rrc(b.get()));
			break;
		case 0x09:			// RRC C
			c.set(opcode_rrc(c.get()));
			break;
		case 0x0A:			// RRC D
			d.set(opcode_rrc(d.get()));
			break;
		case 0x0B:			// RRC E
			e.set(opcode_rrc(e.get()));
			break;
		case 0x0C:			// RRC H
			h.set(opcode_rrc(h.get()));
			break;
		case 0x0D:			// RRC L
			l.set(opcode_rrc(l.get()));
			break;
		case 0x0E:			// RRC HL
			mmu->write8(hl.get(), opcode_rrc(mmu->read8(hl.get())));
			break;
		case 0x0F:			// RRC A
			a.set(opcode_rrc(a.get()));
			break;
		case 0x10:			// RL B
			b.set(opcode_rl(b.get()));
			break;
		case 0x11:			// RL C
			c.set(opcode_rl(c.get()));
			break;
		case 0x12:			// RL D
			d.set(opcode_rl(d.get()));
			break;
		case 0x13:			// RL E
			e.set(opcode_rl(e.get()));
			break;
		case 0x14:			// RL H
			h.set(opcode_rl(h.get()));
			break;
		case 0x15:			// RL L
			l.set(opcode_rl(l.get()));
			break;
		case 0x16:			// RL HL
			mmu->write8(hl.get(), opcode_rl(mmu->read8(hl.get())));
			break;
		case 0x17:			// RL A
			a.set(opcode_rl(a.get()));
			break;
		case 0x18:			// RR B
			b.set(opcode_rr(b.get()));
			break;
		case 0x19:			// RR C
			c.set(opcode_rr(c.get()));
			break;
		case 0x1A:			// RR D
			d.set(opcode_rr(d.get()));
			break;
		case 0x1B:			// RR E
			e.set(opcode_rr(e.get()));
			break;
		case 0x1C:			// RR H
			h.set(opcode_rr(h.get()));
			break;
		case 0x1D:			// RR L
			l.set(opcode_rr(l.get()));
			break;
		case 0x1E:			// RR HL
			mmu->write8(hl.get(), opcode_rr(mmu->read8(hl.get())));
			break;
		case 0x1F:			// RR A
			a.set(opcode_rr(a.get()));
			break;
		case 0x20:			// SLA B
			b.set(opcode_sla(b.get()));
			break;
		case 0x21:			// SLA C
			c.set(opcode_sla(c.get()));
			break;
		case 0x22:			// SLA D
			d.set(opcode_sla(d.get()));
			break;
		case 0x23:			// SLA E
			e.set(opcode_sla(e.get()));
			break;
		case 0x24:			// SLA H
			h.set(opcode_sla(h.get()));
			break;
		case 0x25:			// SLA L
			l.set(opcode_sla(l.get()));
			break;
		case 0x26:			// SLA HL
			mmu->write8(hl.get(), opcode_sla(mmu->read8(hl.get())));
			break;
		case 0x27:			// SLA A
			a.set(opcode_sla(a.get()));
			break;
		case 0x28:			// SRA B
			b.set(opcode_sra(b.get()));
			break;
		case 0x29:			// SRA C
			c.set(opcode_sra(c.get()));
			break;
		case 0x2A:			// SRA D
			d.set(opcode_sra(d.get()));
			break;
		case 0x2B:			// SRA E
			e.set(opcode_sra(e.get()));
			break;
		case 0x2C:			// SRA H
			h.set(opcode_sra(h.get()));
			break;
		case 0x2D:			// SRA L
			l.set(opcode_sra(l.get()));
			break;
		case 0x2E:			// SRA HL
			mmu->write8(hl.get(), opcode_sra(mmu->read8(hl.get())));
			break;
		case 0x2F:			// SRA A
			a.set(opcode_sra(a.get()));
			break;
		case 0x30:			// SWAP B
			b.set(opcode_swap(b.get()));
			break;
		case 0x31:			// SWAP C
			c.set(opcode_swap(c.get()));
			break;
		case 0x32:			// SWAP D
			d.set(opcode_swap(d.get()));
			break;
		case 0x33:			// SWAP E
			e.set(opcode_swap(e.get()));
			break;
		case 0x34:			// SWAP H
			h.set(opcode_swap(h.get()));
			break;
		case 0x35:			// SWAP L
			l.set(opcode_swap(l.get()));
			break;
		case 0x36:			// SWAP HL
			mmu->write8(hl.get(), opcode_swap(mmu->read8(hl.get())));
			break;
		case 0x37:			// SWAP A
			a.set(opcode_swap(a.get()));
			break;
		case 0x38:			// SRL B
			b.set(opcode_srl(b.get()));
			break;
		case 0x39:			// SRL C
			c.set(opcode_srl(c.get()));
			break;
		case 0x3A:			// SRL D
			d.set(opcode_srl(d.get()));
			break;
		case 0x3B:			// SRL E
			e.set(opcode_srl(e.get()));
			break;
		case 0x3C:			// SRL H
			h.set(opcode_srl(h.get()));
			break;
		case 0x3D:			// SRL L
			l.set(opcode_srl(l.get()));
			break;
		case 0x3E:			// SRL [HL]
			mmu->write8(hl.get(), opcode_srl(mmu->read8(hl.get())));
			break;
		case 0x3F:			// SRL A
			a.set(opcode_srl(a.get()));
			break;
		case 0x40:			// BIT 0, B
			opcode_bit(0, b.get());
			break;
		case 0x41:			// BIT 0, C
			opcode_bit(0, c.get());
			break;
		case 0x42:			// BIT 0, D
			opcode_bit(0, d.get());
			break;
		case 0x43:			// BIT 0, E
			opcode_bit(0, e.get());
			break;
		case 0x44:			// BIT 0, H
			opcode_bit(0, h.get());
			break;
		case 0x45:			// BIT 0, L
			opcode_bit(0, l.get());
			break;
		case 0x46:			// BIT 0, HL
			opcode_bit(0, mmu->read8(hl.get()));
			break;
		case 0x47:			// BIT 0, A
			opcode_bit(0, a.get());
			break;
		case 0x48:			// BIT 1, B
			opcode_bit(1, b.get());
			break;
		case 0x49:			// BIT 1, C
			opcode_bit(1, c.get());
			break;
		case 0x4A:			// BIT 1, D
			opcode_bit(1, d.get());
			break;
		case 0x4B:			// BIT 1, E
			opcode_bit(1, e.get());
			break;
		case 0x4C:			// BIT 1, H
			opcode_bit(1, h.get());
			break;
		case 0x4D:			// BIT 1, L
			opcode_bit(1, l.get());
			break;
		case 0x4E:			// BIT 1, HL
			opcode_bit(1, mmu->read8(hl.get()));
			break;
		case 0x4F:			// BIT 1, A
			opcode_bit(1, a.get());
			break;
		case 0x50:			// BIT 2, B
			opcode_bit(2, b.get());
			break;
		case 0x51:			// BIT 2, C
			opcode_bit(2, c.get());
			break;
		case 0x52:			// BIT 2, D
			opcode_bit(2, d.get());
			break;
		case 0x53:			// BIT 2, E
			opcode_bit(2, e.get());
			break;
		case 0x54:			// BIT 2, H
			opcode_bit(2, h.get());
			break;
		case 0x55:			// BIT 2, L
			opcode_bit(2, l.get());
			break;
		case 0x56:			// BIT 2, HL
			opcode_bit(2, mmu->read8(hl.get()));
			break;
		case 0x57:			// BIT 2, A
			opcode_bit(2, a.get());
			break;
		case 0x58:			// BIT 3, B
			opcode_bit(3, b.get());
			break;
		case 0x59:			// BIT 3, C
			opcode_bit(3, c.get());
			break;
		case 0x5A:			// BIT 3, D
			opcode_bit(3, d.get());
			break;
		case 0x5B:			// BIT 3, E
			opcode_bit(3, e.get());
			break;
		case 0x5C:			// BIT 3, H
			opcode_bit(3, h.get());
			break;
		case 0x5D:			// BIT 3, L
			opcode_bit(3, l.get());
			break;
		case 0x5E:			// BIT 3, HL
			opcode_bit(3, mmu->read8(hl.get()));
			break;
		case 0x5F:			// BIT 3, A
			opcode_bit(3, a.get());
			break;
		case 0x60:			// BIT 4, B
			opcode_bit(4, b.get());
			break;
		case 0x61:			// BIT 4, C
			opcode_bit(4, c.get());
			break;
		case 0x62:			// BIT 4, D
			opcode_bit(4, d.get());
			break;
		case 0x63:			// BIT 4, E
			opcode_bit(4, e.get());
			break;
		case 0x64:			// BIT 4, H
			opcode_bit(4, h.get());
			break;
		case 0x65:			// BIT 4, L
			opcode_bit(4, l.get());
			break;
		case 0x66:			// BIT 4, HL
			opcode_bit(4, mmu->read8(hl.get()));
			break;
		case 0x67:			// BIT 4, A
			opcode_bit(4, a.get());
			break;
		case 0x68:			// BIT 5, B
			opcode_bit(5, b.get());
			break;
		case 0x69:			// BIT 5, C
			opcode_bit(5, c.get());
			break;
		case 0x6A:			// BIT 5, D
			opcode_bit(5, d.get());
			break;
		case 0x6B:			// BIT 5, E
			opcode_bit(5, e.get());
			break;
		case 0x6C:			// BIT 5, H
			opcode_bit(5, h.get());
			break;
		case 0x6D:			// BIT 5, L
			opcode_bit(5, l.get());
			break;
		case 0x6E:			// BIT 5, HL
			opcode_bit(5, mmu->read8(hl.get()));
			break;
		case 0x6F:			// BIT 5, A
			opcode_bit(5, a.get());
			break;
		case 0x70:			// BIT 6, B
			opcode_bit(6, b.get());
			break;
		case 0x71:			// BIT 6, C
			opcode_bit(6, c.get());
			break;
		case 0x72:			// BIT 6, D
			opcode_bit(6, d.get());
			break;
		case 0x73:			// BIT 6, E
			opcode_bit(6, e.get());
			break;
		case 0x74:			// BIT 6, H
			opcode_bit(6, h.get());
			break;
		case 0x75:			// BIT 6, L
			opcode_bit(6, l.get());
			break;
		case 0x76:			// BIT 6, HL
			opcode_bit(6, mmu->read8(hl.get()));
			break;
		case 0x77:			// BIT 6, A
			opcode_bit(6, a.get());
			break;
		case 0x78:			// BIT 7, B
			opcode_bit(7, b.get());
			break;
		case 0x79:			// BIT 7, C
			opcode_bit(7, c.get());
			break;
		case 0x7A:			// BIT 7, D
			opcode_bit(7, d.get());
			break;
		case 0x7B:			// BIT 7, E
			opcode_bit(7, e.get());
			break;
		case 0x7C:			// BIT 7, H
			opcode_bit(7, h.get());
			break;
		case 0x7D:			// BIT 7, L
			opcode_bit(7, l.get());
			break;
		case 0x7E:			// BIT 7, HL
			opcode_bit(7, mmu->read8(hl.get()));
			break;
		case 0x7F:			// BIT 7, A
			opcode_bit(7, a.get());
			break;
		case 0x80:			// RES 0, B
			opcode_res(0, b);
			break;
		case 0x81:			// RES 0, C
			opcode_res(0, c);
			break;
		case 0x82:			// RES 0, D
			opcode_res(0, d);
			break;
		case 0x83:			// RES 0, E
			opcode_res(0, e);
			break;
		case 0x84:			// RES 0, H
			opcode_res(0, h);
			break;
		case 0x85:			// RES 0, L
			opcode_res(0, l);
			break;
		case 0x86:			// RES 0, HL
			mmu->write8(hl.get(), opcode_res(0, mmu->read8(hl.get())));
			break;
		case 0x87:			// RES 0, A
			opcode_res(0, a);
			break;
		case 0x88:			// RES 1, B
			opcode_res(1, b);
			break;
		case 0x89:			// RES 1, C
			opcode_res(1, c);
			break;
		case 0x8A:			// RES 1, D
			opcode_res(1, d);
			break;
		case 0x8B:			// RES 1, E
			opcode_res(1, e);
			break;
		case 0x8C:			// RES 1, H
			opcode_res(1, h);
			break;
		case 0x8D:			// RES 1, L
			opcode_res(1, l);
			break;
		case 0x8E:			// RES 1, HL
			mmu->write8(hl.get(), opcode_res(1, mmu->read8(hl.get())));
			break;
		case 0x8F:			// RES 1, A
			opcode_res(1, a);
			break;
		case 0x90:			// RES 2, B
			opcode_res(2, b);
			break;
		case 0x91:			// RES 2, C
			opcode_res(2, c);
			break;
		case 0x92:			// RES 2, D
			opcode_res(2, d);
			break;
		case 0x93:			// RES 2, E
			opcode_res(2, e);
			break;
		case 0x94:			// RES 2, H
			opcode_res(2, h);
			break;
		case 0x95:			// RES 2, L
			opcode_res(2, l);
			break;
		case 0x96:			// RES 2, HL
			mmu->write8(hl.get(), opcode_res(2, mmu->read8(hl.get())));
			break;
		case 0x97:			// RES 2, A
			opcode_res(2, a);
			break;
		case 0x98:			// RES 3, B
			opcode_res(3, b);
			break;
		case 0x99:			// RES 3, C
			opcode_res(3, c);
			break;
		case 0x9A:			// RES 3, D
			opcode_res(3, d);
			break;
		case 0x9B:			// RES 3, E
			opcode_res(3, e);
			break;
		case 0x9C:			// RES 3, H
			opcode_res(3, h);
			break;
		case 0x9D:			// RES 3, L
			opcode_res(3, l);
			break;
		case 0x9E:			// RES 3, HL
			mmu->write8(hl.get(), opcode_res(3, mmu->read8(hl.get())));
			break;
		case 0x9F:			// RES 3, A
			opcode_res(3, a);
			break;
		case 0xA0:			// RES 4, B
			opcode_res(4, b);
			break;
		case 0xA1:			// RES 4, C
			opcode_res(4, c);
			break;
		case 0xA2:			// RES 4, D
			opcode_res(4, d);
			break;
		case 0xA3:			// RES 4, E
			opcode_res(4, e);
			break;
		case 0xA4:			// RES 4, H
			opcode_res(4, h);
			break;
		case 0xA5:			// RES 4, L
			opcode_res(4, l);
			break;
		case 0xA6:			// RES 4, HL
			mmu->write8(hl.get(), opcode_res(4, mmu->read8(hl.get())));
			break;
		case 0xA7:			// RES 4, A
			opcode_res(4, a);
			break;
		case 0xA8:			// RES 5, B
			opcode_res(5, b);
			break;
		case 0xA9:			// RES 5, C
			opcode_res(5, c);
			break;
		case 0xAA:			// RES 5, D
			opcode_res(5, d);
			break;
		case 0xAB:			// RES 5, E
			opcode_res(5, e);
			break;
		case 0xAC:			// RES 5, H
			opcode_res(5, h);
			break;
		case 0xAD:			// RES 5, L
			opcode_res(5, l);
			break;
		case 0xAE:			// RES 5, HL
			mmu->write8(hl.get(), opcode_res(5, mmu->read8(hl.get())));
			break;
		case 0xAF:			// RES 5, A
			opcode_res(5, a);
			break;
		case 0xB0:			// RES 6, B
			opcode_res(6, b);
			break;
		case 0xB1:			// RES 6, C
			opcode_res(6, c);
			break;
		case 0xB2:			// RES 6, D
			opcode_res(6, d);
			break;
		case 0xB3:			// RES 6, E
			opcode_res(6, e);
			break;
		case 0xB4:			// RES 6, H
			opcode_res(6, h);
			break;
		case 0xB5:			// RES 6, L
			opcode_res(6, l);
			break;
		case 0xB6:			// RES 6, HL
			mmu->write8(hl.get(), opcode_res(6, mmu->read8(hl.get())));
			break;
		case 0xB7:			// RES 6, A
			opcode_res(6, a);
			break;
		case 0xB8:			// RES 7, B
			opcode_res(7, b);
			break;
		case 0xB9:			// RES 7, C
			opcode_res(7, c);
			break;
		case 0xBA:			// RES 7, D
			opcode_res(7, d);
			break;
		case 0xBB:			// RES 7, E
			opcode_res(7, e);
			break;
		case 0xBC:			// RES 7, H
			opcode_res(7, h);
			break;
		case 0xBD:			// RES 7, L
			opcode_res(7, l);
			break;
		case 0xBE:			// RES 7, HL
			mmu->write8(hl.get(), opcode_res(7, mmu->read8(hl.get())));
			break;
		case 0xBF:			// RES 7, A
			opcode_res(7, a);
			break;
		case 0xC0:			// SET 0, B
			opcode_set(0, b);
			break;
		case 0xC1:			// SET 0, C
			opcode_set(0, c);
			break;
		case 0xC2:			// SET 0, D
			opcode_set(0, d);
			break;
		case 0xC3:			// SET 0, E
			opcode_set(0, e);
			break;
		case 0xC4:			// SET 0, H
			opcode_set(0, h);
			break;
		case 0xC5:			// SET 0, L
			opcode_set(0, l);
			break;
		case 0xC6:			// SET 0, HL
			mmu->write8(hl.get(), opcode_set(0, mmu->read8(hl.get())));
			break;
		case 0xC7:			// SET 0, A
			opcode_set(0, a);
			break;
		case 0xC8:			// SET 1, B
			opcode_set(1, b);
			break;
		case 0xC9:			// SET 1, C
			opcode_set(1, c);
			break;
		case 0xCA:			// SET 1, D
			opcode_set(1, d);
			break;
		case 0xCB:			// SET 1, E
			opcode_set(1, e);
			break;
		case 0xCC:			// SET 1, H
			opcode_set(1, h);
			break;
		case 0xCD:			// SET 1, L
			opcode_set(1, l);
			break;
		case 0xCE:			// SET 1, HL
			mmu->write8(hl.get(), opcode_set(1, mmu->read8(hl.get())));
			break;
		case 0xCF:			// SET 1, A
			opcode_set(1, a);
			break;
		case 0xD0:			// SET 2, B
			opcode_set(2, b);
			break;
		case 0xD1:			// SET 2, C
			opcode_set(2, c);
			break;
		case 0xD2:			// SET 2, D
			opcode_set(2, d);
			break;
		case 0xD3:			// SET 2, E
			opcode_set(2, e);
			break;
		case 0xD4:			// SET 2, H
			opcode_set(2, h);
			break;
		case 0xD5:			// SET 2, L
			opcode_set(2, l);
			break;
		case 0xD6:			// SET 2, HL
			mmu->write8(hl.get(), opcode_set(2, mmu->read8(hl.get())));
			break;
		case 0xD7:			// SET 2, A
			opcode_set(2, a);
			break;
		case 0xD8:			// SET 3, B
			opcode_set(3, b);
			break;
		case 0xD9:			// SET 3, C
			opcode_set(3, c);
			break;
		case 0xDA:			// SET 3, D
			opcode_set(3, d);
			break;
		case 0xDB:			// SET 3, E
			opcode_set(3, e);
			break;
		case 0xDC:			// SET 3, H
			opcode_set(3, h);
			break;
		case 0xDD:			// SET 3, L
			opcode_set(3, l);
			break;
		case 0xDE:			// SET 3, HL
			mmu->write8(hl.get(), opcode_set(3, mmu->read8(hl.get())));
			break;
		case 0xDF:			// SET 3, A
			opcode_set(3, a);
			break;
		case 0xE0:			// SET 4, B
			opcode_set(4, b);
			break;
		case 0xE1:			// SET 4, C
			opcode_set(4, c);
			break;
		case 0xE2:			// SET 4, D
			opcode_set(4, d);
			break;
		case 0xE3:			// SET 4, E
			opcode_set(4, e);
			break;
		case 0xE4:			// SET 4, H
			opcode_set(4, h);
			break;
		case 0xE5:			// SET 4, L
			opcode_set(4, l);
			break;
		case 0xE6:			// SET 4, HL
			mmu->write8(hl.get(), opcode_set(4, mmu->read8(hl.get())));
			break;
		case 0xE7:			// SET 4, A
			opcode_set(4, a);
			break;
		case 0xE8:			// SET 5, B
			opcode_set(5, b);
			break;
		case 0xE9:			// SET 5, C
			opcode_set(5, c);
			break;
		case 0xEA:			// SET 5, D
			opcode_set(5, d);
			break;
		case 0xEB:			// SET 5, E
			opcode_set(5, e);
			break;
		case 0xEC:			// SET 5, H
			opcode_set(5, h);
			break;
		case 0xED:			// SET 5, L
			opcode_set(5, l);
			break;
		case 0xEE:			// SET 5, HL
			mmu->write8(hl.get(), opcode_set(5, mmu->read8(hl.get())));
			break;
		case 0xEF:			// SET 5, A
			opcode_set(5, a);
			break;
		case 0xF0:			// SET 6, B
			opcode_set(6, b);
			break;
		case 0xF1:			// SET 6, C
			opcode_set(6, c);
			break;
		case 0xF2:			// SET 6, D
			opcode_set(6, d);
			break;
		case 0xF3:			// SET 6, E
			opcode_set(6, e);
			break;
		case 0xF4:			// SET 6, H
			opcode_set(6, h);
			break;
		case 0xF5:			// SET 6, L
			opcode_set(6, l);
			break;
		case 0xF6:			// SET 6, HL
			mmu->write8(hl.get(), opcode_set(6, mmu->read8(hl.get())));
			break;
		case 0xF7:			// SET 6, A
			opcode_set(6, a);
			break;
		case 0xF8:			// SET 7, B
			opcode_set(7, b);
			break;
		case 0xF9:			// SET 7, C
			opcode_set(7, c);
			break;
		case 0xFA:			// SET 7, D
			opcode_set(7, d);
			break;
		case 0xFB:			// SET 7, E
			opcode_set(7, e);
			break;
		case 0xFC:			// SET 7, H
			opcode_set(7, h);
			break;
		case 0xFD:			// SET 7, L
			opcode_set(7, l);
			break;
		case 0xFE:			// SET 7, HL
			mmu->write8(hl.get(), opcode_set(7, mmu->read8(hl.get())));
			break;
		case 0xFF:			// SET 7, A
			opcode_set(7, a);
			break;

		}
	}
}