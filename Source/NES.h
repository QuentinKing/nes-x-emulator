#pragma once

#include <array>
#include <cstdint>

#include "CPU.h"
#include "PPU.h"
#include "GameCartridge.h"

class NES
{
public:
	NES() {};
	~NES() {};

	CPU CPU;
	PPU PPU;

	void PowerOn();
	void LoadGameCartridge(GameCartridge game);

	void RequestNMI();
	void RequestIRQ();

	// CPU gets 64K of memory
	void WriteCpuMemory(uint16_t address, uint8_t data);
	uint8_t ReadCpuMemory(uint16_t address, bool peekMode = false);

	// PPU gets 64K of memory but it's really just 16K mirrored 4 times
	void WritePPUMemory(uint16_t address, uint8_t data);
	uint8_t ReadPPUMemory(uint16_t address);

	uint16_t MirrorPPUAddress(uint16_t address);

	void SetFirstControllerState(uint8_t state) { FirstControllerButtonState = state; }
	void SetSecondControllerState(uint8_t state) { SecondControllerButtonState = state; }

	uint8_t GetCartridgeMirroring() { return m_gameMirroring; }
	uint8_t GetFirstControllerShift() { return FirstControllerShift; }
	uint8_t GetSecondControllerShift() { return SecondControllerShift; }

	void Tick();
	void Clock(bool completeInstruction);
	void ClockFullFrame();

	bool debugRequestStop = false;

private:
	NES(const NES&) = delete;
	NES& operator=(const NES&) = delete;

	bool m_doNMI = false;
	bool m_doIRQ = false;

	uint8_t m_gameMirroring; // TODO: data shouldnt live in NES class

	/* Controllers */
	// Button state is the current state of buttons pressed on a frame, may not actually get polled by the rom though
	// Latch will store and save the button state when 1 is written to $4016
	// When 0 is written to $4016, the latch is saved to the shift register
	// More info at https://www.nesdev.org/wiki/Controller_reading
	uint8_t FirstControllerButtonState = 0x00;
	uint8_t FirstControllerLatch = 0x00;
	uint8_t FirstControllerShift = 0x00;

	uint8_t SecondControllerButtonState = 0x00;
	uint8_t SecondControllerLatch = 0x00;
	uint8_t SecondControllerShift = 0x00;

	const uint16_t m_nonRamMask = 0xE000;
	const uint16_t m_ramAddressMask = 0x07FF;
	const uint16_t m_ramMirror0 = 0x0000;
	const uint16_t m_ramMirror1 = 0x0800;
	const uint16_t m_ramMirror2 = 0x1000;
	const uint16_t m_ramMirror3 = 0x1800;

	long int m_globalClockCount = 0;

	bool IsRamRegister(uint16_t address);
	bool IsPpuRegister(uint16_t address);

	std::array<uint8_t, 64 * 1024> CPUMemory;
	std::array<uint8_t, 64 * 1024> PPUMemory;
};
