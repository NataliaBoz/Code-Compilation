#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "Mesh.h"
#include "Transform.h"
#include "GameEntity.h"
#include "Camera.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateGeometry(); 
	void RefreshImGui(float deltaTime);
	void BuildUI(std::vector<std::shared_ptr<Mesh>> meshes, 
		std::vector<std::shared_ptr<GameEntity>> entities,
		std::vector<std::shared_ptr<Camera>> cameraViews, 
		std::shared_ptr<Camera> &activeCamera);

	// Initialize UI variables 
	float color[4] = { 0.4f, 0.75f, 0.7f, 1.0f }; // Background color
	bool displayImGuiDemo = false; // Flag for window display button
	bool check = false; // Flag for checkbox (just for fun, for now)
	int number = 100; // Initial position of slider (100% has a purpose, possibly)
	//VertexShaderData dataToCopy{ DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		//DirectX::XMMATRIX()}; // Create the constant buffer struct for mesh tint & offset/world

	// Create a pointer to an array (or vector) of meshes to easily loop through for drawing and UI work
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Mesh pointer declarations
	//std::shared_ptr<Mesh> origTriangleMesh; 
	//std::shared_ptr<Mesh> rectangleMesh;
	//std::shared_ptr<Mesh> heartMesh;

	std::shared_ptr<Mesh> cubeMesh;
	std::shared_ptr<Mesh> cylinderMesh;
	std::shared_ptr<Mesh> helixMesh;
	std::shared_ptr<Mesh> quadMesh;
	std::shared_ptr<Mesh> doubleSidedQuadMesh;
	std::shared_ptr<Mesh> sphereMesh;
	std::shared_ptr<Mesh> torusMesh;

	// Create a list of shared pointers to the differnt cameras
	std::vector<std::shared_ptr<Material>> materials;

	/*std::shared_ptr<Material> cyanMaterial;
	std::shared_ptr<Material> magentaMaterial;
	std::shared_ptr<Material> greenYellowMaterial;
	std::shared_ptr<Material> uvsMaterial;
	std::shared_ptr<Material> normalsMaterial;
	std::shared_ptr<Material> customMaterial;*/

	std::shared_ptr<Material> arcadeFloorMaterial;
	std::shared_ptr<Material> blackTealMarbleMaterial;
	std::shared_ptr<Material> blueTravertineMaterial;
	std::shared_ptr<Material> deepBlueTravertineMaterial;
	std::shared_ptr<Material> comboMaterial;

	// Create a list of shared pointers to entities for drawing
	std::vector<std::shared_ptr<GameEntity>> entities;

	//std::shared_ptr<GameEntity> rgbTriangle;
	//std::shared_ptr<GameEntity> rectangle;
	//std::shared_ptr<GameEntity> heart;
	//std::shared_ptr<GameEntity> anotherRect;
	//std::shared_ptr<GameEntity> anotherHeart;

	// Create a list of shared pointers to the differnt cameras
	std::vector<std::shared_ptr<Camera>> cameraViews;

	std::shared_ptr<Camera> activeCamera;
	std::shared_ptr<Camera> frontCamera;
	std::shared_ptr<Camera> sideCamera;
};

