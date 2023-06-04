#include <fstream>
#include <iostream>
#include <string>

#include "NES.h"

void NES::PowerOn()
{
	for (auto& i : CPUMemory) i = 0x00;
	for (auto& i : PPUMemory) i = 0x00;

	CPU.Initialize(this);
	PPU.Initialize(this);
}

void NES::LoadGameCartridge(GameCartridge game)
{
	m_gameMirroring = game.GetMirroringArrangement();

	// Get data
	std::vector<uint8_t> prg = game.GetPrgData();
	std::vector<uint8_t> chr = game.GetChrData();

	// Copy pgr data to $8000 - $FFFF on CPU
	for (int i = 0; i < 32768; i++)
	{
		uint16_t addr = 0x8000 + i;
		WriteCpuMemory(addr, prg[i]);
	}

	// Copy chr data to $0000 - $3FFF on PPU
	for (int i = 0; i < 16384; i++)
	{
		uint16_t addr = 0x0000 + i;
		WritePPUMemory(addr, chr[i]);
	}
}

void NES::RequestNMI()
{
	m_doNMI = true;
}

void NES::RequestIRQ()
{
	m_doIRQ = true;
}

bool NES::IsRamRegister(uint16_t address)
{
	return (address >= 0x0000 && address <= 0x1FFF);
}

bool NES::IsPpuRegister(uint16_t address)
{
	return (address >= 0x2000 && address <= 0x3FFF);
}

void NES::Tick()
{
	PPU.Cycle();
	if (m_globalClockCount % 3 == 0)
		CPU.Cycle();

	if (m_doNMI)
	{
		m_doNMI = false;
		CPU.NonMaskableInterrupt();
	}

	if (m_doIRQ)
	{
		m_doIRQ = false;
		CPU.MaskableInterrupt();
	}

	m_globalClockCount += 1;
}

void NES::Clock(bool completeInstruction)
{
	if (completeInstruction)
	{
		// Force clock cycles until the cpu is ready to execute
		while (m_globalClockCount % 3 != 0)
		{
			Tick();
		}

		// Tick CPU (execute the program counter presumably), until the operation is complete
		do
		{
			Tick();
		} while (CPU.GetClockCycles() != 0);
	}
	else
	{
		Tick();
	}
}

void NES::ClockFullFrame()
{
	do
	{
		Tick();
	} while (!PPU.IsFrameComplete() && !debugRequestStop);

	debugRequestStop = false;
}

void NES::WriteCpuMemory(uint16_t address, uint8_t data)
{
	if (IsRamRegister(address))
	{
		// If first three bits are zero we are doing a ram write which needs mirroring
		// Within $0000 - $1FFF

		uint16_t ramAddress = address & m_ramAddressMask;

		// Write to the four mirrored ram locations
		CPUMemory[ramAddress | m_ramMirror0] = data;
		CPUMemory[ramAddress | m_ramMirror1] = data;
		CPUMemory[ramAddress | m_ramMirror2] = data;
		CPUMemory[ramAddress | m_ramMirror3] = data;
	}
	else if (IsPpuRegister(address))
	{
		// PPU Registers have a lot of side effects rather than just reading / writing. 
		// Delegate the functionality to the PPU and let it handle it.
		PPU.WriteRegister(address, data);
	}
	else if (address == 0x4014)
	{
		// Activate DMA for the PPU OAM data

		// A DMA transfer actually suspends the CPU for 512 clock cycles but I think that's needlessly complex for this.
		// I am just going to transfer all the data at once, which means the PPU is running faster than it would 
		// on a real console. Maybe side effects? 
		uint16_t startAddress = (data << 8); // Page to transfer is the data passed in
		for (int i = 0; i < 256; i++)
		{
			PPU.WriteOAMMemory(i, ReadCpuMemory(startAddress + i));
		}
	}
	else if (address == 0x4016 || address == 0x4017)
	{
		uint8_t contollerLatchMask = data & 0x01;
		if (contollerLatchMask)
		{
			// Poll input
			FirstControllerLatch = FirstControllerButtonState;
			SecondControllerLatch = SecondControllerButtonState;
		}
		else
		{
			// Return to serial mode
			FirstControllerShift = FirstControllerLatch;
			SecondControllerShift = SecondControllerLatch;
		}
	}
	else
	{
		CPUMemory[address] = data;
	}
}

uint8_t NES::ReadCpuMemory(uint16_t address, bool peekMode)
{
	if (IsPpuRegister(address))
	{
		// PPU Registers have a lot of side effects rather than just reading / writing. 
		// Delegate the functionality to the PPU and let it handle it.
		if (peekMode)
		{
			PPU.PeekRegister(address);
		}
		else
		{
			PPU.GetRegister(address);
		}
	}
	else if (address == 0x4016)
	{
		/* First Controller Polling */
		bool data = (FirstControllerShift & 0x80) > 0;
		FirstControllerShift <<= 1;
		return data;
	}
	else if (address == 0x4017)
	{
		/* Second Controller Polling */
		bool data = (SecondControllerShift & 0x80) > 0;
		SecondControllerShift <<= 1;
		return data;
	}
	else
	{
		return CPUMemory[address];
	}
}

uint16_t NES::MirrorPPUAddress(uint16_t address)
{
	uint16_t mirroredAddress = address & 0x3FFF;
	if (mirroredAddress >= 0x3F00 && mirroredAddress < 0x4000)
	{
		mirroredAddress &= 0x001F;
		if (mirroredAddress == 0x0010) mirroredAddress = 0x0000;
		if (mirroredAddress == 0x0014) mirroredAddress = 0x0004;
		if (mirroredAddress == 0x0018) mirroredAddress = 0x0008;
		if (mirroredAddress == 0x001C) mirroredAddress = 0x000C;
		mirroredAddress |= 0x3F00;
	}
	if (mirroredAddress >= 0x3000 && mirroredAddress <= 0x3EFF)
	{
		// Mirrors $2000 - $2EFF
		mirroredAddress &= ~0x1000;
	}
	if (mirroredAddress >= 0x2000 && mirroredAddress <= 0x2FFF)
	{
		if (GetCartridgeMirroring())
		{
			// Vertical name table mirroring
			// 2000 mirrors 2800, 2400 mirrors 2C00
			if (mirroredAddress >= 0x2800 && mirroredAddress <= 0x2FFF)
				mirroredAddress -= 0x0800;
		}
		else
		{
			// Horizontal name table mirroring
			// 2000 mirrors 2400, 2800 mirrors 2C00
			if (mirroredAddress >= 0x2400 && mirroredAddress <= 0x27FF)
				mirroredAddress -= 0x0400;
			if (mirroredAddress >= 0x2C00 && mirroredAddress <= 0x2FFF)
				mirroredAddress -= 0x0400;
		}
	}

	return mirroredAddress;
}

void NES::WritePPUMemory(uint16_t address, uint8_t data)
{
	PPUMemory[MirrorPPUAddress(address)] = data;
}

uint8_t NES::ReadPPUMemory(uint16_t address)
{
	return PPUMemory[MirrorPPUAddress(address)];
}