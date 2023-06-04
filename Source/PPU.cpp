#include "PPU.h"
#include "NES.h"

PPU::PPU()
{
	nesColors[0x00] = NesColor(84, 84, 84);
	nesColors[0x01] = NesColor(0, 30, 116);
	nesColors[0x02] = NesColor(8, 16, 144);
	nesColors[0x03] = NesColor(48, 0, 136);
	nesColors[0x04] = NesColor(68, 0, 100);
	nesColors[0x05] = NesColor(92, 0, 48);
	nesColors[0x06] = NesColor(84, 4, 0);
	nesColors[0x07] = NesColor(60, 24, 0);
	nesColors[0x08] = NesColor(32, 42, 0);
	nesColors[0x09] = NesColor(8, 58, 0);
	nesColors[0x0A] = NesColor(0, 64, 0);
	nesColors[0x0B] = NesColor(0, 60, 0);
	nesColors[0x0C] = NesColor(0, 50, 60);
	nesColors[0x0D] = NesColor(0, 0, 0);
	nesColors[0x0E] = NesColor(0, 0, 0);
	nesColors[0x0F] = NesColor(0, 0, 0);
	nesColors[0x10] = NesColor(152, 150, 152);
	nesColors[0x11] = NesColor(8, 76, 196);
	nesColors[0x12] = NesColor(48, 50, 236);
	nesColors[0x13] = NesColor(92, 30, 228);
	nesColors[0x14] = NesColor(136, 20, 176);
	nesColors[0x15] = NesColor(160, 20, 100);
	nesColors[0x16] = NesColor(152, 34, 32);
	nesColors[0x17] = NesColor(120, 60, 0);
	nesColors[0x18] = NesColor(84, 90, 0);
	nesColors[0x19] = NesColor(40, 114, 0);
	nesColors[0x1A] = NesColor(8, 124, 0);
	nesColors[0x1B] = NesColor(0, 118, 40);
	nesColors[0x1C] = NesColor(0, 102, 120);
	nesColors[0x1D] = NesColor(0, 0, 0);
	nesColors[0x1E] = NesColor(0, 0, 0);
	nesColors[0x1F] = NesColor(0, 0, 0);
	nesColors[0x20] = NesColor(236, 238, 236);
	nesColors[0x21] = NesColor(76, 154, 236);
	nesColors[0x22] = NesColor(120, 124, 236);
	nesColors[0x23] = NesColor(176, 98, 236);
	nesColors[0x24] = NesColor(228, 84, 236);
	nesColors[0x25] = NesColor(236, 88, 180);
	nesColors[0x26] = NesColor(236, 106, 100);
	nesColors[0x27] = NesColor(212, 136, 32);
	nesColors[0x28] = NesColor(160, 170, 0);
	nesColors[0x29] = NesColor(116, 196, 0);
	nesColors[0x2A] = NesColor(76, 208, 32);
	nesColors[0x2B] = NesColor(56, 204, 108);
	nesColors[0x2C] = NesColor(56, 180, 204);
	nesColors[0x2D] = NesColor(60, 60, 60);
	nesColors[0x2E] = NesColor(0, 0, 0);
	nesColors[0x2F] = NesColor(0, 0, 0);
	nesColors[0x30] = NesColor(236, 238, 236);
	nesColors[0x31] = NesColor(168, 204, 236);
	nesColors[0x32] = NesColor(188, 188, 236);
	nesColors[0x33] = NesColor(212, 178, 236);
	nesColors[0x34] = NesColor(236, 174, 236);
	nesColors[0x35] = NesColor(236, 174, 212);
	nesColors[0x36] = NesColor(236, 180, 176);
	nesColors[0x37] = NesColor(228, 196, 144);
	nesColors[0x38] = NesColor(204, 210, 120);
	nesColors[0x39] = NesColor(180, 222, 120);
	nesColors[0x3A] = NesColor(168, 226, 144);
	nesColors[0x3B] = NesColor(152, 226, 180);
	nesColors[0x3C] = NesColor(160, 214, 228);
	nesColors[0x3D] = NesColor(160, 162, 160);
	nesColors[0x3E] = NesColor(0, 0, 0);
	nesColors[0x3F] = NesColor(0, 0, 0);
}

PPU::~PPU()
{

}

void PPU::Initialize(NES* console)
{
	m_NES = console;
}

void PPU::Reset()
{

}

uint16_t PPU::GetPPUIOAddress()
{
	return m_PpuAddress;
}

uint16_t PPU::GetPPULatchAddress()
{
	return m_LatchAddress;
}

