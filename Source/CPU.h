#pragma once

#include <cstdint>
#include <array>
#include <map>

class NES;

class CPU
{
public:
	CPU();
	~CPU();

	enum class AddressMode
	{
		UNDEFINED = -1,
		Accum = 0,
		IMM = 1,
		Absolute = 2,
		ZP = 3,
		ZPX = 4,
		ZPY = 5,
		ABSX = 6,
		ABSY = 7,
		Implied = 8,
		Relative = 9,
		INDX = 10,
		INDY = 11,
		Indirect = 12
	};

	struct Instruction
	{
		std::string name;
		void(CPU::* opCode)(Instruction instruction);
		AddressMode addressMode;
		int clockCycles;
		bool pageBoundaryCycle;
		bool branchPageCycle;
	};

	void Initialize(NES *console);
	void BuildOpCodeLookup();
	void Cycle();

	// Hardware Interrupts
	void Reset();
	void NonMaskableInterrupt(); // NMI
	void MaskableInterrupt(); // IRQ

	void DebugSetPC(uint16_t newPc);

	std::map<uint16_t, std::string> Disassemble(uint16_t nStart, uint16_t nStop);

	void SetAccum(uint8_t data) { m_RegA = data; };

	uint16_t GetProgramCounter() { return m_PC; }
	uint8_t GetRegA() { return m_RegA; }
	uint8_t GetRegX() { return m_RegX; }
	uint8_t GetRegY() { return m_RegY; }
	uint8_t GetStackPointer() { return m_SP; }
	uint16_t GetClockCycles() { return m_clockCycles; }
	uint8_t GetNegativeFlag();
	uint8_t GetOverflowFlag();
	uint8_t GetUnusedFlag();
	uint8_t GetBrkCommandFlag();
	uint8_t GetDecimalModeFlag();
	uint8_t GetIRQFlag();
	uint8_t GetZeroFlag();
	uint8_t GetCarryFlag();

	bool AreAddrsOnSamePage(uint16_t addr1, uint16_t addr2);

private:
	int globalCycles = 0;

	/* The console*/
	NES  *m_NES = nullptr;

	/* Registers */
	uint8_t m_RegA = 0x00;
	uint8_t m_RegX = 0x00;
	uint8_t m_RegY = 0x00;

	/* Stack Pointer */
	uint8_t m_SP = 0xFF;
	const uint16_t m_StackLocation = 0x0100;

	/* Program Counter */
	uint16_t m_PC = 0x0000;

	/* Data from the address mode to use in the instruction */
	uint8_t m_instructionData = 0x00;
	uint16_t m_instructionAddress = 0x0000;
	uint16_t m_branchLocation = 0x0000;

	uint16_t m_clockCycles = 0;

	void ClearRegisters();
	void EvaluatePC();

	/* Utility */
	bool OnSamePage(uint16_t addr1, uint16_t addr2);
	bool DoesInstructionNeedData(Instruction instruction);

	/* Interrupt */
	void DoInterrupt(uint16_t lo, uint16_t high);

	/* Status Flags */
	/* Negative / Overflow / Unused? / Brk Command / Decimal Mode / IRQ Disable / Zero / Carry */
	uint8_t m_Status =             0x00;
	const uint16_t m_highMask =    0xFF00;
	const uint16_t m_lowMask =     0x00FF;
	const uint8_t m_negativeMask = 0b10000000;
	const uint8_t m_overflowMask = 0b01000000;
	const uint8_t m_unusedMask =   0b00100000;
	const uint8_t m_brkMask =      0b00010000;
	const uint8_t m_decimalMask =  0b00001000;
	const uint8_t m_irqMask =      0b00000100;
	const uint8_t m_zeroMask =     0b00000010;
	const uint8_t m_carryMask =    0b00000001;

	void SetNegativeFlag(bool on);
	void SetOverflowFlag(bool on);
	void SetUnusedFlag(bool on);
	void SetBrkCommandFlag(bool on);
	void SetDecimalModeFlag(bool on);
	void SetIRQFlag(bool enable);
	void SetZeroFlag(bool on);
	void SetCarryFlag(bool on);

	/* Op Code Lookup */
	std::array<Instruction, 256> m_opCodeLookup;

	/* Op Codes */
	void ADC(Instruction instruction);
	void AND(Instruction instruction);
	void ASL(Instruction instruction);
	void BCC(Instruction instruction);
	void BCS(Instruction instruction);
	void BEQ(Instruction instruction);
	void BIT(Instruction instruction);
	void BMI(Instruction instruction);
	void BNE(Instruction instruction);
	void BPL(Instruction instruction);
	void BRK(Instruction instruction);
	void BVC(Instruction instruction);
	void BVS(Instruction instruction);
	void CLC(Instruction instruction);
	void CLD(Instruction instruction);
	void CLI(Instruction instruction);
	void CLV(Instruction instruction);
	void CMP(Instruction instruction);
	void CPX(Instruction instruction);
	void CPY(Instruction instruction);
	void DEC(Instruction instruction);
	void DEX(Instruction instruction);
	void DEY(Instruction instruction);
	void EOR(Instruction instruction);
	void INC(Instruction instruction);
	void INX(Instruction instruction);
	void INY(Instruction instruction);
	void JMP(Instruction instruction);
	void JSR(Instruction instruction);
	void LDA(Instruction instruction);
	void LDX(Instruction instruction);
	void LDY(Instruction instruction);
	void LSR(Instruction instruction);
	void NOP(Instruction instruction);
	void ORA(Instruction instruction);
	void PHA(Instruction instruction);
	void PHP(Instruction instruction);
	void PLA(Instruction instruction);
	void PLP(Instruction instruction);
	void ROL(Instruction instruction);
	void ROR(Instruction instruction);
	void RTI(Instruction instruction);
	void RTS(Instruction instruction);
	void SBC(Instruction instruction);
	void SEC(Instruction instruction);
	void SED(Instruction instruction);
	void SEI(Instruction instruction);
	void STA(Instruction instruction);
	void STX(Instruction instruction);
	void STY(Instruction instruction);
	void TAX(Instruction instruction);
	void TAY(Instruction instruction);
	void TSX(Instruction instruction);
	void TXA(Instruction instruction);
	void TXS(Instruction instruction);
	void TYA(Instruction instruction);
};
