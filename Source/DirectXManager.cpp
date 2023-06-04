#include "WindowsWrapper.h"

#include "DirectXManager.h"
#include "Window.h"
#include "ShaderStructs.h"

#include <d3d11.h>
#include <d3dcompiler.h>


#pragma comment(lib, "D3DCompiler.lib")


DirectXManager::DirectXManager(Window& wnd)
{
	SetupAndCreateDevices(wnd);
}

void DirectXManager::SetupAndCreateDevices(Window& wnd)
{
	/* DirectX Setup */
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1; // One back buffer
	sd.OutputWindow = wnd.GetHwnd();
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	HRESULT hResult = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwap, &pDevice, nullptr, &pContext);


	// Set up our back buffer as our render target
	ID3D11Resource* pBackBuffer = nullptr;
	pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&pBackBuffer)); // Get 0th index back buffer
	pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pCurrentTarget);
	pContext->OMSetRenderTargets(1, &pCurrentTarget, NULL);
	pBackBuffer->Release();


	// Setup the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = wnd.GetRenderWidth();
	viewport.Height = wnd.GetRenderHeight();

	pContext->RSSetViewports(1, &viewport);

	// Set up shader (Just the one for this project so it's fine to store here imo)
	ID3DBlob* pVSBlob;
	ID3DBlob* pPSBlob;
	D3DReadFileToBlob(L"CompiledShaders/VertShader.cso", &pVSBlob);
	D3DReadFileToBlob(L"CompiledShaders/FragShader.cso", &pPSBlob);
	pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVS);
	pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPS);

	// Set up vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	pDevice->CreateBuffer(&bd, NULL, &pVBuffer);

	VERTEX OurVertices[] =
	{
		{1.0f, 1.0f, 0.0f,     1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f},
		{1.0f, -1.0f, 0.0f,    0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f},
		{-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,   0.0f, 1.0f},

		{1.0f, 1.0f, 0.0f,     1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
		{-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f},
		{-1.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f},
	};

	D3D11_MAPPED_SUBRESOURCE ms;
	pContext->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, OurVertices, sizeof(OurVertices));
	pContext->Unmap(pVBuffer, NULL);

	// Set up our layout
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"UV",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	pDevice->CreateInputLayout(ied, 3, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pLayout);

	pVSBlob->Release();
	pPSBlob->Release();


	// Create the main texture
	// bottom left to top right
	static const uint32_t s_pixel[] = {0x00ff0000, 0x00ffffff, 0x0000ff00, 0x00ff00ff}; // ABGR -> its backwards, so much confusion LOL
	D3D11_SUBRESOURCE_DATA initData = { &s_pixel, sizeof(uint32_t) * 2, 0 }; // 2nd argument is bytes until next line

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 2;
	desc.Height = 2; 
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	pDevice->CreateTexture2D(&desc, &initData, &tex);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	pDevice->CreateShaderResourceView(tex, &SRVDesc, &texSRV);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	pDevice->CreateSamplerState(&samplerDesc, &sampler);
}

void DirectXManager::RenderFrame(uint32_t* pixelData)
{
	// Clear buffer to solid color
	const float color[] = { 1.0f, 0.4f, 0.2f, 1.0f };
	pContext->ClearRenderTargetView(pCurrentTarget, color);

	// Render Emulation
	pContext->IASetInputLayout(pLayout);
	pContext->VSSetShader(pVS, 0, 0);
	pContext->PSSetShader(pPS, 0, 0);

	// TODO: Just update a shared texture resource rather than creating one every frame, but this works
	D3D11_SUBRESOURCE_DATA initData = { pixelData, sizeof(uint32_t) * 256, 0 }; // 2nd argument is bytes until next line
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 256;
	desc.Height = 240;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	pDevice->CreateTexture2D(&desc, &initData, &tex);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	pDevice->CreateShaderResourceView(tex, &SRVDesc, &texSRV);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pDevice->CreateSamplerState(&samplerDesc, &sampler);


	// Bind texture
	pContext->PSSetSamplers(0, 1, &sampler);
	pContext->PSSetShaderResources(0, 1, &texSRV);

	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Finally draw our result!
	pContext->Draw(6, 0);

	tex->Release();
	texSRV->Release();

	// Present screen
	pSwap->Present(1, 0);
}

void DirectXManager::ReleaseCOM()
{
	pVS->Release();
	pPS->Release();

	pVBuffer->Release();

	pCurrentTarget->Release();

	pDevice->Release();
	pSwap->Release();
	pContext->Release();
}