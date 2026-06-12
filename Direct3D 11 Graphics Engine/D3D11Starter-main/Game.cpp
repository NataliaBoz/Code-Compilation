#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Transform.h"
#include "GameEntity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"

#include <DirectXMath.h>
#include <memory> // Smart Pointers

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "WICTextureLoader.h" // DirectXTK for loading textures

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();
	
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // (..._POINTLIST) (..._LINELIST) (..._LINESTRIP) (..._TRIANGLESTRIP) (..._TRIANGLEFAN)

		// Create the different cameras
		frontCamera = std::make_shared<Camera>(XMFLOAT3(0.0f, 0.5f, -10.0f), // Initial starting position
			4.5f, // Movement speed
			0.005f, // Look speed
			XM_PIDIV2, // Field of view
			Window::AspectRatio() // Aspect ratio
			// Using near clipping plane & far clipping plane defaults
		);

		sideCamera = std::make_shared<Camera>(XMFLOAT3(-3.0f, 3.0f, -3.0f), // Initial starting position
			3.0f, // Movement speed
			0.002f, // Look speed
			XM_PIDIV4, // Field of view
			Window::AspectRatio() // Aspect ratio
		// Using near clipping plane & far clipping plane defaults
		);
		sideCamera->GetTransform()->SetRotation(0.4f, 0.6f, 0.0f); // Set the view angle of the side camera

		// Set the activeCamera to the front view (cameraView[0])
		activeCamera = frontCamera;

		cameraViews.push_back(frontCamera);
		cameraViews.push_back(sideCamera);

		// Create *lights!*
		directShadowedLight.Type = LIGHT_TYPE_DIRECTIONAL;
		directShadowedLight.Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
		directShadowedLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
		directShadowedLight.Intensity = 1.0;

		Light directionalLight1 = {}; // Initialize with 0's
		directionalLight1.Type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight1.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f); // Uses Direction instead of Position
		directionalLight1.Color = XMFLOAT3(1.0f, 1.0f, 0.2f);
		directionalLight1.Intensity = 1.0;

		Light directionalLight2 = {}; 
		directionalLight2.Type = LIGHT_TYPE_DIRECTIONAL;
		//directionalLight2.Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
		directionalLight2.Direction = XMFLOAT3(1.0f, 0.0f, 1.0f);
		directionalLight2.Color = XMFLOAT3(0.2f, 1.0f, 1.0f);
		directionalLight2.Intensity = 1.0;

		Light directionalLight3 = {}; 
		directionalLight3.Type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight3.Direction = XMFLOAT3(0.5f, 0.5f, 1.0f);
		directionalLight3.Color = XMFLOAT3(1.0f, 0.2f, 1.0f);
		directionalLight3.Intensity = 1.0;

		Light pointLight1 = {};
		pointLight1.Type = LIGHT_TYPE_POINT;
		pointLight1.Position = XMFLOAT3(-2.0f, 2.0f, 2.0f);  // Uses Position instead of Direction
		pointLight1.Color = XMFLOAT3(0.5f, 0.5f, 0.5f);
		pointLight1.Intensity = 2.5;
		pointLight1.Range = 10.0f;

		Light spotLight1 = {};
		spotLight1.Type = LIGHT_TYPE_SPOT;
		spotLight1.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		spotLight1.Position = XMFLOAT3(3.0f, 1.75f, 0.0f);
		spotLight1.Color = XMFLOAT3(0.0f, 1.0f, 0.5f);
		spotLight1.Intensity = 1.25;
		spotLight1.Range = 10.0f;
		spotLight1.SpotOuterAngle = XMConvertToRadians(30.0f);
		spotLight1.SpotInnerAngle = XMConvertToRadians(15.0f);	

		// Add each light source to the list
		lights.push_back(directShadowedLight);
		lights.push_back(directionalLight1);
		lights.push_back(directionalLight2);
		lights.push_back(directionalLight3);
		lights.push_back(pointLight1);
		lights.push_back(spotLight1);
	}

	shadowMapResolution = 1024; // Power of 2 is best
	CreateShadowMap();

	blurRadius = 0;

	brightnessThreshold = 0.5f;
	bloomIntensLvl = 1.0f;
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Creates the geometry (mesh vertices, indices, shaders, textures, & materials) to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Load the SimpleShader objects
	std::shared_ptr<SimpleVertexShader> vertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> pixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());

	// UVs Pixel Shader
	//std::shared_ptr<SimplePixelShader> uvsPS = std::make_shared<SimplePixelShader>(
	//	Graphics::Device, Graphics::Context, FixPath(L"UVsPS.cso").c_str());
	//// Normals Pixel Shader
	//std::shared_ptr<SimplePixelShader> normalsPS = std::make_shared<SimplePixelShader>(
	//	Graphics::Device, Graphics::Context, FixPath(L"NormalsPS.cso").c_str());
	//// Custom Pixel Shader
	//std::shared_ptr<SimplePixelShader> customPS = std::make_shared<SimplePixelShader>(
	//	Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());
	//// Combining Textures Pixel Shader
	//std::shared_ptr<SimplePixelShader> combinePS = std::make_shared<SimplePixelShader>(
	//	Graphics::Device, Graphics::Context, FixPath(L"CombinePS.cso").c_str());

	// Sky box shaders
	std::shared_ptr<SimpleVertexShader> skyVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"SkyVS.cso").c_str());
	std::shared_ptr<SimplePixelShader> skyPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"SkyPS.cso").c_str());

	// Shadows Vertex Shader
	shadowsVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"ShadowMapVS.cso").c_str());

	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	/*
	// Set up the vertices of the original triangle to draw			
	Vertex triVertices[] = // 3 floats for position, 7 for color	
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};
	// Set up which vertices to use & in which order for original triangle
	unsigned int triIndices[] = { 0, 1, 2 };

	// Rectangle mesh's vertices & indices
	Vertex rectvertices[] =
	{
		{ XMFLOAT3(-0.6f, +0.15f, +1.0f), blue },
		{ XMFLOAT3(-0.5f, +0.15f, +1.0f), green },
		{ XMFLOAT3(-0.5f, -0.15f, +1.0f), blue },
		{ XMFLOAT3(-0.6f, -0.15f, +1.0f), red },
	};
	unsigned int rectIndices[] = { 0, 1, 2, 0, 2, 3 };

	// Rectangle mesh's vertices & indices
	Vertex heartvertices[] =
	{
		{ XMFLOAT3(+0.5f, +0.5f, +0.5f), red },
		{ XMFLOAT3(+0.6f, +0.6f, +0.5f), red },
		{ XMFLOAT3(+0.7f, +0.5f, +0.5f), red },
		{ XMFLOAT3(+0.5f, +0.2f, +0.5f), white },
		{ XMFLOAT3(+0.3f, +0.5f, +0.5f), red },
		{ XMFLOAT3(+0.4f, +0.6f, +0.5f), red },
	};
	unsigned int heartIndices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 4, 5, 0 };

	// Initialize pointers to each 2D mesh
	origTriangleMesh = std::make_shared<Mesh>(triVertices, triIndices, ARRAYSIZE(triVertices), ARRAYSIZE(triIndices));//, "RGBTriangle"); 
	rectangleMesh = std::make_shared<Mesh>(rectvertices, rectIndices, ARRAYSIZE(rectvertices), ARRAYSIZE(rectIndices));//, "Rectangle");
	heartMesh = std::make_shared<Mesh>(heartvertices, heartIndices, ARRAYSIZE(heartvertices), ARRAYSIZE(heartIndices)); //, "Heart");
	
	// Add each mesh to the list
	meshes.push_back(origTriangleMesh);
	meshes.push_back(rectangleMesh);
	meshes.push_back(heartMesh);

	// Create the pointers to each 2D entity with its mesh
	rgbTriangle = std::make_shared<GameEntity>(origTriangleMesh, cyanMaterial);
	rectangle = std::make_shared<GameEntity>(rectangleMesh, greenYellowMaterial);
	heart = std::make_shared<GameEntity>(heartMesh, magentaMaterial);
	anotherRect = std::make_shared<GameEntity>(rectangleMesh, cyanMaterial);
	anotherHeart = std::make_shared<GameEntity>(heartMesh, greenYellowMaterial); 
	
	// Add each entity to the list for drawing
	entities.push_back(rgbTriangle);
	entities.push_back(rectangle);
	entities.push_back(heart);
	entities.push_back(anotherRect);
	entities.push_back(anotherHeart);
	*/

	// Initialize pointers to each 3D mesh
	cubeMesh = std::make_shared<Mesh>("Cube", FixPath("../../Assets/Models/cube.obj").c_str());
	cylinderMesh = std::make_shared<Mesh>("Cylinder", FixPath("../../Assets/Models/cylinder.obj").c_str());
	helixMesh = std::make_shared<Mesh>("Helix", FixPath("../../Assets/Models/helix.obj").c_str());
	quadMesh = std::make_shared<Mesh>("Quad", FixPath("../../Assets/Models/quad.obj").c_str());
	doubleSidedQuadMesh = std::make_shared<Mesh>("Double-Sided Quad", FixPath("../../Assets/Models/quad_double_sided.obj").c_str());
	sphereMesh = std::make_shared<Mesh>("Sphere", FixPath("../../Assets/Models/sphere.obj").c_str());
	torusMesh = std::make_shared<Mesh>("Torus", FixPath("../../Assets/Models/torus.obj").c_str());

	// Add each mesh to the list
	meshes.push_back(cubeMesh);
	meshes.push_back(cylinderMesh);
	meshes.push_back(helixMesh);
	meshes.push_back(quadMesh);
	meshes.push_back(doubleSidedQuadMesh);
	meshes.push_back(sphereMesh);
	meshes.push_back(torusMesh);

	// Load textures
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> arcadeFloorSRV;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> arcadeFloorNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blackTealMarbleSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blackTealMarbleNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blackTealMarbleRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blackTealMarbleMetalness;

	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blueTravertineSRV;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blueTravertineNormalSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodDiagArrowsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodDiagArrowsNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodDiagArrowsRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodDiagArrowsMetalness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> smoothedRockSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> smoothedRockNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> smoothedRockRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> smoothedRockMetalness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> turquoiseRustedMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> turquoiseRustedMetalNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> turquoiseRustedMetalRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> turquoiseRustedMetalMetalness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalTilesSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalTilesNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalTilesRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalTilesMetalness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rustedPaintSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rustedPaintNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rustedPaintRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rustedPaintMetalness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetalness;

	// Repeat for EACH texture to load from file (PNG preferable, but JPG also works)
	// Arcade floor texture
	/*CreateWICTextureFromFile(Graphics::Device.Get(), // Graphics device
		Graphics::Context.Get(), // The context for auto MOP
		FixPath(L"../../Assets/Textures/ArcadeFloor.png").c_str(), // Texture
		0, // Not the actual texture object, could also use nullptr
		arcadeFloorSRV.GetAddressOf()); // Get SRV
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(), 
		Graphics::Context.Get(), 
		FixPath(L"../../Assets/Textures/Carpet016_1K-PNG_NormalDX.png").c_str(),
		0,
		arcadeFloorNormalSRV.GetAddressOf());*/

	// Black & teal marble texture
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Marble009_1K-PNG_Color.png").c_str(), 
		0, 
		blackTealMarbleSRV.GetAddressOf());
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Marble009_1K-PNG_NormalDX.png").c_str(),
		0,
		blackTealMarbleNormalSRV.GetAddressOf());
	// Roughness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Marble009_1K-PNG_Roughness.png").c_str(),
		0,
		blackTealMarbleRoughness.GetAddressOf());
	// Metalness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Marble009_1K-PNG_Metalness.png").c_str(),
		0,
		blackTealMarbleMetalness.GetAddressOf());

	// Light blue travertine texture
	/*CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Travertine013_1K-PNG_Color.png").c_str(),
		0,
		blueTravertineSRV.GetAddressOf());
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Marble009_1K-PNG_NormalDX.png").c_str(),
		0,
		blueTravertineNormalSRV.GetAddressOf());*/

	/* Wood floor(Diagonal arrows pattern) texture */
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/WoodFloor058_1K-PNG_Color.png").c_str(),
		0,
		woodDiagArrowsSRV.GetAddressOf());
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/WoodFloor058_1K-PNG_NormalDX.png").c_str(),
		0,
		woodDiagArrowsNormalSRV.GetAddressOf());
	// Roughness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/WoodFloor058_1K-PNG_Roughness.png").c_str(),
		0,
		woodDiagArrowsRoughness.GetAddressOf());
	// Metalness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/WoodFloor058_1K-PNG_Metalness.png").c_str(),
		0,
		woodDiagArrowsMetalness.GetAddressOf());

	/* Smoothed rock cliff texture */
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Rock015_1K-PNG_Color.png").c_str(),
		0,
		smoothedRockSRV.GetAddressOf());
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Rock015_1K-PNG_NormalDX.png").c_str(),
		0,
		smoothedRockNormalSRV.GetAddressOf());
	// Roughness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Rock015_1K-PNG_Roughness.png").c_str(),
		0,
		smoothedRockRoughness.GetAddressOf());
	// Metalness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Rock015_1K-PNG_Metalness.png").c_str(),
		0,
		smoothedRockMetalness.GetAddressOf());

	/* Metal with turquoise rust texture */
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Metal058C_1K-PNG_Color.png").c_str(),
		0,
		turquoiseRustedMetalSRV.GetAddressOf());
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Metal058C_1K-PNG_NormalDX.png").c_str(),
		0,
		turquoiseRustedMetalNormalSRV.GetAddressOf());
	// Roughness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Metal058C_1K-PNG_Roughness.png").c_str(),
		0,
		turquoiseRustedMetalRoughness.GetAddressOf());
	// Metalness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Metal058C_1K-PNG_Metalness.png").c_str(),
		0,
		turquoiseRustedMetalMetalness.GetAddressOf());

	/* Offset metal tiles texture */
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/MetalPlates008_1K-PNG_Color.png").c_str(),
		0,
		metalTilesSRV.GetAddressOf());
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/MetalPlates008_1K-PNG_NormalDX.png").c_str(),
		0,
		metalTilesNormalSRV.GetAddressOf());
	// Roughness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/MetalPlates008_1K-PNG_Roughness.png").c_str(),
		0,
		metalTilesRoughness.GetAddressOf());
	// Metalness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/MetalPlates008_1K-PNG_Metalness.png").c_str(),
		0,
		metalTilesMetalness.GetAddressOf());

	/* Teal-painted metal with rust spots texture */
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/PaintedMetal006_1K-PNG_Color.png").c_str(),
		0,
		rustedPaintSRV.GetAddressOf());
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/PaintedMetal006_1K-PNG_NormalDX.png").c_str(),
		0,
		rustedPaintNormalSRV.GetAddressOf());
	// Roughness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/PaintedMetal006_1K-PNG_Roughness.png").c_str(),
		0,
		rustedPaintRoughness.GetAddressOf());
	// Metalness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/PaintedMetal006_1K-PNG_Metalness.png").c_str(),
		0,
		rustedPaintMetalness.GetAddressOf());

	/* Bronze texture */
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_albedo.png").c_str(),
		0,
		bronzeSRV.GetAddressOf());
	// Normal map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_normals.png").c_str(),
		0,
		bronzeNormalSRV.GetAddressOf());
	// Roughness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_roughness.png").c_str(),
		0,
		bronzeRoughness.GetAddressOf());
	// Metalness map
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_metal.png").c_str(),
		0,
		bronzeMetalness.GetAddressOf());
		
	// Create a sampler
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	D3D11_SAMPLER_DESC sampleDescr{};
	sampleDescr.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Handle addresses outside 0-1 UV range
	sampleDescr.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDescr.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDescr.Filter = D3D11_FILTER_ANISOTROPIC; // Handle sampling btw pixels //D3D11_FILTER_MIN_MAG_MIP_LINEAR; D3D11_FILTER_MIN_MAG_MIP_POINT
	sampleDescr.MaxAnisotropy = 16;
	sampleDescr.MaxLOD = D3D11_FLOAT32_MAX; // Mipmapping
	Graphics::Device->CreateSamplerState(&sampleDescr, sampler.GetAddressOf());

	// Create pointers to each different material
	//cyanMaterial = std::make_shared<Material>(XMFLOAT4(0, 1, 1, 1), vertexShader, pixelShader, "Cyan"); 
	//magentaMaterial = std::make_shared<Material>(XMFLOAT4(1, 0, 1, 1), vertexShader, pixelShader, "Magenta"); 
	//greenYellowMaterial = std::make_shared<Material>(XMFLOAT4(0.65f, 1, 0, 1), vertexShader, pixelShader, "Green-Yellow"); 
	//uvsMaterial = std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vertexShader, uvsPS, "UVs"); 
	//normalsMaterial = std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vertexShader, normalsPS, "Normals"); 
	//customMaterial = std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vertexShader, customPS, "Custom"); // Fuzzy Bumblebee???

	// Textured materials (with normal maps)
	/*arcadeFloorMaterial = std::make_shared<Material>("Arcade Floor", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader, XMFLOAT2(2, 2));
	arcadeFloorMaterial->AddSampler("BasicSampler", sampler);
	arcadeFloorMaterial->AddTextureSRV("SurfaceTexture", arcadeFloorSRV);
	arcadeFloorMaterial->AddTextureSRV("NormalMap", arcadeFloorNormalSRV);*/

	blackTealMarbleMaterial = std::make_shared<Material>("Black & Teal Marble", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);
	blackTealMarbleMaterial->AddSampler("BasicSampler", sampler);
	blackTealMarbleMaterial->AddTextureSRV("SurfaceTexture", blackTealMarbleSRV);
	blackTealMarbleMaterial->AddTextureSRV("NormalMap", blackTealMarbleNormalSRV);
	blackTealMarbleMaterial->AddTextureSRV("RoughnessMap", blackTealMarbleRoughness);
	blackTealMarbleMaterial->AddTextureSRV("MetalnessMap", blackTealMarbleMetalness);

	/*blueTravertineMaterial = std::make_shared<Material>("Light Blue Travertine", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader, XMFLOAT2(0.5f, 0.5f));
	blueTravertineMaterial->AddSampler("BasicSampler", sampler);
	blueTravertineMaterial->AddTextureSRV("SurfaceTexture", blueTravertineSRV);
	blueTravertineMaterial->AddTextureSRV("NormalMap", blueTravertineNormalSRV);

	deepBlueTravertineMaterial = std::make_shared<Material>("Deep Blue Travertine", XMFLOAT4(0.25f, 0.35f, 1, 1), vertexShader, pixelShader);
	deepBlueTravertineMaterial->AddSampler("BasicSampler", sampler);
	deepBlueTravertineMaterial->AddTextureSRV("SurfaceTexture", blueTravertineSRV);
	deepBlueTravertineMaterial->AddTextureSRV("NormalMap", blueTravertineNormalSRV);*/

	woodDiagArrowsMaterial = std::make_shared<Material>("Diagonal Arrows Wood Pattern", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);
	woodDiagArrowsMaterial->AddSampler("BasicSampler", sampler);
	woodDiagArrowsMaterial->AddTextureSRV("SurfaceTexture", woodDiagArrowsSRV);
	woodDiagArrowsMaterial->AddTextureSRV("NormalMap", woodDiagArrowsNormalSRV);
	woodDiagArrowsMaterial->AddTextureSRV("RoughnessMap", woodDiagArrowsRoughness);
	woodDiagArrowsMaterial->AddTextureSRV("MetalnessMap", woodDiagArrowsMetalness);
	
	smoothedRockMaterial = std::make_shared<Material>("Smoothed Rock Cliff", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);
	smoothedRockMaterial->AddSampler("BasicSampler", sampler);
	smoothedRockMaterial->AddTextureSRV("SurfaceTexture", smoothedRockSRV);
	smoothedRockMaterial->AddTextureSRV("NormalMap", smoothedRockNormalSRV);
	smoothedRockMaterial->AddTextureSRV("RoughnessMap", smoothedRockRoughness);
	smoothedRockMaterial->AddTextureSRV("MetalnessMap", smoothedRockMetalness);
	
	// *NOTE: Currently NOT affected by lights & has NO normal map*
	/*comboMaterial = std::make_shared<Material>("Combination", XMFLOAT4(1, 1, 1, 1), vertexShader, combinePS);
	comboMaterial->AddSampler("BasicSampler", sampler);
	comboMaterial->AddTextureSRV("InitialTexture", blueTravertineSRV);
	comboMaterial->AddTextureSRV("CombineTexture", arcadeFloorSRV);*/

	turquoiseRustedMetalMaterial = std::make_shared<Material>("Metal With Turquoise Rust", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);
	turquoiseRustedMetalMaterial->AddSampler("BasicSampler", sampler);
	turquoiseRustedMetalMaterial->AddTextureSRV("SurfaceTexture", turquoiseRustedMetalSRV);
	turquoiseRustedMetalMaterial->AddTextureSRV("NormalMap", turquoiseRustedMetalNormalSRV);
	turquoiseRustedMetalMaterial->AddTextureSRV("RoughnessMap", turquoiseRustedMetalRoughness);
	turquoiseRustedMetalMaterial->AddTextureSRV("MetalnessMap", turquoiseRustedMetalMetalness);

	metalTilesMaterial = std::make_shared<Material>("Offset Metal Tiles", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);
	metalTilesMaterial->AddSampler("BasicSampler", sampler);
	metalTilesMaterial->AddTextureSRV("SurfaceTexture", metalTilesSRV);
	metalTilesMaterial->AddTextureSRV("NormalMap", metalTilesNormalSRV);
	metalTilesMaterial->AddTextureSRV("RoughnessMap", metalTilesRoughness);
	metalTilesMaterial->AddTextureSRV("MetalnessMap", metalTilesMetalness);

	rustedPaintMaterial = std::make_shared<Material>("Offset Metal Tiles", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);
	rustedPaintMaterial->AddSampler("BasicSampler", sampler);
	rustedPaintMaterial->AddTextureSRV("SurfaceTexture", rustedPaintSRV);
	rustedPaintMaterial->AddTextureSRV("NormalMap", rustedPaintNormalSRV);
	rustedPaintMaterial->AddTextureSRV("RoughnessMap", rustedPaintRoughness);
	rustedPaintMaterial->AddTextureSRV("MetalnessMap", rustedPaintMetalness);

	bronzeMaterial = std::make_shared<Material>("Bronze", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);
	bronzeMaterial->AddSampler("BasicSampler", sampler);
	bronzeMaterial->AddTextureSRV("SurfaceTexture", bronzeSRV);
	bronzeMaterial->AddTextureSRV("NormalMap", bronzeNormalSRV);
	bronzeMaterial->AddTextureSRV("RoughnessMap", bronzeRoughness);
	bronzeMaterial->AddTextureSRV("MetalnessMap", bronzeMetalness);

	// Put all the materials in a list
	/*materials.push_back(cyanMaterial);
	materials.push_back(magentaMaterial);
	materials.push_back(greenYellowMaterial);
	materials.push_back(uvsMaterial);
	materials.push_back(normalsMaterial);
	materials.push_back(customMaterial)*/;
	//materials.push_back(arcadeFloorMaterial);
	materials.push_back(blackTealMarbleMaterial);
	//materials.push_back(blueTravertineMaterial);
	//materials.push_back(deepBlueTravertineMaterial);
	materials.push_back(woodDiagArrowsMaterial);
	materials.push_back(smoothedRockMaterial);
	//materials.push_back(comboMaterial);
	materials.push_back(turquoiseRustedMetalMaterial);
	materials.push_back(metalTilesMaterial);
	materials.push_back(rustedPaintMaterial);
	materials.push_back(bronzeMaterial);

	// Create pointers to each 3D entity & add to the list for drawing
	entities.push_back(std::make_shared<GameEntity>(quadMesh, woodDiagArrowsMaterial));
	entities.push_back(std::make_shared<GameEntity>(cubeMesh, smoothedRockMaterial));
	entities.push_back(std::make_shared<GameEntity>(cylinderMesh, blackTealMarbleMaterial));
	entities.push_back(std::make_shared<GameEntity>(helixMesh, rustedPaintMaterial));
	entities.push_back(std::make_shared<GameEntity>(doubleSidedQuadMesh, turquoiseRustedMetalMaterial));
	entities.push_back(std::make_shared<GameEntity>(sphereMesh, metalTilesMaterial));
	entities.push_back(std::make_shared<GameEntity>(torusMesh, bronzeMaterial));

	// Resize the quadMesh "floor"
	entities[0]->GetTransform()->SetScale(12.0f, 1.0f, 12.0f);

	// Adjust the meshes' transforms to spread them out 
	for (int i = 1; i < entities.size(); i++)
	{
		entities[i]->GetTransform()->MoveAbsolute(float(-12 + 3.5 * i), 1.5f, 0); // Cast to a float to remove warning
	}

	// Lighting
	//ambientTerm = XMFLOAT3(0.43f, 0.40f, 0.43f); // A bit darker than the background

	// Create the sky box
	skyBox = std::make_shared<Sky>(
		FixPath(L"../../Assets/Textures/Clouds Pink/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Pink/back.png").c_str(),
		cubeMesh,
		skyPS,
		skyVS,
		sampler
		);

	// Post process shaders
	ppFullscrTriVS = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"FullscrTriangleVS.cso").c_str());

	ppBoxBlurPS = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"BoxBlurPS.cso").c_str());
	
	gaussianBlurPS = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"GaussianBlurPS.cso").c_str());

	bloomExtractPS = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"BloomPS_Extract.cso").c_str());
	bloomCombinePS = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"BloomPS_Combine.cso").c_str());

	// Create render targets (resizable if window changes)
	ResizePPResources();

	// Set up sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&ppSampDesc, postProcSampler.GetAddressOf());
}

