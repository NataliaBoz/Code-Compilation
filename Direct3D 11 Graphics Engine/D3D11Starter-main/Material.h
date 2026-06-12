#pragma once

#include <d3d11.h> //  Direct3D "stuff"
#include <memory>
#include "SimpleShader.h"
#include "Transform.h"
#include "Camera.h"
#include <unordered_map>

class Material
{
public:
	// Constructor
	Material(const char* name,
		DirectX::XMFLOAT4 colorTint,
		std::shared_ptr<SimpleVertexShader> vertShader, 
		std::shared_ptr<SimplePixelShader> pixShader, 
		DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1),
		DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0));

	// Getters
	const char* GetMaterialName();
	DirectX::XMFLOAT4 GetColorTint(); 
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> GetTextureSRVMap();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> GetSamplerMap(); 

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 tint);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixShader);
	void SetUVScale(DirectX::XMFLOAT2 uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 uvOffset); 

	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

private:
	// Fields
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertShader;
	std::shared_ptr<SimplePixelShader> pixShader;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	const char* name; // Name to make displaying in the UI easier

	// Hash tables to store textures & samplers
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

