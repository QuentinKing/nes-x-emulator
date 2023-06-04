// NesEmulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>

#include "NES.h"
#include "GameCartridge.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class VisualOutput : public olc::PixelGameEngine
{
private:
	NES m_Nes;

	bool m_enableThrottling = true;

	int cycles = 0;
	float frameTime = 0.0f;
	int m_PlayMode = 0;
	int m_RenderingMode = 1;
	int m_EmulationMode = 0;
	bool m_SpriteMode = true;
	bool m_DisplayPpuMemory = false;
	uint16_t m_customMemoryDrawPage = 0x0100;
	std::chrono::steady_clock::time_point m_prevFrameStart;

public:
	VisualOutput() { sAppName = "NES Debugger"; }
	std::map<uint16_t, std::string> mapAsm;

	std::string hex(uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns)
	{
		int nRamX = x, nRamY = y;
		for (int row = 0; row < nRows; row++)
		{
			std::string sOffset = "$" + hex(nAddr, 4) + ":";
			for (int col = 0; col < nColumns; col++)
			{
				if (m_DisplayPpuMemory)
				{
					sOffset += " " + hex(m_Nes.ReadPPUMemory(nAddr), 2);
				}
				else
				{
					sOffset += " " + hex(m_Nes.ReadCpuMemory(nAddr, true), 2);
				}
				nAddr += 1;
			}
			DrawString(nRamX, nRamY, sOffset);
			nRamY += 10;
		}
	}

	void DrawCpu(int x, int y)
	{
		std::string status = "STATUS: ";
		DrawString(x, y, "STATUS:", olc::WHITE);
		DrawString(x + 64, y, "N", m_Nes.CPU.GetNegativeFlag() ? olc::GREEN : olc::RED);
		DrawString(x + 80, y, "V", m_Nes.CPU.GetOverflowFlag() ? olc::GREEN : olc::RED);
		DrawString(x + 96, y, "-", m_Nes.CPU.GetUnusedFlag() ? olc::GREEN : olc::RED);
		DrawString(x + 112, y, "B", m_Nes.CPU.GetBrkCommandFlag() ? olc::GREEN : olc::RED);
		DrawString(x + 128, y, "D", m_Nes.CPU.GetDecimalModeFlag() ? olc::GREEN : olc::RED);
		DrawString(x + 144, y, "I", m_Nes.CPU.GetIRQFlag() ? olc::GREEN : olc::RED);
		DrawString(x + 160, y, "Z", m_Nes.CPU.GetZeroFlag() ? olc::GREEN : olc::RED);
		DrawString(x + 178, y, "C", m_Nes.CPU.GetCarryFlag() ? olc::GREEN : olc::RED);
		DrawString(x, y + 10, "PC: $" + hex(m_Nes.CPU.GetProgramCounter(), 4));
		DrawString(x + 128, y + 30, "PPU IO: $" + hex(m_Nes.PPU.GetPPUIOAddress(), 4));
		DrawString(x + 128, y + 20, "PPU Latch: $" + hex(m_Nes.PPU.GetPPULatchAddress(), 4));
		DrawString(x + 128, y + 40, "CTRL Latch: $" + hex(m_Nes.GetFirstControllerShift(), 2));
		DrawString(x, y + 20, "A: $" + hex(m_Nes.CPU.GetRegA(), 2) + "  [" + std::to_string(m_Nes.CPU.GetRegA()) + "]");
		DrawString(x, y + 30, "X: $" + hex(m_Nes.CPU.GetRegX(), 2) + "  [" + std::to_string(m_Nes.CPU.GetRegX()) + "]");
		DrawString(x, y + 40, "Y: $" + hex(m_Nes.CPU.GetRegY(), 2) + "  [" + std::to_string(m_Nes.CPU.GetRegY()) + "]");
		DrawString(x + 128, y + 50, "Cycles: " + std::to_string(cycles));
		DrawString(x, y + 50, "Stack P: $" + hex(m_Nes.CPU.GetStackPointer(), 2));
	}

	void DrawCode(int x, int y, int nLines)
	{
		auto it_a = mapAsm.find(m_Nes.CPU.GetProgramCounter());
		int nLineY = (nLines >> 1) * 10 + y;
		if (it_a != mapAsm.end())
		{
			DrawString(x, nLineY, (*it_a).second, olc::CYAN);
			while (nLineY < (nLines * 10) + y)
			{
				nLineY += 10;
				if (++it_a != mapAsm.end())
				{
					DrawString(x, nLineY, (*it_a).second);
				}
			}
		}

		it_a = mapAsm.find(m_Nes.CPU.GetProgramCounter());
		nLineY = (nLines >> 1) * 10 + y;
		if (it_a != mapAsm.end())
		{
			while (nLineY > y)
			{
				nLineY -= 10;
				if (--it_a != mapAsm.end())
				{
					DrawString(x, nLineY, (*it_a).second);
				}
			}
		}
	}

	bool OnUserCreate()
	{
		m_Nes.PowerOn();

		std::shared_ptr<GameCartridge> game = std::make_shared<GameCartridge>();
		game->LoadRomFromFile("Q:/Coding/NesEmulatorProject/NesEmulator/NesEmulator/Roms/smb.nes");
		m_Nes.LoadGameCartridge(*game);

		mapAsm = m_Nes.CPU.Disassemble(0x0000, 0xFFFF);

		m_Nes.CPU.Reset();

		return true;
	}

	void HandleNESControllerInput()
	{
		uint8_t input = 0x00;
		if (GetKey(olc::Key::RIGHT).bHeld) { input |= 0x01; }
		if (GetKey(olc::Key::LEFT).bHeld) { input |= 0x02; }
		if (GetKey(olc::Key::DOWN).bHeld) { input |= 0x04; }
		if (GetKey(olc::Key::UP).bHeld) { input |= 0x08; }
		if (GetKey(olc::Key::S).bHeld) { input |= 0x10; } // Start
		if (GetKey(olc::Key::A).bHeld) { input |= 0x20; } // Select
		if (GetKey(olc::Key::X).bHeld) { input |= 0x40; } // B
		if (GetKey(olc::Key::Z).bHeld) { input |= 0x80; } // A
		m_Nes.SetFirstControllerState(input);
		m_Nes.SetSecondControllerState(0x00);
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		if (m_enableThrottling)
		{
			std::chrono::steady_clock::time_point curTime = std::chrono::high_resolution_clock::now();
			long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - m_prevFrameStart).count();
			if (duration < 16.66666) return true; // Throttle for next frame

			m_prevFrameStart = std::chrono::high_resolution_clock::now();
		}

#if DEBUG
		Clear(olc::DARK_BLUE);
#else
		Clear(olc::BLACK);
#endif

		HandleNESControllerInput();

		if (GetKey(olc::Key::R).bPressed)
		{
			cycles = 0;
			m_Nes.CPU.Reset();
		}

#if DEBUG
		if (GetKey(olc::Key::SPACE).bPressed) m_Nes.ClockFullFrame();
		if (GetKey(olc::Key::C).bPressed)
		{
			cycles++;
			m_Nes.Clock(true);
		}
		if (GetKey(olc::Key::V).bPressed)
		{
			for (int i = 0; i < 32; i++)
			{
				cycles += 1;
				m_Nes.Clock(true);
			}
		}
		if (GetKey(olc::Key::G).bPressed)
		{
			for (int i = 0; i < 1000; i++)
			{
				cycles += 1;
				m_Nes.Clock(true);
			}
		}
		if (GetKey(olc::Key::L).bPressed)
		{
			m_EmulationMode = (m_EmulationMode + 1) % 2;
		}
		if (GetKey(olc::Key::P).bPressed)
		{
			m_PlayMode = (m_PlayMode + 1) % 2;
		}
		if (GetKey(olc::Key::S).bPressed)
		{
			m_SpriteMode = !m_SpriteMode;
		}
		if (GetKey(olc::Key::X).bPressed)
		{
			m_DisplayPpuMemory = !m_DisplayPpuMemory;
		}
		if (m_PlayMode == 1)
		{
			m_Nes.ClockFullFrame();
		}
		else if (m_EmulationMode == 1)
		{
			// Run it
			m_Nes.Clock(true);
		}
		if (GetKey(olc::Key::Q).bPressed)
		{
			// Cycle memory page up
			m_customMemoryDrawPage += 256;
		}
		if (GetKey(olc::Key::E).bPressed)
		{
			// Cycle memory page down
			m_customMemoryDrawPage -= 256;
		}
		if (GetKey(olc::Key::A).bPressed)
		{
			// Cycle memory page up
			m_customMemoryDrawPage += 256 * 16;
		}
		if (GetKey(olc::Key::D).bPressed)
		{
			// Cycle memory page down
			m_customMemoryDrawPage -= 256 * 16;
		}
		if (GetKey(olc::Key::I).bPressed)
		{
			m_Nes.CPU.MaskableInterrupt();
		}
		if (GetKey(olc::Key::N).bPressed)
		{
			m_Nes.CPU.NonMaskableInterrupt();
		}
		if (GetKey(olc::Key::M).bPressed)
		{
			m_RenderingMode = (m_RenderingMode + 1) % 2;
		}

		if (m_RenderingMode == 0)
		{
			// Debug render
			DrawRam(2, 2, 0x0000, 16, 16);
			DrawRam(2, 182, m_customMemoryDrawPage, 16, 16);
			DrawRam(2, 362, 0x8000, 16, 16);
		}
		else
		{
			DrawSprite(0, 0, &m_Nes.PPU.GetScreen(), 2, 0);

			if (true)
			{
				// Draw Palettes & Pattern Tables ==============================================
				const int nSwatchSize = 6;
				for (int p = 0; p < 8; p++) // For each palette
					for (int s = 0; s < 4; s++) // For each index
						FillRect(512 + 10 + p * (nSwatchSize * 5) + s * nSwatchSize, 350,
							nSwatchSize, nSwatchSize, m_Nes.PPU.GetColourFromPaletteRam(p, s));

				DrawSprite(512 + 10, 358, &m_Nes.PPU.GetPatternTable(0, 0), 1, 0);
				DrawSprite(580 + 64 + 10, 358, &m_Nes.PPU.GetPatternTable(1, 0), 1, 0);
			}
		}

		if (!m_SpriteMode)
		{
			DrawCode(512 + 10, 72, 26);
		}
		else
		{
			int maxSprites = 0;
			for (int i = 0; i < 64; i++)
			{
				if (maxSprites == 64) break;
				if (m_Nes.PPU.GetOAMSpriteX(i) == 0) continue;

				maxSprites += 1;

				std::string s = hex(i, 2) + ": (" + std::to_string(m_Nes.PPU.GetOAMSpriteX(i))
					+ ", " + std::to_string(m_Nes.PPU.GetOAMSpriteY(i)) + ") "
					+ "ID: " + hex(m_Nes.PPU.GetOAMSpriteId(i), 2) +
					+" AT: " + hex(m_Nes.PPU.GetOAMSpriteAttribute(i), 2);
				DrawString(516, 72 + maxSprites * 10, s);
			}
		}

		DrawCpu(512 + 10, 2);
		DrawString(10, 530, "SPACE = Step Instruction    R = RESET    I = IRQ    N = NMI");
#else
		m_Nes.ClockFullFrame();
		DrawSprite(0, 0, &m_Nes.PPU.GetScreen(), 2, 0);
#endif

		return true;
	}
};

int main()
{
	VisualOutput out;
#if DEBUG
	out.Construct(740+64+10, 600, 2, 2);
#else
	out.Construct(525, 512, 2, 2);
#endif
	out.Start();
	return 0;
}
