#pragma once

#include "WindowsWrapper.h"

#include <vector>
#include <algorithm>

class MessageListener
{
protected:
	// Abstract
	virtual void HandleMessage(UINT message, WPARAM wparam, LPARAM lparam, HWND hWnd) = 0;

public:
	MessageListener() {};
	~MessageListener() {};

	void NotifyMessage(UINT message, WPARAM wparam, LPARAM lparam, HWND hWnd)
	{
		if (std::find(subscribedMessages.begin(), subscribedMessages.end(), message) != subscribedMessages.end())
		{
			HandleMessage(message, wparam, lparam, hWnd);
		}
	}

	void SubscribeToMessage(UINT message)
	{
		if (std::find(subscribedMessages.begin(), subscribedMessages.end(), message) == subscribedMessages.end())
		{
			subscribedMessages.push_back(message);
		}
	}

	void ClearSubscriptions()
	{
		subscribedMessages.clear();
	}

protected:
	std::vector<UINT> subscribedMessages;
};