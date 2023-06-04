#pragma once 

#include "WindowsWrapper.h"

#include <unordered_map>

class WindowsMessageMap
{
public:
	WindowsMessageMap();
	std::string operator()(DWORD msd, LPARAM lp, WPARAM wp) const;

private:
	std::unordered_map<DWORD,std::string> map;
};