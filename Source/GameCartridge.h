#pragma once

#include <string>
#include <vector>

// Reference: https://www.nesdev.org/wiki/INES

// Wrapper class to parse .NES file format roms

class GameCartridge
{
public:
	void LoadRomFromFile(std::string filePath);

	inline uint8_t GetMirroringArrangement() { return mapperFlags1 & 0x01; }

	std::vector<uint8_t> GetPrgData();
	std::vector<uint8_t> GetChrData();

private:
	void ParseHeaderData(char headerData[]);

	const int kPrgBlockSize = 16384;
	const int kChrBlockSize = 8192;

	char headerConstant[4];
	uint8_t prgRomSize;
	uint8_t chrRomSize;
	uint8_t mapperFlags1;
	uint8_t mapperFlags2;
	uint8_t prgRamSize;
	uint8_t tvSystem;
	uint8_t tvSystemPrgRam;
	char headerPadding[5];

	char m_trainerData[512];

	std::vector<uint8_t> prg;
	std::vector<uint8_t> chr;
};