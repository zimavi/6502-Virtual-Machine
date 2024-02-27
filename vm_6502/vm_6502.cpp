#include <iostream>
#include <cassert>

using byte = unsigned char;
using word = unsigned short;

using u32 = unsigned int;

/// 0x0000 - 0x00FF: Stack + ZeroPage
/// 0x0100 - 0xF0FE: Free to use	(not hardcoded)
/// 0xF0FF - 0xFDFA: ISR Handlering	(not hardcoded)
/// 0xFDFB - 0xFFFC: ISR table 
/// 0xFFFC - 0xFFFF: Startup code
struct Memory
{
	static constexpr u32 MAX_MEM = 1024 * 64;
	byte m_Data[MAX_MEM];

	void Init()
	{
		for (u32 i = 0; i < MAX_MEM; i++)
		{
			m_Data[i] = 0;
		}
	}

	byte operator[](u32 address) const
	{
		assert(address < MAX_MEM);
		return m_Data[address];
	}

	byte& operator[](u32 address)
	{
		assert(address < MAX_MEM);
		return m_Data[address];
	}
};

struct CPU
{

	word PC;
	word SP;
	
	byte A, X, Y;

	byte C : 1;
	byte Z : 1;
	byte I : 1;
	byte D : 1;
	byte B : 1;
	byte V : 1;
	byte N : 1;

	void Reset(Memory& ram)
	{
		PC = 0xFFFC;
		SP = 0x00FF;
		C = Z = I = D = B = V = N = 0;
		A = X = Y = 0;
		ram.Init();
	}
	byte FetchByte(u32& cycles, Memory& ram)
	{
		byte data = ram[PC];
		PC++;
		cycles--;
		return data;
	}
	word FetchWord(u32& cycles, Memory& ram)
	{
		byte high = FetchByte(cycles, ram);
		byte low = FetchByte(cycles, ram);

		word data = (high << 8) | low;
		return data;
	}
	byte ReadByte(u32& cycles, Memory& ram, u32 address)
	{
		byte data = ram[address];
		cycles--;
		return data;
	}
	word ReadWord(u32& cycles, Memory& ram, u32 address)
	{
		byte high = ReadByte(cycles, ram, address);
		byte low = ReadByte(cycles, ram, address + 1);

		word data = (high << 8) | low;
		return data;
	}
	void WriteByte(u32& cycles, Memory& ram, u32 address, byte data)
	{
		ram[address] = data;
		cycles--;
	}
	void WriteWord(u32& cycles, Memory& ram, u32 address, word data)
	{
		byte wordh = (data >> 8) & 0x00FF;
		byte wordl = data & 0x00FF;

		WriteByte(cycles, ram, address, wordh);
		WriteByte(cycles, ram, address + 1, wordl);
	}
	void PushProgramState(u32& cycles, Memory& ram)
	{
		word pState = C & 0x0F | (Z & 0x0F) << 1 | (I & 0x0F) << 2 | (D & 0x0F) << 3 | (B & 0x0F) << 4 | (V & 0x0F) << 5 | (N & 0x0F) << 6;
		SP -= 2;
		WriteWord(cycles, ram, SP, pState);
	}
	void PullProgramState(u32& cycles, Memory& ram)
	{
		word pState = ReadWord(cycles, ram, SP);

		C = pState & 0x000F;
		Z = (pState >> 1) & 0x000F;
		I = (pState >> 2) & 0x000F;
		D = (pState >> 3) & 0x000F;
		B = (pState >> 4) & 0x000F;
		V = (pState >> 5) & 0x000F;
		N = (pState >> 6) & 0x000F;
		SP += 2;
	}

	static constexpr byte INS_ADC_IM	= 0x69; // implemented
	static constexpr byte INS_ADC_ZP	= 0x65; // implemented
	static constexpr byte INS_ADC_ZPX	= 0x75; // implemented
	static constexpr byte INS_ADC_ABS	= 0x6D; // implemented
	static constexpr byte INS_ADC_ABSX	= 0x7D; // implemented
	static constexpr byte INS_ADC_ABSY	= 0x79; // implemented