// --------------------------------------------------------
// Creates the resources needed for the shadow map
// --------------------------------------------------------
void Game::CreateShadowMap()
{
	// Reset
	shadowDSV.Reset();
	shadowSRV.Reset();
	shadowSampler.Reset();
	shadowRasterizer.Reset();

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	// Create a rasterizer state for depth biasing
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Comparison sampler
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// View & project. matr. as tho the camera were seeing from the light
	XMMATRIX lightView = XMMatrixLookToLH(
		-XMVectorSet(0.0f, -1.0f, 1.0f, 0.0f) * 20, // Position: "Backing up" 20 units from origin
		XMVectorSet(0.0f, -1.0f, 1.0f, 0.0f), // Direction: light's direction
		XMVectorSet(0, 1, 0, 0)); // Up: World up vector (Y axis)
	XMStoreFloat4x4(&lightViewMatrix, lightView);

	lightProjectionSize = 20.0f; // Tweak based on scene!
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);
}

// Resize (release & recreate) Post Processing resources
void Game::ResizeRenderTargets(Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv, float rtScale)
{
	// Reset the RTVs & SRVs
	rtv.Reset();
	srv.Reset();
	
	// Describe the texture/render target being created
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (unsigned int)(Window::Width() * rtScale);
	textureDesc.Height = (unsigned int)(Window::Height() * rtScale);
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	Graphics::Device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		rtv.ReleaseAndGetAddressOf());
	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	Graphics::Device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		srv.ReleaseAndGetAddressOf());
}