void PPU::WriteRegister(uint16_t address, uint8_t data)
{
	uint8_t temp = 0;
	switch (address & 0x07)
	{
	case 0x0000:
		HardSetPPUControlRegister(data);
		m_Temp_NameTableX = (data & 0x01);
		m_Temp_NameTableY = (data & 0x02) >> 1;
		break;
	case 0x0001:
		HardSetPPUMask(data);
		break;
	case 0x0002:
		// Status register is read-only
		break;
	case 0x0003:
		HardSetOAMAddress(data);
		break;
	case 0x0004:
		OAMMemory[m_OAMAddress] = data;
		break;
	case 0x0005:
		if (!latch)
		{
			m_xScroll = data;
		}
		else
		{
			m_yScroll = data;
		}
		latch = !latch;
		break;
	case 0x0006:
		if (!latch)
		{
			m_LatchAddress = (m_PpuAddress & 0x00FF) | (data << 8);
			m_Temp_NameTableX = m_LatchAddress & 0x0400;
			m_Temp_NameTableY = m_LatchAddress & 0x0800;
		}
		else
		{
			m_LatchAddress = (m_LatchAddress & 0xFF00) | data;
			m_PpuAddress = m_LatchAddress;
		}
		latch = !latch;
		break;
	case 0x0007:
		m_NES->WritePPUMemory(m_PpuAddress, data);
		if (GetPPUControlVRAMIncrementFlag())
		{
			m_PpuAddress += 32;
		}
		else
		{
			m_PpuAddress += 1;
		}
		break;
	}
}


uint8_t PPU::PeekRegister(uint16_t address)
{
	uint8_t data = 0;
	switch (address & 0x07)
	{
	case 0x0000:
		data = m_PPUControlRegister;
		break;
	case 0x0001:
		data = m_PPUMask;
		break;
	case 0x0002:
		data = m_PPUStatus;
		break;
	case 0x0003:
		// Write only
		break;
	case 0x0004:
		data = OAMMemory[m_OAMAddress];
		break;
	case 0x0005:
		if (!latch) data = m_xScroll; else data = m_yScroll;
		break;
	case 0x0006:
		break;
	case 0x0007:
		data = m_VRAMIORegister;
		break;
	}
	return data;
}

// Has side effects! Don't call unless the actual NES hardware would. Otherwise, for debugging or whatever, use PPU:PeekRegister
uint8_t PPU::GetRegister(uint16_t address)
{
	uint8_t data = 0;
	switch (address & 0x07)
	{
	case 0x0000:
		data = m_PPUControlRegister;
		break;
	case 0x0001:
		data = m_PPUMask;
		break;
	case 0x0002:
		data = m_PPUStatus;
		SetStatusVerticalBlank(false);
		latch = false;
		break;
	case 0x0003:
		// Write only
		break;
	case 0x0004:
		data = OAMMemory[m_OAMAddress];
		break;
	case 0x0005:
		if (!latch) data = m_xScroll; else data = m_yScroll;
		break;
	case 0x0006:
		break;
	case 0x0007:
		data = m_VRAMIORegister;
		m_VRAMIORegister = m_NES->ReadPPUMemory(m_PpuAddress);
		if (m_PpuAddress >= 0x3F00 && m_PpuAddress < 0x3F20) data = m_VRAMIORegister; // TODO: consider mirrors

		if (GetPPUControlVRAMIncrementFlag())
		{
			m_PpuAddress += 32;
		}
		else
		{
			m_PpuAddress += 1;
		}
		break;
	}

	return data;
}

