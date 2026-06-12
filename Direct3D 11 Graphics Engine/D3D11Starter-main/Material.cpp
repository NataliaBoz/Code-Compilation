#include "Material.h"

Material::Material(const char* name,
	DirectX::XMFLOAT4 colorTint,
	std::shared_ptr<SimpleVertexShader> vertShader,
	std::shared_ptr<SimplePixelShader> pixShader,
	DirectX::XMFLOAT2 uvScale,
	DirectX::XMFLOAT2 uvOffset):
		name(name),
		colorTint(colorTint),
		vertShader(vertShader),
		pixShader(pixShader),
		uvScale(uvScale),
		uvOffset(uvOffset)
{
}

// Getters
const char* Material::GetMaterialName() { return name; }
DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vertShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixShader; }
DirectX::XMFLOAT2 Material::GetUVScale() { return uvScale; }
DirectX::XMFLOAT2 Material::GetUVOffset() { return uvOffset; }
std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> Material::GetTextureSRVMap() { return textureSRVs; }
std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> Material::GetSamplerMap() { return samplers; }

// Setters
void Material::SetColorTint(DirectX::XMFLOAT4 tint) { this->colorTint = tint; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertShader) { this->vertShader = vertShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixShader) { this->pixShader = pixShader; }
void Material::SetUVScale(DirectX::XMFLOAT2 uvScale) { this->uvScale = uvScale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 uvOffset) { this->uvOffset = uvOffset; }
void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv) { textureSRVs.insert({ name, srv }); }
void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler) { samplers.insert({ name, sampler }); }
