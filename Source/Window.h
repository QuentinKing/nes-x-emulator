#pragma once

#include "WindowsWrapper.h"

#include <vector>

#include "MessageListener.h"

class Window
{
public:
	Window(HINSTANCE hInst, HICON hIcon, int width, int height, const char* name, const char* title);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool CreateAndDisplayWindow();

	HWND GetHwnd() const { return hWnd; }
	int GetRenderWidth() const { return width; }
	int GetRenderHeight() const { return height; }

	void AddListener(MessageListener* messageListener) { listeners.push_back(messageListener); }

private:
	static LRESULT CALLBACK HandleMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static std::vector<MessageListener*> listeners;

	HWND hWnd;
	HICON hIcon;
	HINSTANCE hInst;

	const char* name;
	const char* title;
	int width;
	int height;
};