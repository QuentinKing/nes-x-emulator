#include "InputState.h"

void InputState::HandleMessage(UINT message, WPARAM wparam, LPARAM lparam, HWND hWnd)
{
	switch (message)
	{
	case WM_KEYUP:
		keyStates[wparam] = false;
		break;
	case WM_KEYDOWN:
		keyStates[wparam] = true;
		break;
	}
}

bool InputState::IsLeftButtonDown() const
{
	return keyStates[VK_LEFT];
}

bool InputState::IsUpButtonDown() const
{
	return keyStates[VK_UP];
}

bool InputState::IsRightButtonDown() const
{
	return keyStates[VK_RIGHT];
}

bool InputState::IsDownButtonDown() const
{
	return keyStates[VK_DOWN];
}

bool InputState::IsAButtonDown() const
{
	return keyStates[0x5A]; // keyboard z
}

bool InputState::IsBButtonDown() const
{
	return keyStates[0x58]; // keyboard x
}

bool InputState::IsStartButtonDown() const
{
	return keyStates[0x53]; // keyboard s
}

bool InputState::IsSelectButtonDown() const
{
	return keyStates[0x41]; // keyboard s1
}