void PPU::RenderPixel()
{
	bool backgroundOpaque = false;
	bool spriteZeroHit = false;

	if (m_curPixelColumn < 256 && m_curPixelRow >= 0 && m_curPixelRow < 240)
	{
		// Background Rendering
		if (GetPPUMaskShowBackground())
		{
			// Pixel / Tile lookup
			// Coarse
			uint8_t xTile = m_curPixelColumn / 8; // 0 - 31
			uint8_t yTile = m_curPixelRow / 8; // 0 - 29
	
			// Fine
			uint8_t xPixel = m_curPixelColumn % 8; // 0 - 7
			uint8_t yPixel = m_curPixelRow % 8; // 0 - 7

			uint16_t nameTableOffset = 0x0000;

			// Apply X Scroll
			uint8_t xTileScroll = m_xScroll / 8;
			uint8_t xPixelScroll = m_xScroll % 8;

			// Apply scroll within the pixel, if scrolling into the next tile, update the tile value
			xPixel += xPixelScroll;
			if (xPixel > 7)
			{
				// Scrolled into new tile
				xTile += 1;
				xPixel = xPixel % 8;
			}

			// Apply scroll of *tiles*, this may take us into a new nametable which will require update.
			xTile += xTileScroll;
			if (xTile > 31)
			{
				// Scrolled into new name table
				xTile = xTile - 32;
				nameTableOffset += 0x0400;
			}

			// Apply Y Scroll
			uint8_t yTileScroll = m_yScroll / 8;
			uint8_t yPixelScroll = m_yScroll % 8;

			// Apply scroll within the pixel, if scrolling into the next tile, update the tile value
			yPixel += yPixelScroll;
			if (yPixel > 7)
			{
				// Scrolled into new tile
				yTile += 1;
				yPixel = yPixel % 8;
			}

			// Apply scroll of *tiles*, this may take us into a new nametable which will require update.
			yTile += yTileScroll;
			if (yTile > 29)
			{
				// Scrolled into new name table
				yTile = yTile - 30;
				nameTableOffset += 0x0800;
			}

			// Name Table lookup
			uint8_t nameTableIndex = (m_Active_NameTableY << 1) | m_Active_NameTableX;
			uint16_t nameTableRoot = 0x2000 + nameTableIndex * 0x0400 + nameTableOffset;

			uint8_t val = m_NES->ReadPPUMemory(nameTableRoot + yTile * 32 + xTile);

			// Pattern lookup
			uint8_t tableId = GetPPUControlBackgroundPatternTable();
			uint8_t l = m_NES->ReadPPUMemory(tableId * 0x1000 + val * 16 + yPixel);
			uint8_t h = m_NES->ReadPPUMemory(tableId * 0x1000 + val * 16 + yPixel + 8);
			uint8_t bitLow = (l >> (7 - xPixel)) & 0x01;
			uint8_t bitHigh = (h >> (7 - xPixel)) & 0x01;

			// This is a value between 0-3 where 0 is transparent
			uint8_t pixelValue = (bitHigh << 1) | bitLow;

			// Palette lookup
			uint16_t attributeTableRoot = nameTableRoot + 0x03C0;
			uint8_t attributeRegionX = xTile / 4; // 0 - 7
			uint8_t attributeRegionY = yTile / 4; // 0 - 6
			uint8_t palette = m_NES->ReadPPUMemory(attributeTableRoot + attributeRegionX + attributeRegionY * 8);

			// Palette lookup is a byte of data containing the palettes for four 2x2 tiles
			uint8_t paletteTL = palette & 0x03;
			uint8_t paletteTR = (palette >> 2) & 0x03;
			uint8_t paletteBL = (palette >> 4) & 0x03;
			uint8_t paletteBR = (palette >> 6) & 0x03;

			// Determine which region our tile is and use that palette
			uint8_t paletteValue = 0;
			uint8_t isRight = ((xTile / 2) % 2 == 1);
			uint8_t isBottom = ((yTile / 2) % 2 == 1);
			if (isRight and isBottom) { paletteValue = paletteBR; }
			if (isRight and !isBottom) { paletteValue = paletteTR; }
			if (!isRight and isBottom) { paletteValue = paletteBL; }
			if (!isRight and !isBottom) { paletteValue = paletteTL; }

			if (pixelValue == 0x00)
			{
				uint8_t data = m_NES->ReadPPUMemory(0x3F00);

				screen[m_curPixelRow * 256 + m_curPixelColumn] = nesColors[data];
			}
			else
			{
				uint8_t data = m_NES->ReadPPUMemory(0x3F00 + (paletteValue << 2) + pixelValue);

				screen[m_curPixelRow * 256 + m_curPixelColumn] = nesColors[data];
			}

			backgroundOpaque = pixelValue != 0x00;
		}
	}

	if (m_curPixelColumn < 256 && m_curPixelRow >= 0 && m_curPixelRow < 240)
	{
		if (GetPPUMaskShowSprites())
		{
			// Foreground Rendering
			for (int i = 0; i < m_activeSprites; i++)
			{
				uint8_t x = GetActiveOAMSpriteX(i);

				uint8_t spritePalette = GetActiveOAMSpritePalette(i);
				uint8_t spritePriority = GetActiveOAMSpritePriority(i);
				uint8_t spriteFlipHorizontally = GetActiveOAMSpriteFlipHorizontal(i);
				uint8_t spriteFlipVertically = GetActiveOAMSpriteFlipVertical(i);
				uint8_t spriteTileId = GetActiveOAMSpriteId(i);

				if (GetPPUControlSpriteSize())
				{
					// 8x16 Sprite Mode
					// TODO
				}
				else
				{
					// 8x8 Sprite Mode
					uint8_t tableId = GetPPUControlForegroundPatternTable();

					if (m_curPixelColumn >= x && m_curPixelColumn - x < 8)
					{
						// Render!
						uint8_t xPixel = spriteFlipHorizontally ? 7 - (m_curPixelColumn - x) : m_curPixelColumn - x; // 0 - 7

						uint8_t l = OAMActiveSpriteLow[i];
						uint8_t h = OAMActiveSpriteHigh[i];
						uint8_t bitLow = (l >> (7 - xPixel)) & 0x01;
						uint8_t bitHigh = (h >> (7 - xPixel)) & 0x01;
						uint8_t pixelValue = (bitHigh << 1) | bitLow;

						if (pixelValue != 0x00) // If not transparent
						{
							// Because the top text flickers after scrolling, it causes the sprite hit not to register sometimes (it's hitting a blank background)
							if (backgroundOpaque && m_OAMActiveContainsSpriteZero && (i == 0))
							{
								SetStatusSpriteHit(true);
							}

							if (!spritePriority || !backgroundOpaque) // Otherwise, background has priority
							{
								uint8_t data = m_NES->ReadPPUMemory(0x3F00 + (spritePalette << 2) + pixelValue);

								screen[m_curPixelRow * 256 + m_curPixelColumn] = nesColors[data];
								break;
							}
						}
					}
				}
			}
		}
	}
}

