#include <iostream>

#include "CPU.h"
#include "NES.h"

CPU::CPU()
{
	ClearRegisters();
}

CPU::~CPU()
{

}

void CPU::Initialize(NES* console)
{
	m_NES = console;

	ClearRegisters();
	BuildOpCodeLookup();
}

// http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf
// http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
// https://www.masswerk.at/6502/6502_instruction_set.html
// https://www.nesdev.org/wiki/Status_flags
// https://en.wikipedia.org/wiki/MOS_Technology_6502#Bugs_and_quirks
void CPU::BuildOpCodeLookup()
{
	for (int i = 0; i < 256; i++)
	{
		// Unused opcodes. Is this actually zero clock cycles? My guess is no, but ideally no rom is calling this anyways.
		m_opCodeLookup[i] = { "NUL", nullptr, AddressMode::UNDEFINED, 2, false, false };
	}

	m_opCodeLookup[0] = { "BRK", &CPU::BRK, AddressMode::Implied, 7, false, false };
	m_opCodeLookup[1] = { "ORA", &CPU::ORA, AddressMode::INDX, 6, false, false };
	m_opCodeLookup[5] = { "ORA", &CPU::ORA, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[6] = { "ASL", &CPU::ASL, AddressMode::ZP, 5, false, false };
	m_opCodeLookup[8] = { "PHP", &CPU::PHP, AddressMode::Implied, 3, false, false };
	m_opCodeLookup[9] = { "ORA", &CPU::ORA, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[10] = { "ASL", &CPU::ASL, AddressMode::Accum, 2, false, false };
	m_opCodeLookup[13] = { "ORA", &CPU::ORA, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[14] = { "ASL", &CPU::ASL, AddressMode::Absolute, 6, false, false };
	m_opCodeLookup[16] = { "BPL", &CPU::BPL, AddressMode::Relative, 2, true, true };
	m_opCodeLookup[17] = { "ORA", &CPU::ORA, AddressMode::INDY, 5, true, false };
	m_opCodeLookup[21] = { "ORA", &CPU::ORA, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[22] = { "ASL", &CPU::ASL, AddressMode::ZPX, 6, false, false };
	m_opCodeLookup[24] = { "CLC", &CPU::CLC, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[25] = { "ORA", &CPU::ORA, AddressMode::ABSY, 3, true, false };
	m_opCodeLookup[29] = { "ORA", &CPU::ORA, AddressMode::ABSX, 3, true, false };
	m_opCodeLookup[30] = { "ASL", &CPU::ASL, AddressMode::ABSX, 7, false, false };
	m_opCodeLookup[32] = { "JSR", &CPU::JSR, AddressMode::Absolute, 6, false, false };
	m_opCodeLookup[33] = { "AND", &CPU::AND, AddressMode::INDX, 6, false, false };
	m_opCodeLookup[36] = { "BIT", &CPU::BIT, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[37] = { "AND", &CPU::AND, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[38] = { "ROL", &CPU::ROL, AddressMode::ZP, 5, false, false };
	m_opCodeLookup[40] = { "PLP", &CPU::PLP, AddressMode::Implied, 4, false, false };
	m_opCodeLookup[41] = { "AND", &CPU::AND, AddressMode::IMM, 4, false, false };
	m_opCodeLookup[42] = { "ROL", &CPU::ROL, AddressMode::Accum, 2, false, false };
	m_opCodeLookup[44] = { "BIT", &CPU::BIT, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[45] = { "AND", &CPU::AND, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[46] = { "ROL", &CPU::ROL, AddressMode::Absolute, 6, false, false };
	m_opCodeLookup[48] = { "BMI", &CPU::BMI, AddressMode::Relative, 2, true, true };
	m_opCodeLookup[49] = { "AND", &CPU::AND, AddressMode::INDY, 5, true, false };
	m_opCodeLookup[53] = { "AND", &CPU::AND, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[54] = { "ROL", &CPU::ROL, AddressMode::ZPX, 6, false, false };
	m_opCodeLookup[56] = { "SEC", &CPU::SEC, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[57] = { "AND", &CPU::AND, AddressMode::ABSY, 4, true, false };
	m_opCodeLookup[61] = { "AND", &CPU::AND, AddressMode::ABSX, 4, true, false };
	m_opCodeLookup[62] = { "ROL", &CPU::ROL, AddressMode::ABSX, 7, false, false };
	m_opCodeLookup[64] = { "RTI", &CPU::RTI, AddressMode::Implied, 6, false, false };
	m_opCodeLookup[65] = { "EOR", &CPU::EOR, AddressMode::INDX, 6, false, false };
	m_opCodeLookup[69] = { "EOR", &CPU::EOR, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[70] = { "LSR", &CPU::LSR, AddressMode::ZP, 5, false, false };
	m_opCodeLookup[72] = { "PHA", &CPU::PHA, AddressMode::Implied, 3, false, false };
	m_opCodeLookup[73] = { "EOR", &CPU::EOR, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[74] = { "LSR", &CPU::LSR, AddressMode::Accum, 2, false, false };
	m_opCodeLookup[76] = { "JMP", &CPU::JMP, AddressMode::Absolute, 3, false, false };
	m_opCodeLookup[77] = { "EOR", &CPU::EOR, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[78] = { "LSR", &CPU::LSR, AddressMode::Absolute, 6, false, false };
	m_opCodeLookup[80] = { "BVC", &CPU::BVC, AddressMode::Relative, 2, true, true };
	m_opCodeLookup[81] = { "EOR", &CPU::EOR, AddressMode::INDY, 5, true, false };
	m_opCodeLookup[85] = { "EOR", &CPU::EOR, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[86] = { "LSR", &CPU::LSR, AddressMode::ZPX, 5, false, false };
	m_opCodeLookup[88] = { "CLI", &CPU::CLI, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[89] = { "EOR", &CPU::EOR, AddressMode::ABSY, 4, true, false };
	m_opCodeLookup[93] = { "EOR", &CPU::EOR, AddressMode::ABSX, 4, true, false };
	m_opCodeLookup[94] = { "LSR", &CPU::LSR, AddressMode::ABSX, 7, false, false };
	m_opCodeLookup[96] = { "RTS", &CPU::RTS, AddressMode::Implied, 6, false, false };
	m_opCodeLookup[97] = { "ADC", &CPU::ADC, AddressMode::INDX, 6, false, false };
	m_opCodeLookup[101] = { "ADC", &CPU::ADC, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[102] = { "ROR", &CPU::ROR, AddressMode::ZP, 5, false, false };
	m_opCodeLookup[104] = { "PLA", &CPU::PLA, AddressMode::Implied, 4, false, false };
	m_opCodeLookup[105] = { "ADC", &CPU::ADC, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[106] = { "ROR", &CPU::ROR, AddressMode::Accum, 2, false, false };
	m_opCodeLookup[108] = { "JMP", &CPU::JMP, AddressMode::Indirect, 5, false, false };
	m_opCodeLookup[109] = { "ADC", &CPU::ADC, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[110] = { "ROR", &CPU::ROR, AddressMode::Absolute, 6, false, false };
	m_opCodeLookup[112] = { "BVS", &CPU::BVS, AddressMode::Relative, 2, true, true };
	m_opCodeLookup[113] = { "ADC", &CPU::ADC, AddressMode::INDY, 5, true, true };
	m_opCodeLookup[117] = { "ADC", &CPU::ADC, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[118] = { "ROR", &CPU::ROR, AddressMode::ZPX, 6, false, false };
	m_opCodeLookup[120] = { "SEI", &CPU::SEI, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[121] = { "ADC", &CPU::ADC, AddressMode::ABSY, 4, true, false };
	m_opCodeLookup[125] = { "ADC", &CPU::ADC, AddressMode::ABSX, 4, true, false };
	m_opCodeLookup[126] = { "ROR", &CPU::ROR, AddressMode::ABSX, 7, false, false };
	m_opCodeLookup[129] = { "STA", &CPU::STA, AddressMode::INDX, 6, false, false };
	m_opCodeLookup[132] = { "STY", &CPU::STY, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[133] = { "STA", &CPU::STA, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[134] = { "STX", &CPU::STX, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[136] = { "DEY", &CPU::DEY, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[138] = { "TXA", &CPU::TXA, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[140] = { "STY", &CPU::STY, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[141] = { "STA", &CPU::STA, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[142] = { "STX", &CPU::STX, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[144] = { "BCC", &CPU::BCC, AddressMode::Relative, 2, true, true };
	m_opCodeLookup[145] = { "STA", &CPU::STA, AddressMode::INDY, 6, false, false };
	m_opCodeLookup[148] = { "STY", &CPU::STY, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[149] = { "STA", &CPU::STA, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[150] = { "STX", &CPU::STX, AddressMode::ZPY, 4, false, false };
	m_opCodeLookup[152] = { "TYA", &CPU::TYA, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[153] = { "STA", &CPU::STA, AddressMode::ABSY, 5, false, false };
	m_opCodeLookup[154] = { "TXS", &CPU::TXS, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[157] = { "STA", &CPU::STA, AddressMode::ABSX, 5, false, false };
	m_opCodeLookup[160] = { "LDY", &CPU::LDY, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[161] = { "LDA", &CPU::LDA, AddressMode::INDX, 6, false, false };
	m_opCodeLookup[162] = { "LDX", &CPU::LDX, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[164] = { "LDY", &CPU::LDY, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[165] = { "LDA", &CPU::LDA, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[166] = { "LDX", &CPU::LDX, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[168] = { "TAY", &CPU::TAY, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[169] = { "LDA", &CPU::LDA, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[170] = { "TAX", &CPU::TAX, AddressMode::Implied, 3, false, false };
	m_opCodeLookup[172] = { "LDY", &CPU::LDY, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[173] = { "LDA", &CPU::LDA, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[174] = { "LDX", &CPU::LDX, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[176] = { "BCS", &CPU::BCS, AddressMode::Relative, 2, true, true };
	m_opCodeLookup[177] = { "LDA", &CPU::LDA, AddressMode::INDY, 5, true, false };
	m_opCodeLookup[180] = { "LDY", &CPU::LDY, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[181] = { "LDA", &CPU::LDA, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[182] = { "LDX", &CPU::LDX, AddressMode::ZPY, 4, false, false };
	m_opCodeLookup[184] = { "CLV", &CPU::CLV, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[185] = { "LDA", &CPU::LDA, AddressMode::ABSY, 4, true, false };
	m_opCodeLookup[186] = { "TSX", &CPU::TSX, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[188] = { "LDY", &CPU::LDY, AddressMode::ABSX, 4, true, false };
	m_opCodeLookup[189] = { "LDA", &CPU::LDA, AddressMode::ABSX, 4, true, false };
	m_opCodeLookup[190] = { "LDX", &CPU::LDX, AddressMode::ABSY, 4, true, false };
	m_opCodeLookup[192] = { "CPY", &CPU::CPY, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[193] = { "CMP", &CPU::CMP, AddressMode::INDX, 6, false, false };
	m_opCodeLookup[196] = { "CPY", &CPU::CPY, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[197] = { "CMP", &CPU::CMP, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[198] = { "DEC", &CPU::DEC, AddressMode::ZP, 5, false, false };
	m_opCodeLookup[200] = { "INY", &CPU::INY, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[201] = { "CMP", &CPU::CMP, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[202] = { "DEX", &CPU::DEX, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[204] = { "CPY", &CPU::CPY, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[205] = { "CMP", &CPU::CMP, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[206] = { "DEC", &CPU::DEC, AddressMode::Absolute, 6, false, false };
	m_opCodeLookup[208] = { "BNE", &CPU::BNE, AddressMode::Relative, 2, true, true };
	m_opCodeLookup[209] = { "CMP", &CPU::CMP, AddressMode::INDY, 5, true, false };
	m_opCodeLookup[213] = { "CMP", &CPU::CMP, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[214] = { "DEC", &CPU::DEC, AddressMode::ZPX, 6, false, false };
	m_opCodeLookup[216] = { "CLD", &CPU::CLD, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[217] = { "CMP", &CPU::CMP, AddressMode::ABSY, 4, true, false };
	m_opCodeLookup[221] = { "CMP", &CPU::CMP, AddressMode::ABSX, 4, true, false };
	m_opCodeLookup[222] = { "DEC", &CPU::DEC, AddressMode::ABSX, 7, true, false };
	m_opCodeLookup[224] = { "CPX", &CPU::CPX, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[225] = { "SBC", &CPU::SBC, AddressMode::INDX, 6, false, false };
	m_opCodeLookup[228] = { "CPX", &CPU::CPX, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[229] = { "SBC", &CPU::SBC, AddressMode::ZP, 3, false, false };
	m_opCodeLookup[230] = { "INC", &CPU::INC, AddressMode::ZP, 5, false, false };
	m_opCodeLookup[232] = { "INX", &CPU::INX, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[233] = { "SBC", &CPU::SBC, AddressMode::IMM, 2, false, false };
	m_opCodeLookup[234] = { "NOP", &CPU::NOP, AddressMode::Implied, 2, false, false };
	m_opCodeLookup[236] = { "CPX", &CPU::CPX, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[237] = { "SBC", &CPU::SBC, AddressMode::Absolute, 4, false, false };
	m_opCodeLookup[238] = { "INC", &CPU::INC, AddressMode::Absolute, 6, false, false };
	m_opCodeLookup[240] = { "BEQ", &CPU::BEQ, AddressMode::Relative, 4, true, true };
	m_opCodeLookup[241] = { "SBC", &CPU::SBC, AddressMode::INDY, 5, true, false };
	m_opCodeLookup[245] = { "SBC", &CPU::SBC, AddressMode::ZPX, 4, false, false };
	m_opCodeLookup[246] = { "INC", &CPU::INC, AddressMode::ZPX, 6, false, false };
	m_opCodeLookup[248] = { "SED", &CPU::SED, AddressMode::Implied, 4, false, false };
	m_opCodeLookup[249] = { "SBC", &CPU::SBC, AddressMode::ABSY, 4, true, false };
	m_opCodeLookup[253] = { "SBC", &CPU::SBC, AddressMode::ABSX, 4, true, false };
	m_opCodeLookup[254] = { "INC", &CPU::INC, AddressMode::ABSX, 7, false, false };
}

bool CPU::OnSamePage(uint16_t addr1, uint16_t addr2)
{
	return (addr1 & 0xFF00) == (addr2 & 0xFF00);
}

void CPU::Reset()
{
	ClearRegisters();

	uint16_t lo = m_NES->ReadCpuMemory(0xFFFC);
	uint16_t hi = m_NES->ReadCpuMemory(0xFFFD);
	m_PC = (hi << 8) | lo;

	m_clockCycles += 6;
}

void CPU::NonMaskableInterrupt()
{
	DoInterrupt(0xFFFA, 0xFFFB);
	m_clockCycles += 8;
}

void CPU::MaskableInterrupt()
{
	if (!GetIRQFlag())
	{
		DoInterrupt(0xFFFE, 0xFFFF);
		m_clockCycles += 7;
	}
}

void CPU::DebugSetPC(uint16_t newPC)
{
	m_PC = newPC;
}

void CPU::DoInterrupt(uint16_t lo, uint16_t high)
{
	// Push program counter to stack
	uint8_t pcHigh = m_PC >> 8;
	uint8_t pcLow = (uint8_t)(m_PC & 0x00FF);
	m_NES->WriteCpuMemory(m_StackLocation + m_SP, pcHigh);
	m_SP--;
	m_NES->WriteCpuMemory(m_StackLocation + m_SP, pcLow);
	m_SP--;

	// Push status register to stack
	SetUnusedFlag(true);
	SetIRQFlag(true);
	SetBrkCommandFlag(false);
	m_NES->WriteCpuMemory(m_StackLocation + m_SP, m_Status);
	m_SP--;

	// Get address to set program counter to
	uint16_t newPcLo = m_NES->ReadCpuMemory(lo);
	uint16_t newPcHigh = m_NES->ReadCpuMemory(high);

	// Set it
	m_PC = (newPcHigh << 8) | newPcLo;
}

void CPU::ClearRegisters()
{
	m_RegA = 0x00;
	m_RegX = 0x00;
	m_RegY = 0x00;

	m_SP = 0xFD;

	m_PC = 0x0000;

	m_Status = 0x00;
}

// Do one clock cycle
void CPU::Cycle()
{
	if (m_clockCycles > 0)
	{
		// "Running" the last command
	}
	else
	{
		EvaluatePC();
	}

	m_clockCycles -= 1;
}

bool CPU::DoesInstructionNeedData(Instruction instruction)
{
	if (instruction.name.compare("ORA") == 0) return true;
	if (instruction.name.compare("ASC") == 0) return true;
	if (instruction.name.compare("AND") == 0) return true;
	if (instruction.name.compare("BIT") == 0) return true;
	if (instruction.name.compare("ROC") == 0) return true;
	if (instruction.name.compare("EOR") == 0) return true;
	if (instruction.name.compare("LSR") == 0) return true;
	if (instruction.name.compare("ADC") == 0) return true;
	if (instruction.name.compare("ROR") == 0) return true;
	if (instruction.name.compare("LDY") == 0) return true;
	if (instruction.name.compare("LDX") == 0) return true;
	if (instruction.name.compare("LDA") == 0) return true;
	if (instruction.name.compare("CPY") == 0) return true;
	if (instruction.name.compare("CMP") == 0) return true;
	if (instruction.name.compare("DEC") == 0) return true;
	if (instruction.name.compare("CPX") == 0) return true;
	if (instruction.name.compare("INC") == 0) return true;
	if (instruction.name.compare("SBC") == 0) return true;
	if (instruction.name.compare("ASL") == 0) return true;
	if (instruction.name.compare("ROL") == 0) return true;
	return false;
}
void CPU::EvaluatePC()
{
	uint16_t ogPc = m_PC;
	uint8_t operation = m_NES->ReadCpuMemory(m_PC++);
	Instruction instruction = m_opCodeLookup[operation];

	m_instructionData = 0x00;
	m_instructionAddress = 0x00;
	m_branchLocation = 0x0000;
	m_clockCycles += instruction.clockCycles;

	/*
		Reading memory can have side effects, eg if you touch the PPU registers
		Check if our instruction actually needs to use the m_instructionData value, otherwise don't touch memory.
	*/
	bool needsInstructionData = DoesInstructionNeedData(instruction);

	bool pageBoundaryCrossed = false;

	uint8_t low, high, offset;
	uint16_t address, zeroPageAddress, newAddress, newAddressLow, newAddressHigh;
	int8_t signedOffset;

	// Get the data from the given address mode
	switch (instruction.addressMode)
	{
	case AddressMode::Accum:
		// Accumulator addressing, operation on the accumulator, no memory addressing
		m_instructionData = m_RegA;
		m_instructionAddress = UINT16_MAX;
		break;
	case AddressMode::IMM:
		// Immediate, the data is just the next byte in the program
		m_instructionAddress = m_PC;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(m_PC++);
		break;
	case AddressMode::Absolute:
		// Absolute, next two bytes specify a 16 memory address (so anywhere in the memory), its in little endian though
		low = m_NES->ReadCpuMemory(m_PC++);
		high = m_NES->ReadCpuMemory(m_PC++);
		address = ((high << 8) | (uint16_t)low);
		m_instructionAddress = address;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(address);
		break;
	case AddressMode::ZP:
		// Zero page, memory is on the zero page. The next byte is the 8 least significant bits
		low = m_NES->ReadCpuMemory(m_PC++);
		high = 0;
		address = ((high << 8) | (uint16_t)low);
		m_instructionAddress = address;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(address);
		break;
	case AddressMode::ZPX:
		// Same as zero page but offset by the value in the x register
		low = m_NES->ReadCpuMemory(m_PC++) + m_RegX;
		high = 0;
		address = ((high << 8) | (uint16_t)low);
		m_instructionAddress = address;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(address);
		break;
	case AddressMode::ZPY:
		// Same as zero page but offset by the value in the y register
		low = m_NES->ReadCpuMemory(m_PC++) + m_RegY;
		high = 0;
		address = ((high << 8) | (uint16_t)low);
		m_instructionAddress = address;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(address);
		break;
	case AddressMode::ABSX:
		// Same as absolute but offset by the value in the x register
		low = m_NES->ReadCpuMemory(m_PC++);
		high = m_NES->ReadCpuMemory(m_PC++);
		address = ((high << 8) | (uint16_t)low) + m_RegX;
		if (((address && 0xFF00) >> 8) != high) pageBoundaryCrossed = true;
		m_instructionAddress = address;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(address);
		break;
	case AddressMode::ABSY:
		// Same as absolute but offset by the value in the y register
		low = m_NES->ReadCpuMemory(m_PC++);
		high = m_NES->ReadCpuMemory(m_PC++);
		address = ((high << 8) | (uint16_t)low) + m_RegY;
		m_instructionAddress = address;
		if (((address && 0xFF00) >> 8) != high) pageBoundaryCrossed = true;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(address);
		break;
	case AddressMode::Implied:
		// The operation or address is implied by the instruction and doesn't need to be looked up
		break;
	case AddressMode::Relative:
		// The next byte is an offset to the program counter, which specifies a destination for the next instruction
		offset = m_NES->ReadCpuMemory(m_PC++);

		// offset is a signed value in this case
		signedOffset = (int8_t)offset;

		m_branchLocation = m_PC + signedOffset;
		break;
	case AddressMode::INDX:
		// Indexed indirect, The next byte is added with the x register to form an 8 bit address (ignore overflow),
		// that address on the zero page contains two bytes that specify the absolute address of the data we are looking for

		// This may also overflow, where the first part of the address is read at FF and the second part at 00
		offset = m_NES->ReadCpuMemory(m_PC++);
		address = (offset + m_RegX) & 0x00FF;
		low = m_NES->ReadCpuMemory(address);
		address = (address + 1) & 0x00FF;
		high = m_NES->ReadCpuMemory(address);
		newAddress = ((high << 8) | (uint16_t)low);
		m_instructionAddress = newAddress;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(newAddress);
		break;
	case AddressMode::INDY:
		// Indirect indexed, the next byte is a zero page address which contains a 16 bit address, with the y register added to that address
		
		// Like indirect X, the zero page address may overflow and wrap around
		zeroPageAddress = ((uint16_t)m_NES->ReadCpuMemory(m_PC++) & 0x00FF);
		low = m_NES->ReadCpuMemory(zeroPageAddress);
		zeroPageAddress = (zeroPageAddress + 1) & 0x00FF;
		high = m_NES->ReadCpuMemory(zeroPageAddress);
		address = ((high << 8) | (uint16_t)low) + m_RegY;
		if (((address && 0xFF00) >> 8) != high) pageBoundaryCrossed = true;
		m_instructionAddress = address;
		if (needsInstructionData) m_instructionData = m_NES->ReadCpuMemory(address);
		break;
	case AddressMode::Indirect:
		// The next two bytes are a memory address to another memory address (like a pointer I guess), that address is where the program counter will jump to
		low = m_NES->ReadCpuMemory(m_PC++);
		high = m_NES->ReadCpuMemory(m_PC++);
		address = ((high << 8) | (uint16_t)low);
		if (low == 0xFF)
		{
			// Hardware bug
			newAddressLow = m_NES->ReadCpuMemory(address);
			newAddressHigh = m_NES->ReadCpuMemory(address & m_highMask);
		}
		else
		{
			newAddressLow = m_NES->ReadCpuMemory(address++);
			newAddressHigh = m_NES->ReadCpuMemory(address);
		}
		newAddress = ((newAddressHigh << 8) | (uint16_t)newAddressLow);
		m_branchLocation = newAddress;
		break;
	}

	// Got the data! Run the instruction
	(this->*(instruction.opCode))(instruction);

	// Add variable clock cycles
	bool branched = (m_PC == m_branchLocation);

	if (instruction.pageBoundaryCycle && instruction.branchPageCycle)
	{
		if (branched)
		{
			m_clockCycles += 1;
			if (!AreAddrsOnSamePage(ogPc, m_PC)) m_clockCycles += 1;
		}
	}

	if (instruction.pageBoundaryCycle && !instruction.branchPageCycle)
	{
		if (pageBoundaryCrossed) m_clockCycles += 1;
	}
}

bool CPU::AreAddrsOnSamePage(uint16_t addr1, uint16_t addr2)
{
	return (addr1 && 0xFF00) == (addr2 && 0xFF00);
}

// Break (software interrupt)
void CPU::BRK(Instruction instruction)
{
	m_PC++;
	DoInterrupt(0xFFFE, 0xFFFF);
}

// Return from interrupt
void CPU::RTI(Instruction instruction)
{
	// Pull processor stack
	m_SP++;
	uint8_t status = m_NES->ReadCpuMemory(m_StackLocation + m_SP);
	m_Status = status;


	// Pull program counter
	m_SP++;
	uint16_t lo = m_NES->ReadCpuMemory(m_StackLocation + m_SP);
	m_SP++;
	uint16_t high = m_NES->ReadCpuMemory(m_StackLocation + m_SP);

	m_PC = (high << 8) | lo;

	// TODO, ignore bits 4 and 5 (unused / break)
}

// Bitwise OR with Accumulator
void CPU::ORA(Instruction instruction)
{
	m_RegA |= m_instructionData;

	SetZeroFlag(m_RegA == 0x00);
	SetNegativeFlag((m_RegA & 0x80) != 0);
}

// Arithmetic Shift Left
void CPU::ASL(Instruction instruction)
{
	SetCarryFlag((m_instructionData & 0x80) != 0);

	uint16_t result = m_instructionData << 1;
	uint8_t data = (result & m_lowMask);

	if (instruction.addressMode == AddressMode::Accum)
	{
		m_RegA = data;
	}
	else
	{
		m_NES->WriteCpuMemory(m_instructionAddress, data);
	}

	SetZeroFlag(data == 0x00);
	SetNegativeFlag((data & 0x80) != 0);
}

// Branch on Plus
void CPU::BPL(Instruction instruction)
{
	if (GetNegativeFlag() == 0)
	{
		if (OnSamePage(m_PC, m_branchLocation))
		{
			m_clockCycles += 1;
		}
		else
		{
			m_clockCycles += 2;
		}

		m_PC = m_branchLocation;
	}
}

// Clear carry
void CPU::CLC(Instruction instruction)
{
	SetCarryFlag(false);
}

// Bitwise AND with Accumulator
void CPU::AND(Instruction instruction)
{
	m_RegA = m_instructionData & m_RegA;

	SetZeroFlag(m_RegA == 0x00);
	SetNegativeFlag((m_RegA & 0x80) != 0);
}

// Test Bits
void CPU::BIT(Instruction instruction)
{
	SetZeroFlag((m_instructionData & m_RegA) == 0x00);
	SetNegativeFlag((m_instructionData & 0x80) != 0x00);
	SetOverflowFlag((m_instructionData & 0x40) != 0x00);
}

// Rotate all bits left
void CPU::ROL(Instruction instruction)
{
	bool setCarry = (m_instructionData & 0x80) != 0x00;
	uint16_t result = m_instructionData << 1;
	uint8_t data = (result & m_lowMask) | GetCarryFlag();

	if (instruction.addressMode == AddressMode::Accum)
	{
		m_RegA = data;
	}
	else
	{
		m_NES->WriteCpuMemory(m_instructionAddress, data);
	}

	SetCarryFlag(setCarry);
	SetZeroFlag((data & m_RegA) == 0x00);
	SetNegativeFlag((data & 0x80) != 0x00);
}

// Pull Processor Status
void CPU::PLP(Instruction instruction)
{
	// TODO, ignore bits 4 and 5 (unused / break)

	m_SP++;
	uint8_t data = m_NES->ReadCpuMemory(m_StackLocation + m_SP);
	m_Status = data;
}

// Push Processor Status
void CPU::PHP(Instruction instruction)
{
	bool unused = GetUnusedFlag();
	bool brk = GetBrkCommandFlag();

	SetUnusedFlag(true);
	SetBrkCommandFlag(true);

	m_NES->WriteCpuMemory(m_StackLocation + m_SP, m_Status);

	SetUnusedFlag(unused);
	SetBrkCommandFlag(brk);

	m_SP--;
}

// Branch on Minus
void CPU::BMI(Instruction instruction)
{
	if (GetNegativeFlag() != 0)
	{
		if (OnSamePage(m_PC, m_branchLocation))
		{
			m_clockCycles += 1;
		}
		else
		{
			m_clockCycles += 2;
		}

		m_PC = m_branchLocation;
	}
}

// Set Carry
void CPU::SEC(Instruction instruction)
{
	SetCarryFlag(true);
}

// Bitwise Exclusive OR
void CPU::EOR(Instruction instruction)
{
	m_RegA = m_instructionData ^ m_RegA;

	SetZeroFlag(m_RegA == 0x00);
	SetNegativeFlag((m_RegA & 0x80) != 0x00);
}

// Logical Shift Right
void CPU::LSR(Instruction instruction)
{
	SetCarryFlag(m_instructionData & 0x01);

	uint8_t data = m_instructionData >> 1;

	SetZeroFlag(data == 0x00);
	SetNegativeFlag(false); // Shifting a zero in on the left side


	if (instruction.addressMode == AddressMode::Accum)
	{
		m_RegA = data;
	}
	else
	{
		m_NES->WriteCpuMemory(m_instructionAddress, data);
	}
}

// Jump
void CPU::JMP(Instruction instruction)
{
	if (instruction.addressMode == AddressMode::Indirect)
	{
		m_PC = m_branchLocation;
	}
	else
	{
		m_PC = m_instructionAddress;
	}
}

// Branch if overflow clear
void CPU::BVC(Instruction instruction)
{
	if (!GetOverflowFlag())
	{
		if (OnSamePage(m_PC, m_branchLocation))
		{
			m_clockCycles += 1;
		}
		else
		{
			m_clockCycles += 2;
		}

		m_PC = m_branchLocation;
	}
}

// Clear interupt
void CPU::CLI(Instruction instruction)
{
	SetIRQFlag(false);
}

// Jump to Subroutine
void CPU::JSR(Instruction instruction)
{
	// Move back and push the program counter to the stack
	m_PC--;

	// high then low
	uint8_t high = m_PC >> 8;
	uint8_t low = (uint8_t)(m_PC & 0x00FF);
	m_NES->WriteCpuMemory(m_StackLocation + m_SP, high);
	m_SP--;
	m_NES->WriteCpuMemory(m_StackLocation + m_SP, low);
	m_SP--;

	m_PC = m_instructionAddress;
}

// Return from Subroutine
void CPU::RTS(Instruction instruction)
{
	m_SP++;
	uint8_t low = m_NES->ReadCpuMemory(m_StackLocation + m_SP);
	m_SP++;
	uint8_t high = m_NES->ReadCpuMemory(m_StackLocation + m_SP);
	uint16_t address = (((uint16_t)high) << 8) | ((uint16_t)low);
	m_PC = address + 1;
}

// Add with carry
void CPU::ADC(Instruction instruction)
{
	uint16_t data = (uint16_t)m_RegA + m_instructionData + GetCarryFlag();
	uint8_t result = (uint8_t)(data & m_lowMask);

	uint8_t a = (m_RegA & 0x80);
	uint8_t m = (m_instructionData & 0x80);
	uint8_t r = (result & 0x80);

	// Ah jeez what the fuck
	// This handles overflow for signed numbers
	bool arithmeticOverflow = (((a ^ r) & ~(a ^ m)) & 0x80);
	m_RegA = result;

	SetCarryFlag((data & m_highMask) != 0x0000);
	SetZeroFlag(result == 0x00);
	SetNegativeFlag(result & 0x80);
	SetOverflowFlag(arithmeticOverflow);
}

// Rotate right
void CPU::ROR(Instruction instruction)
{
	uint8_t ogCarry = GetCarryFlag();
	SetCarryFlag(m_instructionData & 0x01);
	uint8_t result = m_instructionData >> 1;

	if (ogCarry)
	{
		result = result | 0x80;
	}

	if (instruction.addressMode == AddressMode::Accum)
	{
		m_RegA = result;
	}
	else
	{
		m_NES->WriteCpuMemory(m_instructionAddress, result);
	}

	SetZeroFlag(result == 0x00);
	SetNegativeFlag(result & 0x80);
}

// Push accumulator to stack
void CPU::PHA(Instruction instruction)
{
	m_NES->WriteCpuMemory(m_StackLocation + m_SP, m_RegA);
	m_SP--;
}

// Pull accumulator
void CPU::PLA(Instruction instruction)
{
	m_SP++; // Stack pointer is point to empty space on the top of the stack, go back one to get the top value
	m_RegA = m_NES->ReadCpuMemory(m_StackLocation + m_SP);
	SetZeroFlag(m_RegA == 0x00);
	SetNegativeFlag(m_RegA & 0x80);
}

// Branch if overflow set
void CPU::BVS(Instruction instruction)
{
	if (GetOverflowFlag())
	{
		if (OnSamePage(m_PC, m_branchLocation))
		{
			m_clockCycles += 1;
		}
		else
		{
			m_clockCycles += 2;
		}

		m_PC = m_branchLocation;
	}
}

// Set interrupt
void CPU::SEI(Instruction instruction)
{
	SetIRQFlag(true);
}

// Store accumulator
void CPU::STA(Instruction instruction)
{
	m_NES->WriteCpuMemory(m_instructionAddress, m_RegA);
}

// Store y register
void CPU::STY(Instruction instruction)
{
	m_NES->WriteCpuMemory(m_instructionAddress, m_RegY);
}

// Store x register
void CPU::STX(Instruction instruction)
{
	m_NES->WriteCpuMemory(m_instructionAddress, m_RegX);
}

// Decrement Y
void CPU::DEY(Instruction instruction)
{
	m_RegY -= 1;

	SetNegativeFlag(m_RegY & 0x80);
	SetZeroFlag(m_RegY == 0x00);
}

// Decrement X
void CPU::DEX(Instruction instruction)
{
	m_RegX -= 1;

	SetNegativeFlag(m_RegX & 0x80);
	SetZeroFlag(m_RegX == 0x00);
}

// Transfer X to A
void CPU::TXA(Instruction instruction)
{
	m_RegA = m_RegX;

	SetNegativeFlag(m_RegA & 0x80);
	SetZeroFlag(m_RegA == 0x00);
}

// Transfer A to X
void CPU::TAX(Instruction instruction)
{
	m_RegX = m_RegA;

	SetNegativeFlag(m_RegX & 0x80);
	SetZeroFlag(m_RegX == 0x00);
}

// Transfer A to Y
void CPU::TAY(Instruction instruction)
{
	m_RegY = m_RegA;

	SetNegativeFlag(m_RegY & 0x80);
	SetZeroFlag(m_RegY == 0x00);
}

// Branch on carry clear
void CPU::BCC(Instruction instruction)
{
	if (!GetCarryFlag())
	{
		if (OnSamePage(m_PC, m_branchLocation))
		{
			m_clockCycles += 1;
		}
		else
		{
			m_clockCycles += 2;
		}

		m_PC = m_branchLocation;
	}
}

// Transfer x to stack pointer
void CPU::TXS(Instruction instruction)
{
	m_SP = m_RegX;
}

// Load Y register
void CPU::LDY(Instruction instruction)
{
	m_RegY = m_instructionData;

	SetNegativeFlag(m_RegY & 0x80);
	SetZeroFlag(m_RegY == 0x00);
}

// Load X Register
void CPU::LDX(Instruction instruction)
{
	m_RegX = m_instructionData;

	SetNegativeFlag(m_RegX & 0x80);
	SetZeroFlag(m_RegX == 0x00);
}

// Load Accumulator
void CPU::LDA(Instruction instruction)
{
	m_RegA = m_instructionData;

	SetNegativeFlag(m_RegA & 0x80);
	SetZeroFlag(m_RegA == 0x00);
}

// Transfer stack ptr to X
void CPU::TSX(Instruction instruction)
{
	m_RegX = m_SP;
	SetZeroFlag(m_RegX == 0x00);
	SetNegativeFlag(m_RegX & 0x80);
}

// Branch on carry set
void CPU::BCS(Instruction instruction)
{
	if (GetCarryFlag())
	{
		if (OnSamePage(m_PC, m_branchLocation))
		{
			m_clockCycles += 1;
		}
		else
		{
			m_clockCycles += 2;
		}

		m_PC = m_branchLocation;
	}
}

// Clear overflow
void CPU::CLV(Instruction instruction)
{
	SetOverflowFlag(false);
}

// Compare Y
void CPU::CPY(Instruction instruction)
{
	SetCarryFlag(m_RegY >= m_instructionData);
	SetZeroFlag(m_RegY == m_instructionData);
	SetNegativeFlag(((m_RegY - m_instructionData) & 0x80) != 0);
}

// Compare Accumulator
void CPU::CMP(Instruction instruction)
{
	SetCarryFlag(m_RegA >= m_instructionData);
	SetZeroFlag(m_RegA == m_instructionData);
	SetNegativeFlag(((m_RegA-m_instructionData)&0x80) != 0);
}

// Decrement memory
void CPU::DEC(Instruction instruction)
{
	uint8_t value = m_instructionData - 0x01;
	m_NES->WriteCpuMemory(m_instructionAddress, value);
	SetZeroFlag(value == 0x00);
	SetNegativeFlag(value & 0x80);
}

// Increment Y
void CPU::INY(Instruction instruction)
{
	m_RegY += 0x01;
	SetZeroFlag(m_RegY == 0x00);
	SetNegativeFlag(m_RegY & 0x80);
}

// Branch if not equal
void CPU::BNE(Instruction instruction)
{
	if (!GetZeroFlag())
	{
		if (OnSamePage(m_PC, m_branchLocation))
		{
			m_clockCycles += 1;
		}
		else
		{
			m_clockCycles += 2;
		}

		m_PC = m_branchLocation;
	}
}
// Clear decimal
void CPU::CLD(Instruction instruction)
{
	SetDecimalModeFlag(false);
}

// Compare X Register
void CPU::CPX(Instruction instruction)
{
	SetCarryFlag(m_RegX >= m_instructionData);
	SetZeroFlag(m_RegX == m_instructionData);
	SetNegativeFlag(((m_RegX - m_instructionData) & 0x80) != 0);
}

// Transfer Y to A
void CPU::TYA(Instruction instruction)
{
	m_RegA = m_RegY;

	SetNegativeFlag(m_RegA & 0x80);
	SetZeroFlag(m_RegA == 0x00);
}

// Subtract with carry
void CPU::SBC(Instruction instruction)
{

	uint16_t result;
	if (GetCarryFlag())
	{
		result = m_RegA + (m_instructionData ^ 0x00FF) + 0x01;
	}
	else
	{
		result = m_RegA + (m_instructionData ^ 0x00FF);
	}
	SetZeroFlag((result & 0x00FF) == 0x000);
	SetOverflowFlag((result ^ m_RegA) & (result ^ (m_instructionData ^ 0x00FF)) & 0x0080); // TODO: wrap your brain around this
	SetNegativeFlag((result & 0x0080) != 0);
	SetCarryFlag((result & 0xFF00) != 0);
	m_RegA = (uint8_t)(result & 0x00FF);
}

// Increment memory
void CPU::INC(Instruction instruction)
{
	uint8_t result = m_instructionData + 0x01;
	m_NES->WriteCpuMemory(m_instructionAddress, result);
	SetNegativeFlag(result & 0x80);
	SetZeroFlag(result == 0x00);
}

// Increment X
void CPU::INX(Instruction instruction)
{
	m_RegX = m_RegX + 0x01;
	SetNegativeFlag(m_RegX & 0x80);
	SetZeroFlag(m_RegX == 0x00);
}

// No operation
void CPU::NOP(Instruction instruction)
{
	/*
`;-.          ___,
  `.`\_...._/`.-"`
	\        /      ,
	/()   () \    .' `-._
   |)  .    ()\  /   _.'
   \  -'-     ,; '. <
	;.__     ,;|   > \
   / ,    / ,  |.-'.-'
  (_/    (_/ ,;|.<`
	\    ,     ;-`
	 >   \    /
	(_,-'`> .'
		 (_,'
	*/
}

// Branch if equal
void CPU::BEQ(Instruction instruction)
{
	if (GetZeroFlag())
	{
		if (OnSamePage(m_PC, m_branchLocation))
		{
			m_clockCycles += 1;
		}
		else
		{
			m_clockCycles += 2;
		}

		m_PC = m_branchLocation;
	}
}


// Set decimal
void CPU::SED(Instruction instruction)
{
	SetDecimalModeFlag(true);
}

void CPU::SetNegativeFlag(bool on)
{
	m_Status = on ? m_Status | m_negativeMask : m_Status & ~m_negativeMask;
}

void CPU::SetOverflowFlag(bool on)
{
	m_Status = on ? m_Status | m_overflowMask : m_Status & ~m_overflowMask;
}

void CPU::SetBrkCommandFlag(bool on)
{
	m_Status = on ? m_Status | m_brkMask : m_Status & ~m_brkMask;
}

void CPU::SetUnusedFlag(bool on)
{
	m_Status = on ? m_Status | m_unusedMask : m_Status & ~m_unusedMask;
}

void CPU::SetDecimalModeFlag(bool on)
{
	m_Status = on ? m_Status | m_decimalMask : m_Status & ~m_decimalMask;
}

void CPU::SetIRQFlag(bool on)
{
	m_Status = on ? m_Status | m_irqMask : m_Status & ~m_irqMask;
}

void CPU::SetZeroFlag(bool on)
{
	m_Status = on ? m_Status | m_zeroMask : m_Status & ~m_zeroMask;
}

void CPU::SetCarryFlag(bool on)
{
	m_Status = on ? m_Status | m_carryMask : m_Status & ~m_carryMask;
}

uint8_t CPU::GetNegativeFlag()
{
	return (m_Status & m_negativeMask) == 0 ? 0 : 1;
}

uint8_t CPU::GetOverflowFlag()
{
	return (m_Status & m_overflowMask) == 0 ? 0 : 1;
}

uint8_t CPU::GetUnusedFlag()
{
	return (m_Status & m_unusedMask) == 0 ? 0 : 1;
}

uint8_t CPU::GetBrkCommandFlag()
{
	return (m_Status & m_brkMask) == 0 ? 0 : 1;
}

uint8_t CPU::GetDecimalModeFlag()
{
	return (m_Status & m_decimalMask) == 0 ? 0 : 1;
}

uint8_t CPU::GetIRQFlag()
{
	return (m_Status & m_irqMask) == 0 ? 0 : 1;
}

uint8_t CPU::GetZeroFlag()
{
	return (m_Status & m_zeroMask) == 0 ? 0 : 1;
}

uint8_t CPU::GetCarryFlag()
{
	return (m_Status & m_carryMask) == 0 ? 0 : 1;
}

std::map<uint16_t, std::string> CPU::Disassemble(uint16_t nStart, uint16_t nStop)
{
	uint32_t addr = nStart;
	uint8_t value = 0x00, lo = 0x00, hi = 0x00;
	std::map<uint16_t, std::string> mapLines;
	uint16_t line_addr = 0;

	// A convenient utility to convert variables into
	// hex strings because "modern C++"'s method with
	// streams is atrocious
	auto hex = [](uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	// Starting at the specified address we read an instruction
	// byte, which in turn yields information from the lookup table
	// as to how many additional bytes we need to read and what the
	// addressing mode is. I need this info to assemble human readable
	// syntax, which is different depending upon the addressing mode

	// As the instruction is decoded, a std::string is assembled
	// with the readable output
	while (addr <= (uint32_t)nStop)
	{
		line_addr = addr;

		// Prefix line with instruction address
		std::string sInst = "$" + hex(addr, 4) + ": ";

		// Read instruction, and get its readable name
		uint8_t opcode = m_NES->ReadCpuMemory(addr); addr++;
		sInst += m_opCodeLookup[opcode].name + " ";

		// Get oprands from desired locations, and form the
		// instruction based upon its addressing mode. These
		// routines mimmick the actual fetch routine of the
		// 6502 in order to get accurate data as part of the
		// instruction
		if (m_opCodeLookup[opcode].addressMode == AddressMode::Implied)
		{
			sInst += " {IMP}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::IMM)
		{
			value = m_NES->ReadCpuMemory(addr); addr++;
			sInst += "#$" + hex(value, 2) + " {IMM}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::ZP)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = 0x00;
			sInst += "$" + hex(lo, 2) + " {ZP0}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::ZPX)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = 0x00;
			sInst += "$" + hex(lo, 2) + ", X {ZPX}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::ZPY)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = 0x00;
			sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::INDX)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = 0x00;
			sInst += "($" + hex(lo, 2) + ", X) {IZX}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::INDY)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = 0x00;
			sInst += "($" + hex(lo, 2) + "), Y {IZY}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::Absolute)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = m_NES->ReadCpuMemory(addr); addr++;
			sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::ABSX)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = m_NES->ReadCpuMemory(addr); addr++;
			sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::ABSY)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = m_NES->ReadCpuMemory(addr); addr++;
			sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::Indirect)
		{
			lo = m_NES->ReadCpuMemory(addr); addr++;
			hi = m_NES->ReadCpuMemory(addr); addr++;
			sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";
		}
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::Relative)
		{
			value = m_NES->ReadCpuMemory(addr); addr++;
			sInst += "$" + hex(value, 2) + " [$" + hex(addr + (int8_t)value, 4) + "] {REL}";
		} 
		else if (m_opCodeLookup[opcode].addressMode == AddressMode::Accum)
		{
			sInst += " {Accum}";
		}

		// Add the formed string to a std::map, using the instruction's
		// address as the key. This makes it convenient to look for later
		// as the instructions are variable in length, so a straight up
		// incremental index is not sufficient.
		mapLines[line_addr] = sInst;
	}

	return mapLines;
}
