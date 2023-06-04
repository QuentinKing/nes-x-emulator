#pragma once

#include <iostream>
#include <sstream>

#include "MessageListener.h"

// Random class that does helps me debug stuff when I need it
class DebugListener : public MessageListener
{
public:
	DebugListener()
	{
		this->SubscribeToMessage(WM_CHAR);
		this->SubscribeToMessage(WM_LBUTTONDOWN);
	}

	void HandleMessage(UINT message, WPARAM wparam, LPARAM lparam, HWND hWnd) override
	{
		switch(message)
		{
		case WM_CHAR:
		{
			static std::string title;
			title.push_back(wparam);
			SetWindowText(hWnd, title.c_str());
			break;
		}
		case WM_LBUTTONDOWN:
			POINTS pt = MAKEPOINTS(lparam);
			std::ostringstream ss;
			ss << "(" << pt.x << " , " << pt.y << ")";
			SetWindowText(hWnd, ss.str().c_str());
			break;
		}
	}
};