	static constexpr byte INS_CLC_IM	= 0x18; // implemented
	static constexpr byte INS_CLD_IM	= 0xD8; // implemented
	static constexpr byte INS_CLV_IM	= 0xB8; // implemented

	static constexpr byte INS_EOR_IM	= 0x49; // implemented
	static constexpr byte INS_EOR_ZP	= 0x45; // implemented
	static constexpr byte INS_EOR_ZPX	= 0x55; // implemented
	static constexpr byte INS_EOR_ABS	= 0x4D; // implemented
	static constexpr byte INS_EOR_ABSX	= 0x5D; // implemented
	static constexpr byte INS_EOR_ABSY	= 0x59; // implemented

	static constexpr byte INS_AND_IM	= 0x29; // implemented
	static constexpr byte INS_AND_ZP	= 0x25; // implemented
	static constexpr byte INS_AND_ZPX	= 0x35; // implemented
	static constexpr byte INS_AND_ABS	= 0x2D; // implemented
	static constexpr byte INS_AND_ABSX	= 0x3D; // implemented
	static constexpr byte INS_AND_ABSY	= 0x39; // implemented

	static constexpr byte INS_ORA_IM	= 0x09; // implemented
	static constexpr byte INS_ORA_ZP	= 0x05; // implemented
	static constexpr byte INS_ORA_ZPX	= 0x15; // implemented
	static constexpr byte INS_ORA_ABS	= 0x0D; // implemented
	static constexpr byte INS_ORA_ABSX	= 0x1D; // implemented
	static constexpr byte INS_ORA_ABSY	= 0x19; // implemented

	static constexpr byte INS_BCC_RL	= 0x90; // implemented
	static constexpr byte INS_BCS_RL	= 0xB0; // implemented
	static constexpr byte INS_BEQ_RL	= 0xF0; // implemented
	static constexpr byte INS_BMI_RL	= 0x30; // implemented
	static constexpr byte INS_BNE_RL	= 0xD0; // implemented
	static constexpr byte INS_BPL_RL	= 0x10; // implemented
	static constexpr byte INS_BVC_RL	= 0x50; // implemented
	static constexpr byte INS_BVS_RL	= 0x70; // implemented

	static constexpr byte INS_LDA_IM	= 0xA9; // implemented
	static constexpr byte INS_LDA_ZP	= 0xA5; // implemented
	static constexpr byte INS_LDA_ZPX	= 0xB5; // implemented	

	static constexpr byte INS_LDY_IM	= 0xA0; // implemented
	static constexpr byte INS_LDY_ZP	= 0xA4; // implemented
	static constexpr byte INS_LDY_ZPX	= 0xB4; // implemented

	static constexpr byte INS_LDX_IM	= 0xA2; // implemented
	static constexpr byte INS_LDX_ZP	= 0xA6; // implemented
	static constexpr byte INS_LDX_ZPY	= 0xB6; // implemented

	static constexpr byte INS_JMP_ABS	= 0x4C; // implemented
	static constexpr byte INS_JSR_ABS	= 0x20; // implemented

	static constexpr byte INS_RTS_ABS	= 0x60; // implemented

	static constexpr byte INS_CMP_IM	= 0xC9; // implemented
	static constexpr byte INS_CMP_ZP	= 0xC5;	// implemented
	static constexpr byte INS_CMP_ZPX	= 0xD5; // implemented
	static constexpr byte INS_CMP_ABS	= 0xCD; // implemented
	static constexpr byte INS_CMP_ABSX	= 0xDD; // implemented
	static constexpr byte INS_CMP_ABSY	= 0xD9; // implemented

	static constexpr byte INS_CPX_IM	= 0xE0; // implemented
	static constexpr byte INS_CPX_ZP	= 0xE4; // implemented
	static constexpr byte INS_CPX_ABS	= 0xEC; // implemented

	static constexpr byte INS_CPY_IM	= 0xC0; // implemented
	static constexpr byte INS_CPY_ZP	= 0xC4; // implemented
	static constexpr byte INS_CPY_ABS	= 0xCC; // implemented

	static constexpr byte INS_DEC_ZP	= 0xC6; // implemented
	static constexpr byte INS_DEC_ZPX	= 0xD6; // implemented
	static constexpr byte INS_DEC_ABS	= 0xCE; // implemented
	static constexpr byte INS_DEC_ABSX	= 0xDE; // implemented