// Resize all the Post Processes at once
void Game::ResizePPResources()
{
	// "Box" Blur
	ResizeRenderTargets(ppBoxBlurRTV, ppBoxBlurSRV);
	
	// Gaussian Blur
	ResizeRenderTargets(horizBlurRTV, horizBlurSRV, 0.5f);
	ResizeRenderTargets(verticBlurRTV, verticBlurSRV, 0.5f);
	
	// Bloom 
	ResizeRenderTargets(ppBloomRTV, ppBloomSRV);
	ResizeRenderTargets(ppBloomExtractRTV, ppBloomExtractSRV, 0.5f);
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for (auto& c : cameraViews)
	{
		if (c != NULL) // Make sure the camera exists first
		{
			// Keep the camera’s aspect ratio up to date with the window’s new aspect ratio
			c->UpdateProjectionMatrix(Window::AspectRatio());
		}
	}

	// Resize post process effects if window size changes
	if (Graphics::Device) { ResizePPResources(); }
}


// Helper method that is called from Game::Update() to update the ImGui UI
void Game::RefreshImGui(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	//ImGui::ShowDemoWindow();
	if (displayImGuiDemo)
	{
		ImGui::ShowDemoWindow();
	}
}	


// Helper method called in Game::Update() for UI-creation
void Game::BuildUI(std::vector<std::shared_ptr<Mesh>> meshes,
	std::vector<std::shared_ptr<GameEntity>> entities,
	std::vector<std::shared_ptr<Camera>> cameraViews,
	std::shared_ptr<Camera> &activeCamera,
	std::vector<Light> &lights)//DirectX::XMFLOAT3 &ambientTerm
{
	// Create the UI window
	ImGui::Begin("Inspector Window", 0, ImGuiWindowFlags_NoBackground);

	// Display the current framerate & window dimensions
	ImGui::Text("Frame Rate: %f fps", ImGui::GetIO().Framerate);
	ImGui::Text("Window Dimensions: %dx%d", Window::Width(), Window::Height());
	// Can create a 3 or 4-component color editors, too!
	// - Notice the two different function names below
	ImGui::ColorEdit4("Background Color", &color[0]);
	
	// Button to show/hide ImGui Demo
	if (ImGui::Button("ImGui Demo Window"))
	{
		displayImGuiDemo = !displayImGuiDemo;
	}

	// Checkbox
	ImGui::Checkbox("Check it!", &check);						/***** Make Hide/Show Sky Box??? *****/

	// Color Editor & Slider to adjust mesh tint & offset
	//ImGui::ColorEdit4("Mesh Tint", &dataToCopy.tint.x);
	//ImGui::SliderFloat3("Mesh Offset", &dataToCopy.offset.x, -1.0f, 1.0f);

	// Make a tab to display meshes info. 
	if (ImGui::CollapsingHeader("Meshes:"))
	{
		// Display the available meshes to inspect
		for (int i = 0; i < meshes.size(); i++) // ARRAYSIZE(meshes) // Or meshes.size()???
		{
			ImGui::PushID(meshes[i].get());

			if (ImGui::CollapsingHeader(meshes[i]->GetMeshName()))
			{
				ImGui::Text("Triangles: %u", meshes[i]->GetIndexCount() / 3);
				ImGui::Text("Vertices: %u", meshes[i]->GetVertexCount());
				ImGui::Text("Indices: %u", meshes[i]->GetIndexCount()); 
			}

			ImGui::PopID();
		}
	}

	// Make a tab to display all entities' transform data 
	if (ImGui::CollapsingHeader("Entities:"))
	{
		for (int i = 0; i < entities.size(); i++)
		{
			// Push unique internal ID to support multiple widgets with the same name
			ImGui::PushID(entities[i].get());

			if (ImGui::TreeNode("Node", "Entity %u", i))
			{
				ImGui::Text("Mesh Index Count: %u", entities[i]->GetMesh()->GetIndexCount());

				std::shared_ptr<Transform> entTransform = entities[i]->GetTransform(); 
				XMFLOAT3 entPosition = entTransform->GetPosition();
				XMFLOAT3 entRotation = entTransform->GetPitchYawRoll();
				XMFLOAT3 entScale = entTransform->GetScale();

				if (ImGui::DragFloat3("Position", &entPosition.x, 0.1f))
				{
					entTransform->SetPosition(entPosition);
				}

				if (ImGui::DragFloat3("Rotation (rad.)", &entRotation.x, 0.1f))
				{
					entTransform->SetRotation(entRotation);
				}

				if (ImGui::DragFloat3("Scale", &entScale.x, 0.1f))
				{
					entTransform->SetScale(entScale);
				}

				ImGui::TreePop();
			}

			ImGui::PopID();
		}
	}

	// Make a tab to display the available cameras to view from 
	if (ImGui::CollapsingHeader("Cameras:"))
	{
		// Push unique internal ID to support multiple widgets with the same name
		ImGui::PushID(cameraViews[0].get());
		if (ImGui::Selectable("Front Camera", false))
		{
			activeCamera = cameraViews[0];
		}
		ImGui::PopID();

		// Push unique internal ID to support multiple widgets with the same name
		ImGui::PushID(cameraViews[1].get());
		if (ImGui::Selectable("Side Camera", false))
		{
			activeCamera = cameraViews[1];
		}
		ImGui::PopID();
	}

	// Make a tab to display all the available materials
	if (ImGui::CollapsingHeader("Materials:"))
	{
		for (int i = 0; i < materials.size(); i++)
		{
			// Support multiple widgets with the same name
			ImGui::PushID(materials[i].get());

			ImGui::Text(materials[i]->GetMaterialName());

			// Adjust the color tint
			DirectX::XMFLOAT4 matColor = materials[i]->GetColorTint();
			if (ImGui::ColorEdit4("Color Tint", &matColor.x))
			{
				materials[i]->SetColorTint(matColor);
			}

			// Adjust the UV scale & offset of the material
			XMFLOAT2 uvScale = materials[i]->GetUVScale();
			XMFLOAT2 uvOffset = materials[i]->GetUVOffset();

			if (ImGui::DragFloat2("UV Scale", &uvScale.x, 0.1f))
			{
				materials[i]->SetUVScale(uvScale);
			}

			if (ImGui::DragFloat2("UV Offset", &uvOffset.x, 0.1f))
			{
				materials[i]->SetUVOffset(uvOffset);
			}

			// Display all the textures used in this material
			for (auto& t : materials[i]->GetTextureSRVMap())
			{
				ImGui::Text(t.first.c_str());
				ImGui::Image(unsigned long long(t.second.Get()), ImVec2(256, 256)); // *NOTE:* static_cast<void*> NOR <ImTextureID> were working
			}
			ImGui::Spacing();

			ImGui::PopID();
		}
	}

	// Make a tab to display all the available materials
	if (ImGui::CollapsingHeader("Lights:"))
	{
		
		// Ambient light
		/*if (ImGui::TreeNode("Node", "Ambient Light"))
		{
			ImGui::ColorEdit3("Ambient Color", &ambientTerm.x);
			ImGui::TreePop();
		}*/
		
		// Directional, point, & spot lights
		for (int i = 0; i < lights.size(); i++) 
		{
			std::string lightType = " Light (%u)";

			switch (lights[i].Type)
			{
				case LIGHT_TYPE_DIRECTIONAL:
					lightType = "Directional" + lightType;
					break;

				case LIGHT_TYPE_POINT:
					lightType = "Point" + lightType;
					break;

				case LIGHT_TYPE_SPOT:
					lightType = "Spot" + lightType;
					break;
			}

			// Support multiple widgets with the same name
			ImGui::PushID(&lights[i]);

			if (ImGui::TreeNode("Light Node", lightType.c_str(), i))
			{
				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				ImGui::SliderFloat("Intensity", &lights[i].Intensity, 0.0f, 5.0f);

				// ONLY display the shadow map for the light that's using it
				if (i == 0)
				{
					ImGui::Spacing();
					ImGui::SliderInt("Shadow Map Resolution", &shadowMapResolution, int(pow(2, 6)), int(pow(2, 12))); // Pow of 2 works best
					if (ImGui::SliderFloat("Shadow Projection Size", &lightProjectionSize, 0.5f, 250.0f))
					{
						XMMATRIX shadowLightProj = XMMatrixOrthographicLH(
							lightProjectionSize,
							lightProjectionSize,
							1.0f,
							100.0f);
						XMStoreFloat4x4(&lightProjectionMatrix, shadowLightProj);
					}
					// Shadow Map
					ImGui::Image(unsigned long long(shadowSRV.Get()), ImVec2(512, 512));
				}
				

				/*if (ImGui::RadioButton("Directional", lights[i].Type == LIGHT_TYPE_DIRECTIONAL))
				{
					lights[i].Type == LIGHT_TYPE_DIRECTIONAL;
				}
				if (ImGui::RadioButton("Point", lights[i].Type == LIGHT_TYPE_POINT))
				{
					lights[i].Type == LIGHT_TYPE_POINT;
				}
				if (ImGui::RadioButton("Spot", lights[i].Type == LIGHT_TYPE_SPOT))
				{
					lights[i].Type == LIGHT_TYPE_SPOT;
				}*/
				
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	// Make a tab to display all the post process effects
	if (ImGui::CollapsingHeader("Post Processes:"))
	{
		ImVec2 size;
		size.x = ImGui::GetWindowWidth() - 50;
		size.y = size.x / Window::AspectRatio();
		
		ImGui::SliderInt("\"Box\" Blur", &blurRadius, 0, 10);
		ImGui::Image(unsigned long long(ppBoxBlurSRV.Get()), size);

		ImGui::SliderFloat("Bloom Threshold", &brightnessThreshold, 0, 10);
		ImGui::SliderFloat("Bloom Intensity", &bloomIntensLvl, 0, 10);
	}

	// End the current window
	ImGui::End(); 
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Update ImGui UI
	RefreshImGui(deltaTime);

	// Grab the shadowMapResolution before creating the UI
	int prevShadowMapRes = shadowMapResolution;

	// Create ImGui UI
	//BuildUI(meshes, entities, cameraViews, activeCamera, lights, ambientTerm);
	BuildUI(meshes, entities, cameraViews, activeCamera, lights);

	// Recreate the shadow map if the res. changed
	if (prevShadowMapRes != shadowMapResolution)
	{
		CreateShadowMap();
	}

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update Entity transforms
	//rectangle->GetTransform()->SetPosition(sin(totalTime)/2, 0, 0); // Move back & forth along x
	//heart->GetTransform()->Rotate(0, 0, deltaTime); // Spin about the origin
	//rgbTriangle->GetTransform()->SetScale(abs(cos(totalTime)), abs(cos(totalTime)), 1); // Bouncy scaling

	entities[1]->GetTransform()->SetPosition(-8.5f, sin(totalTime) * 2 + 1, 0); // Move up & down
	entities[2]->GetTransform()->SetPosition(-5 + sin(totalTime), 1.5f, cos(totalTime)); // Move in a clockwise circle
	entities[3]->GetTransform()->Rotate(0, deltaTime, 0); // Spin/twist like a screw
	entities[4]->GetTransform()->Rotate(deltaTime, 0, deltaTime); // Rotate along the x & z-axis like a gyroscope
	entities[5]->GetTransform()->SetScale(abs(cos(totalTime)) + 0.1f, abs(cos(totalTime)) + 0.1f, abs(cos(totalTime)) + 0.1f); // Bouncy scaling
	entities[6]->GetTransform()->SetScale(1, abs(sin(totalTime)) + 0.2f, 1); // Squash & stretch along the y-axis

	// Update the camera each frame
	activeCamera->Update(deltaTime);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen (with color!)) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// Before anything else (including changing buffers for PP), render the shadow map
	RenderShadowMap(); 

	// Clear any and all extra render targets
	Graphics::Context->ClearRenderTargetView(ppBoxBlurRTV.Get(), color);

	Graphics::Context->ClearRenderTargetView(ppBloomRTV.Get(), color);
	Graphics::Context->ClearRenderTargetView(ppBloomExtractRTV.Get(), color);

	Graphics::Context->ClearRenderTargetView(horizBlurRTV.Get(), color);
	Graphics::Context->ClearRenderTargetView(verticBlurRTV.Get(), color);

	// Swap the active render target for post processing
	//Graphics::Context->OMSetRenderTargets(1, ppBloomRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());
	Graphics::Context->OMSetRenderTargets(1, ppBoxBlurRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());

	// Rotate around z-axis based on time
	//XMMATRIX translMatrix = XMMatrixTranslation(sin(totalTime), 0, 0);
	//XMMATRIX zRotMatrix = XMMatrixRotationZ(totalTime);

	// Store before copying
	//XMFLOAT4X4 zRot;
	//XMFLOAT4X4 transl;
	//XMStoreFloat4x4(&zRot, zRotMatrix);

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		// Draw all the entities in the list
		for (auto& e : entities)
		{
			// Bind the textures
			//e->GetMaterial()->GetPixelShader()->SetShaderResourceView("ColorTexture", textureSRV);

			e->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", lightViewMatrix);
			e->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProj", lightProjectionMatrix);

			//e->GetMaterial()->GetPixelShader()->SetFloat3("ambientColor", ambientTerm);
			e->GetMaterial()->GetPixelShader()->SetFloat("Time", totalTime);
			e->GetMaterial()->GetPixelShader()->SetData(
				"lights", // The name of the variable in the shader
				&lights[0], // The address of the data to set
				sizeof(Light) * (int)lights.size()); // The size of the data (the whole structs!) to set

			e->GetMaterial()->GetPixelShader()->SetShaderResourceView("ShadowMap", shadowSRV);
			e->GetMaterial()->GetPixelShader()->SetSamplerState("ShadowSampler", shadowSampler);

			e->Draw(activeCamera);
		}

		// Draw the sky box afterwards to avoid unnecessary work
		skyBox->Draw(activeCamera);

		// Unbind the shadow map as a shader resource so it can be used as a depth buffer at the start of next frame!
		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
	}

	// Post Processing:
	{
		Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0); // Reset the backBuffer

		// Swap to "fullscreen triangle trick" by turning off normal vertex & index buffers
		Graphics::Context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
		ID3D11Buffer* emptyBuffer = 0;
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		Graphics::Context->IASetVertexBuffers(0, 1, &emptyBuffer, &stride, &offset);
		/*
		Graphics::Context->PSSetSamplers(0, 1, postProcSampler.GetAddressOf()); // If all the post process steps have a single sampler at register 0

		// Bloom Extract:
		// Half-sized texture, so adjust viewport
		D3D11_VIEWPORT vp = {};
		vp.Width = Window::Width() * 0.5f;
		vp.Height = Window::Height() * 0.5f;
		vp.MaxDepth = 1.0f;
		Graphics::Context->RSSetViewports(1, &vp);

		// Render to extract texture
		Graphics::Context->OMSetRenderTargets(1, ppBloomExtractRTV.GetAddressOf(), 0);

		// Activate shader & set resources
		bloomExtractPS->SetShader();
		bloomExtractPS->SetShaderResourceView("Pixels", ppBloomSRV.Get()); // IMPORTANT: This step takes the original post process texture!
		// Note: Sampler set already!

		// Set post process specific data
		bloomExtractPS->SetFloat("brightnessThreshold", brightnessThreshold);
		bloomExtractPS->CopyAllBufferData();

		// Draw exactly 3 vertices for our "full screen triangle"
		Graphics::Context->Draw(3, 0);

		// Blur Horizontal:

		// Blur Vertical:

		// Combine:





		// Now swap the active render target over to chain bloom & box blur
		Graphics::Context->OMSetRenderTargets(1, ppBoxBlurRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get()); 
		*/

		//Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0); // Reset the backBuffer

		// Swap to "fullscreen triangle trick" by turning off normal vertex & index buffers
		/*Graphics::Context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
		ID3D11Buffer* emptyBuffer = 0;
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		Graphics::Context->IASetVertexBuffers(0, 1, &emptyBuffer, &stride, &offset);*/

		// Activate shaders and bind resources
		ppFullscrTriVS->SetShader();
		ppBoxBlurPS->SetShader();
		ppBoxBlurPS->SetShaderResourceView("Pixels", ppBoxBlurSRV.Get());
		ppBoxBlurPS->SetSamplerState("ClampSampler", postProcSampler.Get());

		// Set required cbuffer data
		ppBoxBlurPS->SetInt("blurRadius", blurRadius);
		ppBoxBlurPS->SetFloat("pixelWidth", 1.0f / Window::Width());
		ppBoxBlurPS->SetFloat("pixelHeight", 1.0f / Window::Height());
		ppBoxBlurPS->CopyAllBufferData();

		// Draw the triangle filling the screen
		Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)

		// Unbind at frame end for rendering into at start of next
		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Draw ImGui after Box Blur PP to keep it crisp
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState(); // Syncronize frame rate
		// Show user what's been rendered
		Graphics::SwapChain->Present( 
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}


void Game::RenderShadowMap()
{
	// Set up shadow map as depth buffer
	Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0); // Clear shadow map
	// Set up output merger stage
	//ID3D11RenderTargetView* nullRTV{};
	//Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());
	Graphics::Context->OMSetRenderTargets(0, 0, shadowDSV.Get()); 
	Graphics::Context->RSSetState(shadowRasterizer.Get());

	// Change other render state to prepare for the shadow render
	// Match vieewport to shadow map res. instead of screen size
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	// Set up shadow shaders
	shadowsVS->SetShader();
	Graphics::Context->PSSetShader(0, 0, 0); // Unbind pixel shader to prevent pixel processing entirely

	shadowsVS->SetMatrix4x4("view", lightViewMatrix);
	shadowsVS->SetMatrix4x4("projection", lightProjectionMatrix);

	// Loop thru entities & draw to the shadow map
	for (auto& e : entities)
	{
		shadowsVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowsVS->CopyAllBufferData();

		// Draw the mesh directly to avoid the entity's material
		e->GetMesh()->SetAndDrawBuffers();
	}

	// Reset to the normal render target & back buffer
	Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());
	
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height(); 
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->RSSetState(0);
}
