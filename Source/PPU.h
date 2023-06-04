#pragma once

#include <cstdint>
#include <array>
#include <map>
#include <string>

class NES;

struct NesColor
{
	union
	{
		uint32_t n = 0;
		struct { uint8_t r; uint8_t g; uint8_t b; uint8_t a; };
	};

	NesColor() {}
	NesColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 0x00)
	{
		n = red | (green << 8) | (blue << 16) | (alpha << 24);
	}
};

class PPU
{
public:
	PPU();
	~PPU();

	void Cycle();
	void Initialize(NES* nes);
	void Reset();

	void WriteRegister(uint16_t address, uint8_t data);
	uint8_t PeekRegister(uint16_t address);
	uint8_t GetRegister(uint16_t address);

	inline void WriteOAMMemory(uint8_t address, uint8_t data) { OAMMemory[address] = data; }
	inline uint8_t ReadOAMMemory(uint8_t address) { return OAMMemory[address]; }

	inline uint8_t GetOAMSpriteY(int index) { return OAMMemory[index * 4]; }
	inline uint8_t GetOAMSpriteId(int index) { return OAMMemory[index * 4 + 1]; }
	inline uint8_t GetOAMSpriteAttribute(int index) { return OAMMemory[index * 4 + 2]; }
	inline uint8_t GetOAMSpriteX(int index) { return OAMMemory[index * 4 + 3]; }
	inline uint8_t GetOAMSpritePalette(int index) { return (OAMMemory[index * 4 + 2] & 0x03) + 0x04; }
	inline uint8_t GetOAMSpritePriority(int index) { return (OAMMemory[index * 4 + 2] & 0x20) >> 5; }
	inline uint8_t GetOAMSpriteFlipHorizontal(int index) { return (OAMMemory[index * 4 + 2] & 0x40) >> 6; }
	inline uint8_t GetOAMSpriteFlipVertical(int index) { return (OAMMemory[index * 4 + 2] & 0x80) >> 7; }
	inline uint8_t GetOAMSpriteTileId(int index) { return OAMMemory[index * 4 + 1] >> 7; }
	inline uint8_t GetOAMSpritePatternId(int index) { return OAMMemory[index * 4 + 1] & 0x01; }

	inline uint8_t GetActiveOAMSpriteY(int index) { return OAMActiveMemory[index * 4]; }
	inline uint8_t GetActiveOAMSpriteId(int index) { return OAMActiveMemory[index * 4 + 1]; }
	inline uint8_t GetActiveOAMSpriteAttribute(int index) { return OAMActiveMemory[index * 4 + 2]; }
	inline uint8_t GetActiveOAMSpriteX(int index) { return OAMActiveMemory[index * 4 + 3]; }
	inline uint8_t GetActiveOAMSpritePalette(int index) { return (OAMActiveMemory[index * 4 + 2] & 0x03) + 0x04; }
	inline uint8_t GetActiveOAMSpritePriority(int index) { return (OAMActiveMemory[index * 4 + 2] & 0x20) >> 5; }
	inline uint8_t GetActiveOAMSpriteFlipHorizontal(int index) { return (OAMActiveMemory[index * 4 + 2] & 0x40) >> 6; }
	inline uint8_t GetActiveOAMSpriteFlipVertical(int index) { return (OAMActiveMemory[index * 4 + 2] & 0x80) >> 7; }
	inline uint8_t GetActiveOAMSpriteTileId(int index) { return OAMActiveMemory[index * 4 + 1] >> 7; }
	inline uint8_t GetActiveOAMSpritePatternId(int index) { return OAMActiveMemory[index * 4 + 1] & 0x01; }

	uint16_t GetPPUIOAddress();
	uint16_t GetPPULatchAddress();

	bool IsFrameComplete();
	NesColor* GetScreenBuffer() { return screen; }

private:
	void RenderPixel();

