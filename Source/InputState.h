#pragma once

#include "WindowsWrapper.h"
#include "MessageListener.h"

class InputState : public MessageListener
{
public:
	InputState()
	{
		this->SubscribeToMessage(WM_KEYDOWN);
		this->SubscribeToMessage(WM_KEYUP);

		for (int i = 0; i < 255; i++)
		{
			keyStates[i] = false;
		}
	}

	bool IsLeftButtonDown() const;
	bool IsRightButtonDown() const;
	bool IsUpButtonDown() const;
	bool IsDownButtonDown() const;
	bool IsAButtonDown() const;
	bool IsBButtonDown() const;
	bool IsStartButtonDown() const;
	bool IsSelectButtonDown() const;

	void HandleMessage(UINT message, WPARAM wparam, LPARAM lparam, HWND hWnd) override;

private:
	bool keyStates[255];
};