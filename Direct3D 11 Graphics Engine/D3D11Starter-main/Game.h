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
#include "Lights.h"
#include "Sky.h"

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
		std::shared_ptr<Camera>& activeCamera,
		std::vector<Light>& lights);//DirectX::XMFLOAT3& ambientTerm

	void CreateShadowMap();
	void RenderShadowMap();

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

	//std::shared_ptr<Material> arcadeFloorMaterial;
	std::shared_ptr<Material> blackTealMarbleMaterial;
	//std::shared_ptr<Material> blueTravertineMaterial;
	//std::shared_ptr<Material> deepBlueTravertineMaterial;
	std::shared_ptr<Material> woodDiagArrowsMaterial;
	std::shared_ptr<Material> smoothedRockMaterial;
	//std::shared_ptr<Material> comboMaterial;
	std::shared_ptr<Material> turquoiseRustedMetalMaterial;
	std::shared_ptr<Material> metalTilesMaterial;
	std::shared_ptr<Material> rustedPaintMaterial;
	std::shared_ptr<Material> bronzeMaterial;

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

	// Create a list of all the lights in the scene
	std::vector<Light> lights;
	Light directShadowedLight = {}; // Initialize with 0's

	// Simple approximation of global illumination: all the light that bounces around a scene
	// i.e. A single color that represents the minimum light level in a scene
	//DirectX::XMFLOAT3 ambientTerm; 

	// Shadows
	int shadowMapResolution;
	float lightProjectionSize;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	std::shared_ptr<SimpleVertexShader> shadowsVS;

	// Pointer to the sky box
	std::shared_ptr<Sky> skyBox;

	// Post Processes:
	// Helper functions for window resizing
	void ResizeRenderTargets(Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv, float rtScale = 1.0f);
	void ResizePPResources();

	// Resources that are shared among ALL post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> postProcSampler;
	std::shared_ptr<SimpleVertexShader> ppFullscrTriVS;

	// Resources that are tied to a particular post process:
	// Blur
	int blurRadius; 
	std::shared_ptr<SimplePixelShader> ppBoxBlurPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppBoxBlurRTV; // For rendering into internal texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppBoxBlurSRV; // For sampling from internal texture

	// Gaussian Blur PP for Bloom
	std::shared_ptr<SimplePixelShader> gaussianBlurPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> horizBlurRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> horizBlurSRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> verticBlurRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> verticBlurSRV;

	// Bloom
	float brightnessThreshold;
	float bloomIntensLvl;
	std::shared_ptr<SimplePixelShader> bloomExtractPS;
	std::shared_ptr<SimplePixelShader> bloomCombinePS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppBloomRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppBloomSRV; 
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppBloomExtractRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppBloomExtractSRV;
};

