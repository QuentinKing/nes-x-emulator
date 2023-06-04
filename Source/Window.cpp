#include "WindowsWrapper.h"

#include "WindowsMessageMap.h"
#include "Window.h"

#include <sstream>
#include <vector>

std::vector<MessageListener*> Window::listeners;

Window::Window(HINSTANCE hInst, HICON hIcon, int width, int height, const char* name, const char* title)
	: hInst(hInst), hIcon(hIcon), name(name), width(width), height(height), title(title), hWnd(nullptr)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;

	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);

	wcex.hIcon = hIcon;
	wcex.hIconSm = hIcon;

	wcex.lpszClassName = name;
	wcex.lpszMenuName = nullptr;
	wcex.hInstance = hInst;
	wcex.lpfnWndProc = HandleMessage;

	RegisterClassEx(&wcex);
}

Window::~Window()
{
	UnregisterClass(name, GetModuleHandle(NULL));
}

bool Window::CreateAndDisplayWindow()
{
	// Fix client size
	RECT wr = { 0, 0, width, height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindowEx(NULL, name, title, WS_OVERLAPPEDWINDOW, 0, 0, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInst, NULL);
	if (!hWnd)
	{
		MessageBox(0, "Failed to create window", 0, 0);
		return false;
	}
	ShowWindow(hWnd, SW_SHOW);
	return true;
}

LRESULT CALLBACK Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static WindowsMessageMap mm;
	OutputDebugString(mm(msg, lparam, wparam).c_str());

	// Notify all listeners
	for (MessageListener* listener : listeners)
	{
		listener->NotifyMessage(msg, wparam, lparam, hWnd);
	}

	// Quit if closed
	switch (msg)
	{
	case WM_NCDESTROY:
		PostQuitMessage(1);
		break;
	}

	// Default to normal behaviour
	return DefWindowProc(hWnd, msg, wparam, lparam);
}



