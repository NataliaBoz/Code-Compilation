#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	this->mesh = mesh;
	this->material = material; 
	transform = std::make_shared<Transform>();
}

GameEntity::~GameEntity()
{

}

// Getters
std::shared_ptr<Mesh> GameEntity::GetMesh() { return mesh; }
std::shared_ptr<Material> GameEntity::GetMaterial() { return material; }
std::shared_ptr<Transform> GameEntity::GetTransform() { return transform; }

// Setters
void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }
void GameEntity::SetMaterial(std::shared_ptr<Material> material) { this->material = material; }

// Main drawing function
void GameEntity::Draw(std::shared_ptr<Camera> camera)
{
	// Activate which shaders are bound BEFORE drawing each entity
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();
	vs->SetShader();
	ps->SetShader();

	// Strings here MUST match variable
	vs->SetMatrix4x4("world", transform->GetWorldMatrix()); 
	vs->SetMatrix4x4("view", camera->GetView());			// names in your shader’s cbuffer!
	vs->SetMatrix4x4("projection", camera->GetProjection()); 
	vs->SetMatrix4x4("worldInvTransp", transform->GetWorldInverseTransposeMatrix());

	ps->SetFloat4("colorTint", material->GetColorTint());
	ps->SetFloat2("uvScale", material->GetUVScale());
	ps->SetFloat2("uvOffset", material->GetUVOffset());

	// Maps, memcpys, & unmaps struct
	vs->CopyAllBufferData(); // Copies data to GPU; CAN'T DRAW WITHOUT!
	ps->CopyAllBufferData();

	// Set the textures & sampler state
	for (auto& t : material->GetTextureSRVMap()) { ps->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : material->GetSamplerMap()) { ps->SetSamplerState(s.first.c_str(), s.second); }

	// Set correct vertex & index buffers
	mesh->SetAndDrawBuffers();
}
