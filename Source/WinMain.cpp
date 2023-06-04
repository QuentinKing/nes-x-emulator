#pragma comment(lib,"d3d11.lib")

#include "WindowsWrapper.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <d3d11.h>
#include <memory>
#include <chrono>

#include "WindowsMessageMap.h"
#include "Window.h"
#include "InputState.h"
#include "DebugListener.h"
#include "DirectXManager.h"
#include "NES.h"

#include "../resource.h"

#define HInstance() GetModuleHandle(NULL)

INT WindowHeight;
INT WindowWidth;

HICON hIcon;

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
	/*
	Set up application window 
	*/
	WindowWidth = 1080;
	WindowHeight = 1013;
	hIcon = LoadIcon(HInstance(), MAKEINTRESOURCE(IDI_MAINICON));
	Window window(HInstance(), hIcon, WindowWidth, WindowHeight, "NesXMain", "Nes X Emulator");
	if (!window.CreateAndDisplayWindow())
	{
		// Couldn't create or show window 
		return -1;
	}


	/* 
	Set up our window listeners
	*/
	std::unique_ptr<InputState> inputState(new InputState());
	window.AddListener(inputState.get());


	/*
	Create our graphics (DirectX) manager 
	*/
	std::unique_ptr<DirectXManager> graphicsManager(new DirectXManager(window));


	/*
	Set up emulation components
	*/
	NES nes;
	nes.PowerOn();

	std::shared_ptr<GameCartridge> game = std::make_shared<GameCartridge>();
	game->LoadRomFromFile("Q:/Coding/ROMs/ebike.nes");
	nes.LoadGameCartridge(*game);

	nes.CPU.Reset();


	/*
	Application Loop
	*/
	std::chrono::steady_clock::time_point m_prevFrameStart;
	while (true)
	{
		std::chrono::steady_clock::time_point curTime = std::chrono::high_resolution_clock::now();
		long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - m_prevFrameStart).count();
		if (duration < 16.66) continue; // Throttle for next frame to ensure 60 fps

		m_prevFrameStart = std::chrono::high_resolution_clock::now();

		// Flush out all window messages / get input
		MSG msg = { 0 };
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return msg.wParam;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Pass input state into our emulator
		uint8_t input = 0x00;
		if (inputState->IsRightButtonDown()) { input |= 0x01; }
		if (inputState->IsLeftButtonDown()) { input |= 0x02; }
		if (inputState->IsDownButtonDown()) { input |= 0x04; }
		if (inputState->IsUpButtonDown()) { input |= 0x08; }
		if (inputState->IsStartButtonDown()) { input |= 0x10; } // Start
		if (inputState->IsSelectButtonDown()) { input |= 0x20; } // Select
		if (inputState->IsBButtonDown()) { input |= 0x40; } // B
		if (inputState->IsAButtonDown()) { input |= 0x80; } // A
		nes.SetFirstControllerState(input);
		
		// TODO: Handle second player input
		// nes.SetSecondControllerState(0x00);

		// Run a frame of emulation
		nes.ClockFullFrame();

		// Spit out to result to our graphics manager and render the frame
		NesColor* screen = nes.PPU.GetScreenBuffer();
		graphicsManager->RenderFrame(reinterpret_cast<uint32_t*>(screen));
	}

	return 0;
}