#include "GameCartridge.h"

#include <fstream>
#include <memory>

// Support iNES file format
void GameCartridge::LoadRomFromFile(std::string filePath)
{
    // Reference: https://www.nesdev.org/wiki/INES

    std::ifstream romFile;
    romFile.open(filePath.c_str(), std::ifstream::binary);

    if (romFile.is_open())
    {
        romFile.seekg(0, romFile.end);
        int length = romFile.tellg();
        romFile.seekg(0, romFile.beg);

        // Get Header
        const int kHeaderSize = 16;
        char header[kHeaderSize] = { 0 };
        romFile.read(header, kHeaderSize);
        ParseHeaderData(header);

        // Get trainer data
        if (mapperFlags1 & 0x04)
        {
            // Trainer flag set
            romFile.read(m_trainerData, 512);
        }
        else
        {
            // Zero out trainer data
            char* begin = m_trainerData;
            char* end = begin + sizeof(m_trainerData);
            std::fill(begin, end, 0);
        }

        prg.resize(prgRomSize * kPrgBlockSize);
        chr.resize(chrRomSize * kChrBlockSize);

        // Get Program Data
        romFile.read((char*)prg.data(), prg.size());

        // Get Character Data
        romFile.read((char*)chr.data(), chr.size());

        // Get PlayChoice inst-rom data

        // Get PlayChoice PROM data

        romFile.close();
    }
}

void GameCartridge::ParseHeaderData(char headerData[])
{
    std::copy(headerData, headerData + 4, headerConstant);
    prgRomSize = headerData[4];
    chrRomSize = headerData[5];
    mapperFlags1 = headerData[6];
    mapperFlags2 = headerData[7];
    prgRamSize = headerData[8];
    tvSystem = headerData[9];
    tvSystemPrgRam = headerData[10];
    std::copy(headerData + 11, headerData + 16, headerPadding);
}

/*
* 
* Do the address mapping here. (The mapper was hardware of the cartridge anyways??)
* Pass the mapped addresses to the NES when they need it.
* 
*/
std::vector<uint8_t> GameCartridge::GetPrgData()
{
    std::vector<uint8_t> data;
    data.resize(32768);

    for (int i = 0; i < data.size(); i++)
    {
        data[i] = prg[i % prg.size()];
    }

    return data;
}

std::vector<uint8_t> GameCartridge::GetChrData()
{
    std::vector<uint8_t> data;
    data.resize(16384);

    for (int i = 0; i < data.size(); i++)
    {
        data[i] = chr[i % chr.size()];
    }

    return data;
}