	/* $2000 Register - PPUCTRL */
	// No idea what the master / slave bit does. Should usually be cleared though
	uint8_t m_PPUControlRegister = 0;
	inline void HardSetPPUControlRegister(uint8_t reg) { m_PPUControlRegister = reg; }
	inline bool GetPPUControlNMIFlag() { return (m_PPUControlRegister & 0b10000000) != 0; }
	//inline bool GetPPUControlMasterSlave() { return (m_PPUControlRegister & 0b01000000) != 0; }
	inline bool GetPPUControlSpriteSize() { return (m_PPUControlRegister & 0b00100000) != 0; }
	inline bool GetPPUControlBackgroundPatternTable() { return (m_PPUControlRegister & 0b00010000) != 0; }
	inline bool GetPPUControlForegroundPatternTable() { return (m_PPUControlRegister & 0b00001000) != 0; }
	inline bool GetPPUControlVRAMIncrementFlag() { return (m_PPUControlRegister & 0b00000100) != 0; }
	inline uint8_t GetPPUControlNameTable() { return m_PPUControlRegister & 0x03; }


	/* $2001 Register - PPU Mask */
	uint8_t m_PPUMask = 0;
	inline void HardSetPPUMask(uint8_t reg) { m_PPUMask = reg; }
	inline bool GetPPUMaskGrayscale() { return (m_PPUMask & 0x01) != 0x00; } // Not yet implemented
	inline bool GetPPUMaskShowLeftBackground() { return (m_PPUMask & 0x02) != 0x00; } // Not yet implemented
	inline bool GetPPUMaskShowLeftSprites() { return (m_PPUMask & 0x04) != 0x00; } // Not yet implemented
	inline bool GetPPUMaskShowBackground() { return (m_PPUMask & 0x08) != 0x00; }
	inline bool GetPPUMaskShowSprites() { return (m_PPUMask & 0x10) != 0x00; }
	inline bool GetPPUMaskEmphasizeRed() { return (m_PPUMask & 0x20) != 0x00; } // Not yet implemented
	inline bool GetPPUMaskEmphasizeGreen() { return (m_PPUMask & 0x40) != 0x00; } // Not yet implemented
	inline bool GetPPUMaskEmphasizeBlue() { return (m_PPUMask & 0x80) != 0x00; } // Not yet implemented

	/* $2002 Register - PPU Status */
	uint8_t m_PPUStatus = 0;
	inline void HardSetPPUStatus(uint8_t reg) { m_PPUStatus = reg; }
	inline void SetStatusVerticalBlank(bool on) { m_PPUStatus = on ? m_PPUStatus | 0b10000000 : m_PPUStatus & ~0b10000000; }
	inline void SetStatusSpriteHit(bool on) { m_PPUStatus = on ? m_PPUStatus | 0b01000000 : m_PPUStatus & ~0b01000000; }
	inline void SetStatusOverflow(bool on) { m_PPUStatus = on ? m_PPUStatus | 0b00100000 : m_PPUStatus & ~0b00100000; }
	inline bool GetStatusVerticalBlank() { return (m_PPUStatus & 0b10000000) != 0; }
	inline bool GetStatusSpriteHit() { return (m_PPUStatus & 0b01000000) != 0; }

	/* $2003 Register - OAM Address */
	uint8_t m_OAMAddress = 0x00;
	inline void HardSetOAMAddress(uint8_t reg) { m_OAMAddress = reg; }

	/* $2004 Register - OAM Data */
	std::array<uint8_t, 256> OAMMemory;
	std::array<uint8_t, 32> OAMActiveMemory;
	std::array<uint8_t, 8> OAMActiveSpriteLow;
	std::array<uint8_t, 8> OAMActiveSpriteHigh;
	bool m_OAMActiveContainsSpriteZero = false;
	int m_activeSprites = 0;

	/* $2005 Register - Nametable scroll */
	uint8_t m_xScroll;
	uint8_t m_yScroll;

	/* $2006 and $2007 Registers - PPU Addressing */
	uint8_t m_VRAMIORegister = 0;
	uint16_t m_LatchAddress = 0x0000;
	uint16_t m_PpuAddress = 0x0000;
	bool latch = false;

	NES  *m_NES = nullptr;
	int m_curPixelRow = 0;
	int m_curPixelColumn = 0;
	bool m_completeFrame = false;
	bool m_flagSpriteZeroHit = false;

	uint8_t m_Temp_NameTableX = 0;
	uint8_t m_Temp_NameTableY = 0;
	uint8_t m_Active_NameTableX = 0;
	uint8_t m_Active_NameTableY = 0;

	NesColor nesColors[0x40];
	NesColor screen[61440]; 
};