	static constexpr byte INS_DEX_IM	= 0xCA; // implemented

	static constexpr byte INS_DEY_IM	= 0x88; // implemented

	static constexpr byte INS_INC_ZP	= 0xE6; // implemented
	static constexpr byte INS_INC_ZPX	= 0xF6; // implemented
	static constexpr byte INS_INC_ABS	= 0xEE; // implemented
	static constexpr byte INS_INC_ABSX	= 0xFE; // implemented

	static constexpr byte INS_INX_IM	= 0xE8; // implemented

	static constexpr byte INS_INY_IM	= 0xC8; // implemented

	static constexpr byte INS_PHA_IM	= 0x48; // implemented
	static constexpr byte INS_PHP_IM	= 0x08; // implemented

	static constexpr byte INS_PLA_IM	= 0x68; // implemented
	static constexpr byte INS_PLP_IM	= 0x28; // implemented

	static constexpr byte INS_SDA_ZP	= 0x85; // implemented
	static constexpr byte INS_SDA_ZPX	= 0x95; // implemented
	static constexpr byte INS_SDA_ABS	= 0x8D; // implemented
	static constexpr byte INS_SDA_ABSX	= 0x9D; // implemented
	static constexpr byte INS_SDA_ABSY	= 0x99; // implemented

	static constexpr byte INS_SDX_ZP	= 0x86; // implemented
	static constexpr byte INS_SDX_ZPY	= 0x96; // implemented
	static constexpr byte INS_SDX_ABS	= 0x8E; // implemented

	static constexpr byte INS_TAX_IM	= 0xAA;
	static constexpr byte INS_TAY_IM	= 0xA8;
	static constexpr byte INS_TSX_IM	= 0xA8;

	static constexpr byte INS_CLI_IM	= 0x58; // implemented
	static constexpr byte INS_SEI_IM	= 0x78; // implemented
	static constexpr byte INS_NOP_IM	= 0xEA; // implemented
	static constexpr byte INS_BRK_IM	= 0x00; // implemented
	static constexpr byte INS_RTI_IM	= 0x40; // implemented


	void Execute(u32 cycles, Memory& ram)
	{
		while (cycles > 0)
		{
			byte ins = FetchByte(cycles, ram);
			switch (ins)
			{
				case INS_ADC_IM:
				{
					byte data = FetchByte(cycles, ram);
					byte oldA = A;
					A += data + C;
					C = (A < oldA);
					Z = (A == 0);
					V = 0;						// don't know how to implement
					N = (A & 0b10000000) > 0;
				} break;

				case INS_ADC_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					byte oldA = A;
					A += data + C;
					C = (A < oldA);
					Z = (A == 0);
					V = 0;
					N = (A & 0b10000000) > 0;
				} break;

				case INS_ADC_ZPX:
				{
					word address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					byte oldA = A;
					A += data + C;
					C = (A < oldA);
					Z = (A == 0);
					V = 0;
					N = (A & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_ADC_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					byte oldA = A;
					A += data + C;
					C = (A < oldA);
					Z = (A == 0);
					V = 0;
					N = (A & 0b10000000) > 0;
				} break;

				case INS_ADC_ABSX:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					byte oldA = A;
					A += data + C;
					C = (A < oldA);
					Z = (A == 0);
					V = 0;
					N = (A & 0b10000000) > 0;
				} break;

				case INS_ADC_ABSY:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + Y);
					byte oldA = A;
					A += data + C;
					C = (A < oldA);
					Z = (A == 0);
					V = 0;
					N = (A & 0b10000000) > 0;
				} break;

				case INS_CLC_IM:
				{
					C = 0;
					cycles--;
				} break;

				case INS_CLD_IM:
				{
					D = 0;
					cycles--;
				} break;

				case INS_CLV_IM:
				{
					V = 0;
					cycles--;
				} break;

