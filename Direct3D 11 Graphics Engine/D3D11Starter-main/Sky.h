#pragma once

#include <wrl/client.h> // For ComPtr
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"
#include <memory>

class Sky
{
public:
	// Constructor 
	Sky(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		std::shared_ptr<Mesh> skyMesh,
		std::shared_ptr<SimplePixelShader> skyPS,
		std::shared_ptr<SimpleVertexShader> skyVS,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOpts
	);
	// Deconstructor
	~Sky();
	void Draw(std::shared_ptr<Camera> camera);

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOpts;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthBuffCompType;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterOpts; // Drawing the object’s “inside”
	std::shared_ptr<Mesh> skyMesh; // Geometry to use when drawing the sky
	std::shared_ptr<SimplePixelShader> skyPS;
	std::shared_ptr<SimpleVertexShader> skyVS;

	// --------------------------------------------------------
	// Author: Chris Cascioli
	// Purpose: Creates a cube map on the GPU from 6 individual textures
	// 
	// - You are allowed to directly copy/paste this into your code base
	//   for assignments, given that you clearly cite that this is not
	//   code of your own design.
	//
	// - Note: This code assumes you’re putting the function in Sky.cpp, 
	//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
	//   ComPtr called “device”.  Make any adjustments necessary for
	//   your own implementation.
	// --------------------------------------------------------


	// --- HEADER ---

	// Helper for creating a cubemap from 6 individual textures
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
			const wchar_t* right,
			const wchar_t* left,
			const wchar_t* up,
			const wchar_t* down,
			const wchar_t* front,
			const wchar_t* back);
};

