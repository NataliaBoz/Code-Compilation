#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace Graphics
{
	// --- GLOBAL VARS ---

	// Primary D3D11 API objects on CPU
	inline Microsoft::WRL::ComPtr<ID3D11Device> Device; // Resource alloc. (Create stuff on GPU)
	inline Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context; // Changing render settings & work submission (Perform rendering tasks)
	inline Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain; // Presenting frames & double buffering (Swaps btw buffers)

	// Rendering buffers
	inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferRTV;
	inline Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthBufferDSV;

	// --- FUNCTIONS ---

	// Getters
	bool VsyncState();
	std::wstring APIName();

	// General functions
	HRESULT Initialize(unsigned int windowWidth, unsigned int windowHeight, HWND windowHandle, bool vsyncIfPossible);
	void ShutDown();
	void ResizeBuffers(unsigned int width, unsigned int height);

	// Debug Layer
	void PrintDebugMessages();
}