				case INS_EOR_IM:
				{
					byte data = FetchByte(cycles, ram);
					A = A ^ data;
					Z = (A = 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_EOR_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					A = A ^ data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_EOR_ZPX:
				{
					word address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					A = A ^ data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_EOR_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					A = A ^ data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_EOR_ABSX:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					A = A ^ data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_EOR_ABSY:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + Y);
					A = A ^ data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_ORA_IM:
				{
					byte data = FetchByte(cycles, ram);
					A = A | data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_ORA_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					A = A | data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_ORA_ZPX:
				{
					word address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					A = A | data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_ORA_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					A = A | data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_ORA_ABSX:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					A = A | data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_ORA_ABSY:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + Y);
					A = A | data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_BCC_RL:
				{
					cycles --;
					byte offset = FetchByte(cycles, ram);

					if (!C)
					{
						PC += offset;
						cycles--;
					}

				} break;

				case INS_BCS_RL:
				{
					cycles --;
					byte offset = FetchByte(cycles, ram);

					if (C)
					{
						PC += offset;
						cycles--;
					}
				} break;

				case INS_BEQ_RL:
				{
					cycles--;
					byte offset = FetchByte(cycles, ram);

					if (Z)
					{
						PC += offset;
						cycles--;
					}
				} break;

				case INS_BNE_RL:
				{
					cycles--;
					byte offset = FetchByte(cycles, ram);

					if (!Z)
					{
						PC += offset;
						cycles--;
					}
				} break;

				case INS_BPL_RL:
				{
					cycles--;
					byte offset = FetchByte(cycles, ram);

					if (!N)
					{
						PC += offset;
						cycles--;
					}
				} break;

				case INS_BVC_RL:
				{
					cycles--;
					byte offset = FetchByte(cycles, ram);

					if (!V)
					{
						PC += offset;
						cycles--;
					}
				} break;

				case INS_BVS_RL:
				{
					cycles--;
					byte offset = FetchByte(cycles, ram);

					if (V)
					{
						PC += offset;
						cycles--;
					}
				} break;

				case INS_AND_IM:
				{
					byte data = FetchByte(cycles, ram);
					A = A & data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_AND_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					A = A & data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_AND_ZPX:
				{
					word address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					A = A & data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_AND_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					A = A & data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_AND_ABSX:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					A = A & data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_AND_ABSY:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + Y);
					A = A & data;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_LDA_IM:
				{
					byte value = FetchByte(cycles, ram);
					A = value;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_LDA_ZP:
				{
					byte zero_page_address = FetchByte(cycles, ram);
					A = ReadByte(cycles, ram, zero_page_address);
				} break;

				case INS_LDA_ZPX:
				{
					word zero_page_address = FetchByte(cycles, ram);
					zero_page_address += X;
					cycles--;
					A = ReadByte(cycles, ram, zero_page_address);
				} break;
				
				case INS_LDX_IM:
				{
					byte value = FetchByte(cycles, ram);
					X = value;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_LDX_ZP:
				{
					byte zero_page_address = FetchByte(cycles, ram);
					X = ReadByte(cycles, ram, zero_page_address);
				} break;

				case INS_LDX_ZPY:
				{
					byte zero_page_address = FetchByte(cycles, ram);
					zero_page_address += Y;
					cycles--;
					X = ReadByte(cycles, ram, zero_page_address);
				} break;

				case INS_LDY_IM:
				{
					byte value = FetchByte(cycles, ram);
					Y = value;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
				} break;

				case INS_LDY_ZP:
				{
					byte zero_page_address = FetchByte(cycles, ram);
					Y = ReadByte(cycles, ram, zero_page_address);
				} break;

				case INS_LDY_ZPX:
				{
					word zero_page_address = FetchByte(cycles, ram);
					zero_page_address += X;
					cycles--;
					Y = ReadByte(cycles, ram, zero_page_address);
				} break;

				case INS_JMP_ABS:
				{
					word address = FetchWord(cycles, ram);
					PC = address;
				} break;

				case INS_JSR_ABS:
				{
					SP += 2;
					WriteWord(cycles, ram, SP, PC - 1);

					word sub_rutine = FetchWord(cycles, ram);
					PC = sub_rutine;
					cycles--;
				} break;

				case INS_RTS_ABS:
				{
					word return_address = ReadWord(cycles, ram, SP);
					SP += 2;

					PC = return_address;
					cycles -= 3;
				} break;

				case INS_CMP_IM:
				{
					byte data = FetchByte(cycles, ram);
					byte result = data - A;
					C = (A >= data);
					Z = (A == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CMP_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					byte result = data - A;
					C = (A >= data);
					Z = (A == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CMP_ZPX:
				{
					word address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					byte result = data - A;
					C = (A >= data);
					Z = (A == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CMP_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					byte result = data - A;
					C = (A >= data);
					Z = (A == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CMP_ABSX:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					byte result = data - A;
					C = (A >= data);
					Z = (A == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CMP_ABSY:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + Y);
					byte result = data - A;
					C = (A >= data);
					Z = (A == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CPX_IM:
				{
					byte data = FetchByte(cycles, ram);
					byte result = data - X;
					C = (X >= data);
					Z = (X == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CPX_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					byte result = data - X;
					C = (X >= data);
					Z = (X == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CPX_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					byte result = data - X;
					C = (X >= data);
					Z = (X == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CPY_IM:
				{
					byte data = FetchByte(cycles, ram);
					byte result = data - Y;
					C = (Y >= data);
					Z = (Y == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CPY_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					byte result = data - Y;
					C = (Y >= data);
					Z = (Y == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_CPY_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					byte result = data - Y;
					C = (Y >= data);
					Z = (Y == data);
					N = (result & 0b10000000) > 0;
				} break;

				case INS_DEC_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);

					data--;
					Z = (data == 0);
					N = (data & 0b10000000) > 0;
					WriteByte(cycles, ram, address, data);
					cycles--;
				} break;

				case INS_DEC_ZPX:
				{
					word address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);

					data--;
					Z = (data == 0);
					N = (data & 0b10000000) > 0;
					WriteByte(cycles, ram, address + X, data);
					cycles -= 2;
				} break;

				case INS_DEC_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);

					data--;
					Z = (data == 0);
					N = (data & 0b10000000) > 0;
					WriteByte(cycles, ram, address, data);
					cycles--;
				} break;

				case INS_DEC_ABSX:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);

					data--;
					Z = (data == 0);
					N = (data & 0b10000000) > 0;
					WriteByte(cycles, ram, address, data);
					cycles -= 2;
				} break;

				case INS_DEX_IM:
				{
					X--;
					Z = (X == 0);
					N = (X & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_DEY_IM:
				{
					Y--;
					Z = (Y == 0);
					N = (Y & 0b10000000) > 0;
					cycles--;
				} break;

				case INS_INC_ZP:
				{
					byte address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					data++;
					Z = (data == 0);
					N = (data & 0b10000000) > 0;
					WriteByte(cycles, ram, address, data);
					cycles--;
				} break;

				case INS_INC_ZPX:
				{
					word address = FetchByte(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					data++;
					Z = (data == 0);
					N = (data & 0b10000000) > 0;
					WriteByte(cycles, ram, address + X, data);
					cycles -= 2;
				} break;

				case INS_INC_ABS:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address);
					data++;
					Z = (data == 0);
					N = (data & 0b10000000) > 0;
					WriteByte(cycles, ram, address, data);
					cycles--;
				} break;

				case INS_INC_ABSX:
				{
					word address = FetchWord(cycles, ram);
					byte data = ReadByte(cycles, ram, address + X);
					data++;
					Z = (data == 0);
					N = (data & 0b10000000) > 0;
					WriteByte(cycles, ram, address + X, data);
				} break;

				case INS_INX_IM:
				{
					X++;
					cycles--;
				} break;

				case INS_INY_IM:
				{
					Y++;
					cycles--;
				} break;

				case INS_PHA_IM:
				{
					SP -= 2;
					WriteWord(cycles, ram, SP, A);
				} break;

				case INS_PHP_IM:
				{ /* order: C Z I D B V N */ 
					PushProgramState(cycles, ram);
				} break;

				case INS_PLA_IM:
				{
					word value = ReadWord(cycles, ram, SP);
					A = value;
					Z = (A == 0);
					N = (A & 0b10000000) > 0;
					SP += 2;
				} break;

				case INS_PLP_IM:
				{ /* order: C Z I D B V N */ 
					PullProgramState(cycles, ram);
				} break;

				case INS_SDA_ZP:
				{
					byte address = FetchByte(cycles, ram);
					WriteByte(cycles, ram, address, A);
				} break;

				case INS_SDA_ZPX:
				{
					word address = FetchByte(cycles, ram);
					WriteByte(cycles, ram, address + X, A);
					cycles--;
				} break;

				case INS_SDA_ABS:
				{
					word address = FetchWord(cycles, ram);
					WriteByte(cycles, ram, address, A);
					if (address == 0xFFFF)
					{
						std::cout << ram[0xFFFF];
					}
				} break;

				case INS_SDA_ABSX:
				{
					word address = FetchWord(cycles, ram);
					WriteByte(cycles, ram, address + X, A);
					cycles--;
					if (address + X == 0xFFFF)
					{
						std::cout << ram[0xFFFF];
					}
				} break;

				case INS_SDA_ABSY:
				{
					word address = FetchWord(cycles, ram);
					WriteByte(cycles, ram, address + Y, A);
					cycles--;
					if (address + Y == 0xFFFF)
					{
						std::cout << ram[0xFFFF];
					}
				} break;

				case INS_SDX_ZP:
				{
					byte address = FetchByte(cycles, ram);
					WriteByte(cycles, ram, address, X);
				} break;

				case INS_SDX_ZPY:
				{
					word address = FetchByte(cycles, ram);
					WriteByte(cycles, ram, address + Y, X);
					cycles--;
				} break;

				case INS_SDX_ABS:
				{
					word address = FetchWord(cycles, ram);
					WriteByte(cycles, ram, address, X);
					if (address == 0xFFFF)
					{
						std::cout << ram[0xFFFF];
					}
				} break;

				case INS_CLI_IM:
				{
					I = 0;
					cycles--;
				} break; 
				
				case INS_SEI_IM:
				{
					I = 1;
					cycles--;
				} break;

				case INS_NOP_IM:
				{
					cycles--;
				} break;

				case INS_BRK_IM:
				{
					SP -= 2;
					WriteWord(cycles, ram, SP, PC - 1);
					PushProgramState(cycles, ram);
					B = 1;

					// 0xFDFB address of ISR table (up to 256 as it ends before 0xFFFC execution address)
					word ISRHandlerAddress = ReadWord(cycles, ram, 0xFDFC + ((word)A * 2)); 
					PC = ISRHandlerAddress;
				} break;

				case INS_RTI_IM:
				{
					PullProgramState(cycles, ram);
					word callerAddress = ReadWord(cycles, ram, SP);
					PC = callerAddress + 1;
					SP += 2;
					cycles--;
				} break;

				default:
				{
					SP -= 2;
					WriteWord(cycles, ram, SP, PC);
					PushProgramState(cycles, ram);
					A = 0x06;						// Invlid Opcode exception
					X = ins;
					word ISRHandlerAddress = ReadWord(cycles, ram, 0xFDFC + (5 * 2));
					PC = ISRHandlerAddress;
				} break;
			}
		}
	}
};

/// 0x0000 - 0x00FF: Stack + ZeroPage
/// 0x0100 - 0xF0FF: Free to use	(not hardcoded)	\
/// 0xF100 - 0xFDFA: ISR Handlering	(not hardcoded)	 \ Basically both free to use
/// 0xFDFC - 0xFFFB: ISR table 
/// 0xFFFC - 0xFFFE: Startup code
/// 0xFFFF		   : Output char
int main()
{
	Memory ram;
	CPU cpu6502;
	cpu6502.Reset(ram);

	/* DATA SEGMENT */

	ram[0x0000] = 0;

	/* ISR HANDLERS */
	
	// interrupt 0 (ISR) 
	// Sets A to 0x69
	ram[0xF100] = CPU::INS_LDA_IM;
	ram[0xF101] = 0x69;
	ram[0xF102] = CPU::INS_RTI_IM;

	// interrupt 1 (ISR)
	// Prints char
	ram[0xF103] = CPU::INS_SDX_ABS;
	ram[0xF104] = 0xFF;
	ram[0xF105] = 0xFF;
	ram[0xF106] = CPU::INS_RTI_IM;

	// interrupt 255 (ISR)
	// Halts CPU (by jumping)
	ram[0xF107] = CPU::INS_JMP_ABS;
	ram[0xF108] = 0xF1;
	ram[0xF109] = 0x03;
	ram[0xF10A] = CPU::INS_RTI_IM; // just in case;

	// interrupt 5 (ISR)
	// prints invalid opcode
	//
	// push message to the stack
	ram[0xF10B] = CPU::INS_PHA_IM;
	ram[0xF10C] = CPU::INS_LDA_IM;
	ram[0xF10D] = 'I';
	ram[0xF10E] = CPU::INS_PHA_IM;
	ram[0xF10F] = CPU::INS_LDA_IM;
	ram[0xF110] = 'n';
	ram[0xF111] = CPU::INS_PHA_IM;
	ram[0xF112] = CPU::INS_LDA_IM;
	ram[0xF113] = 'v';
	ram[0xF114] = CPU::INS_PHA_IM;
	ram[0xF115] = CPU::INS_LDA_IM;
	ram[0xF116] = 'a';
	ram[0xF117] = CPU::INS_PHA_IM;
	ram[0xF118] = CPU::INS_LDA_IM;
	ram[0xF119] = 'l';
	ram[0xF11A] = CPU::INS_PHA_IM;
	ram[0xF11B] = CPU::INS_LDA_IM;
	ram[0xF11C] = 'i';
	ram[0xF11D] = CPU::INS_PHA_IM;
	ram[0xF11E] = CPU::INS_LDA_IM;
	ram[0xF11F] = 'd';
	ram[0xF120] = CPU::INS_PHA_IM;
	ram[0xF121] = CPU::INS_LDA_IM;
	ram[0xF122] = ' ';
	ram[0xF123] = CPU::INS_PHA_IM;
	ram[0xF124] = CPU::INS_LDA_IM;
	ram[0xF125] = 'o';
	ram[0xF126] = CPU::INS_PHA_IM;
	ram[0xF127] = CPU::INS_LDA_IM;
	ram[0xF128] = 'p';
	ram[0xF129] = CPU::INS_PHA_IM;
	ram[0xF12A] = CPU::INS_LDA_IM;
	ram[0xF12B] = 'c';
	ram[0xF12C] = CPU::INS_PHA_IM;
	ram[0xF12D] = CPU::INS_LDA_IM;
	ram[0xF12E] = 'o';
	ram[0xF12F] = CPU::INS_PHA_IM;
	ram[0xF130] = CPU::INS_LDA_IM;
	ram[0xF131] = 'd';
	ram[0xF132] = CPU::INS_PHA_IM;
	ram[0xF133] = CPU::INS_LDA_IM;
	ram[0xF134] = 'e';
	ram[0xF135] = CPU::INS_PHA_IM;
	ram[0xF136] = CPU::INS_LDA_IM;
	ram[0xF137] = '\n';
	ram[0xF138] = CPU::INS_PHA_IM;
	// print message 13
	ram[0xF139] = CPU::INS_INY_IM;
	ram[0xF13A] = CPU::INS_PLA_IM;
	ram[0xF13B] = CPU::INS_SDA_ABS;
	ram[0xF13C] = 0xFF;
	ram[0xF13D] = 0xFF;
	ram[0xF13E] = CPU::INS_CPY_IM;
	ram[0xF13F] = 15;
	ram[0xF140] = CPU::INS_BEQ_RL;
	ram[0xF141] = 4;
	ram[0xF142] = CPU::INS_JMP_ABS;
	ram[0xF143] = 0xF1;
	ram[0xF144] = 0x39;
	ram[0xF145] = CPU::INS_RTI_IM;

	/* ISR TABLE */

	// ISR = 0
	ram[0xFDFC] = 0xF1;
	ram[0xFDFD] = 0x00;

	// ISR = 1
	ram[0xFDFE] = 0xF1;
	ram[0xFDFF] = 0x03;

	// ISR = 5 (invalid opcode)
	ram[0xFE06] = 0xF1;
	ram[0xFE07] = 0x0B;

	// ISR = 256
	ram[0xFFFA] = 0xF1;
	ram[0xFFFB] = 0x07;

	/* STARTUP */
	// Startup jump to address 0x0060
	ram[0xFFFC] = CPU::INS_JMP_ABS;
	ram[0xFFFD] = 0x01;
	ram[0xFFFE] = 0x00;

	/* PROGRAM GOES HERE */
	ram[0x0100] = CPU::INS_LDA_IM;
	ram[0x0101] = 'H';
	ram[0x0102] = CPU::INS_SDA_ZP;
	ram[0x0103] = 0x00;
	ram[0x0104] = CPU::INS_LDA_IM;
	ram[0x0105] = 'e';
	ram[0x0106] = CPU::INS_SDA_ZP;
	ram[0x0107] = 0x01;
	ram[0x0108] = CPU::INS_LDA_IM;
	ram[0x0109] = 'l';
	ram[0x010A] = CPU::INS_SDA_ZP;
	ram[0x010B] = 0x02;
	ram[0x010C] = CPU::INS_LDA_IM;
	ram[0x010D] = 'l';
	ram[0x010E] = CPU::INS_SDA_ZP;
	ram[0x010F] = 0x03;
	ram[0x0110] = CPU::INS_LDA_IM;
	ram[0x0111] = 'o';
	ram[0x0112] = CPU::INS_SDA_ZP;
	ram[0x0113] = 0x04;
	ram[0x0114] = CPU::INS_LDA_IM;
	ram[0x0115] = ' ';
	ram[0x0116] = CPU::INS_SDA_ZP;
	ram[0x0117] = 0x05;
	ram[0x0118] = CPU::INS_LDA_IM;
	ram[0x0119] = 'W';
	ram[0x011A] = CPU::INS_SDA_ZP;
	ram[0x011B] = 0x06;
	ram[0x011C] = CPU::INS_LDA_IM;
	ram[0x011D] = 'o';
	ram[0x011E] = CPU::INS_SDA_ZP;
	ram[0x011F] = 0x07;
	ram[0x0120] = CPU::INS_LDA_IM;
	ram[0x0121] = 'r';
	ram[0x0122] = CPU::INS_SDA_ZP;
	ram[0x0123] = 0x08;
	ram[0x0124] = CPU::INS_LDA_IM;
	ram[0x0125] = 'l';
	ram[0x0126] = CPU::INS_SDA_ZP;
	ram[0x0127] = 0x09;
	ram[0x0128] = CPU::INS_LDA_IM;
	ram[0x0129] = 'd';
	ram[0x012A] = CPU::INS_SDA_ZP;
	ram[0x012B] = 0x0A;
	ram[0x012C] = CPU::INS_LDA_IM;
	ram[0x012D] = '!';
	ram[0x012E] = CPU::INS_SDA_ZP;
	ram[0x012F] = 0x0B;
	ram[0x0130] = CPU::INS_LDA_IM;
	ram[0x0131] = '\n';
	ram[0x0132] = CPU::INS_SDA_ZP;

	ram[0x0133] = 0x0C;
	ram[0x0134] = CPU::INS_LDA_IM;	// load interrupt number to A
	ram[0x0135] = 0x01;				// interrupt 1 (printing char in 0xFFFF)
	ram[0x0136] = CPU::INS_INY_IM;	// increment Y (string loop counter)
	ram[0x0137] = CPU::INS_LDX_ZPY; // load sysmbol from ZP:Y (zp -> Y offset) to X
	ram[0x0138] = 0x02;				// points to ZP		 ^
	ram[0x0139] = CPU::INS_BRK_IM;	// call interrupt
	ram[0x013A] = CPU::INS_CPY_IM;	// compare Y to value
	ram[0x013B] = 0x0D;				// value		^
	ram[0x013C] = CPU::INS_BEQ_RL;	// jump if equal
	ram[0x013D] = 0x03;				// offset to jump to
	ram[0x013E] = CPU::INS_JMP_ABS;	// if previous doesn't executes, jump to the beging;
	ram[0x013F] = 0x01;				// both address
	ram[0x0140] = 0x02;
	ram[0x0141] = CPU::INS_JMP_ABS;	// will jump to here (loop uses as halt)
	ram[0x0142] = 0x01;
	ram[0x0143] = 0x0D;

	cpu6502.Execute(1, ram);
	return 0;
}