void PPU::Cycle()
{
	m_completeFrame = false;

	// Pre render line, clear flags
	if (m_curPixelRow == 261 && m_curPixelColumn == 1)
	{
		SetStatusVerticalBlank(false);
		SetStatusSpriteHit(false);
		SetStatusOverflow(false);
	}

	// Pre render line, update active vertical name table each of these ticks
	if (m_curPixelRow == 261 && m_curPixelColumn >= 280 && m_curPixelColumn <= 304)
	{
		if (GetPPUMaskShowBackground() || GetPPUMaskShowSprites())
		{
			m_Active_NameTableY = m_Temp_NameTableY;
		}
	}

	// Now in first render line
	if (m_curPixelRow == 261 && m_curPixelColumn == 339)
	{
		m_curPixelRow = 0;
		m_curPixelColumn = 0;
	}

	// Update active horizontal name table
	if (m_curPixelColumn == 257)
	{
		if (GetPPUMaskShowBackground() || GetPPUMaskShowSprites())
		{
			m_Active_NameTableX = m_Temp_NameTableX;
		}
	}

	// Evaluate sprites for next line
	// TODO: Timing is a bit off here, should happen at column 65 and set data for the next row
	if ((m_curPixelRow <= 239 || m_curPixelRow == 261) && m_curPixelColumn == 340)
	{
		// Clear and check sprites for OAM memory
		for (int i = 0; i < 32; i++)
		{
			OAMActiveMemory[i] = 0x00;
		}
		for (int i = 0; i < 8; i++)
		{
			OAMActiveSpriteLow[i] = 0x00;
			OAMActiveSpriteHigh[i] = 0x00;
		}
		m_OAMActiveContainsSpriteZero = false;
		m_activeSprites = 0;

		for (int i = 0; i < 64; i++)
		{
			if (m_activeSprites >= 8) break;

			uint8_t y = GetOAMSpriteY(i)+1;

			if ((m_curPixelRow+1) >= y && (m_curPixelRow+1) - y < 8)
			{
				int j = m_activeSprites * 4;
				OAMActiveMemory[j] = y;
				OAMActiveMemory[j + 1] = GetOAMSpriteId(i);
 				OAMActiveMemory[j + 2] = GetOAMSpriteAttribute(i);
				OAMActiveMemory[j + 3] = GetOAMSpriteX(i);

				uint8_t yPixel = GetOAMSpriteFlipVertical(i) ? 7 - ((m_curPixelRow+1) - y) : (m_curPixelRow+1) - y; // 0 - 7
				uint8_t tableId = GetPPUControlForegroundPatternTable();
				uint8_t l = m_NES->ReadPPUMemory(tableId * 0x1000 + GetOAMSpriteId(i) * 16 + yPixel);
				uint8_t h = m_NES->ReadPPUMemory(tableId * 0x1000 + GetOAMSpriteId(i) * 16 + yPixel + 8);
				OAMActiveSpriteLow[m_activeSprites] = l;
				OAMActiveSpriteHigh[m_activeSprites] = h;

				m_activeSprites += 1;
	
				// Is sprite zero active? If it is we need to check for sprite 0 hits
				if (i == 0)
				{
					m_OAMActiveContainsSpriteZero = true;
				}
			}
		}
	}

	// Notify of vertical blank (end of visible frame)
	if (m_curPixelRow == 241 && m_curPixelColumn == 1)
	{
		SetStatusVerticalBlank(true);
		if (GetPPUControlNMIFlag())
		{
			m_NES->RequestNMI();
		}
	}

	// Render the pixel if we are in the visible frame
	if (!GetStatusVerticalBlank())
	{
		RenderPixel();
	}

	m_curPixelColumn++;
	if (m_curPixelColumn >= 341)
	{
		m_curPixelColumn = 0;
		m_curPixelRow++;
		if (m_curPixelRow == 261)
		{
			// Just completed a frame, now in pre-render line
			m_completeFrame = true;
		}
	}
}

bool PPU::IsFrameComplete()
{
	return m_completeFrame;
}