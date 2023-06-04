#pragma once
#pragma comment(lib,"d3d11")

#include <d3d11.h>

#include "Window.h"

class DirectXManager
{
public:
	DirectXManager(Window& wnd);
	~DirectXManager() { ReleaseCOM(); }

	void RenderFrame(uint32_t* pixelData);

private:
	void SetupAndCreateDevices(Window& wnd);
	void ReleaseCOM();

	ID3D11VertexShader* pVS;
	ID3D11PixelShader* pPS;

	ID3D11RenderTargetView* pCurrentTarget;

	ID3D11Buffer* pVBuffer;

	ID3D11InputLayout* pLayout;

	ID3D11Texture2D* tex;
	ID3D11ShaderResourceView* texSRV;
	ID3D11SamplerState* sampler;

	ID3D11Device* pDevice;
	IDXGISwapChain* pSwap;
	ID3D11DeviceContext* pContext;

};