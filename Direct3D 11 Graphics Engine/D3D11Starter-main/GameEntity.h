#pragma once

#include "Transform.h"
#include "Mesh.h"
#include <DirectXMath.h>
#include "Camera.h"
#include "Material.h"

class GameEntity
{
public:
	// Constructor & Destructor
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~GameEntity();
	
	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	std::shared_ptr<Transform> GetTransform(); // Shared pointer version
	//Transform* GetTransform() // Raw pointer version
	//Transform& GetTransform() // Reference version

	// Setters
	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMaterial(std::shared_ptr<Material> material);

	// Methods
	void Draw(std::shared_ptr<Camera> camera);

private:
	// Fields
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;
};

