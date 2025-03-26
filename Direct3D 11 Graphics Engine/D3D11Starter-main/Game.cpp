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

		sideCamera = std::make_shared<Camera>(XMFLOAT3(-1.5f, 1.0f, -1.5f), // Initial starting position
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
	}
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
// Creates the geometry (mesh vertices, indices, & shaders) to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Load the SimpleShader objects
	std::shared_ptr<SimpleVertexShader> vertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> pixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	// UVs Pixel Shader
	std::shared_ptr<SimplePixelShader> uvsPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"UVsPS.cso").c_str());
	// Normals Pixel Shader
	std::shared_ptr<SimplePixelShader> normalsPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"NormalsPS.cso").c_str());
	// Custom Pixel Shader
	std::shared_ptr<SimplePixelShader> customPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());
	// Combining Textures Pixel Shader
	std::shared_ptr<SimplePixelShader> combinePS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CombinePS.cso").c_str());

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
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> arcadeFloorSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blackTealMarbleSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blueTravertineSRV;
	

	// Repeat for EACH texture to load from file (PNG preferable, but JPG also works)
	// Arcade floor texture
	CreateWICTextureFromFile(Graphics::Device.Get(), // Graphics device
		Graphics::Context.Get(), // The context for auto MOP
		FixPath(L"../../Assets/Textures/ArcadeFloor.png").c_str(), // Texture
		0, // Not the actual texture object, could also use nullptr
		arcadeFloorSRV.GetAddressOf()); // Get SRV

	// Black & teal marble texture
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Marble009_1K-PNG_Color.png").c_str(), 
		0, 
		blackTealMarbleSRV.GetAddressOf());

	// Light blue travertine texture
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Travertine013_1K-PNG_Color.png").c_str(),
		0,
		blueTravertineSRV.GetAddressOf());
		
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

	// Textured materials
	arcadeFloorMaterial = std::make_shared<Material>("Arcade Floor", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader, XMFLOAT2(2, 2));
	arcadeFloorMaterial->AddSampler("BasicSampler", sampler);
	arcadeFloorMaterial->AddTextureSRV("SurfaceTexture", arcadeFloorSRV);

	blackTealMarbleMaterial = std::make_shared<Material>("Black & Teal Marble", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);
	blackTealMarbleMaterial->AddSampler("BasicSampler", sampler);
	blackTealMarbleMaterial->AddTextureSRV("SurfaceTexture", blackTealMarbleSRV);

	blueTravertineMaterial = std::make_shared<Material>("Light Blue Travertine", XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader, XMFLOAT2(0.5f, 0.5f));
	blueTravertineMaterial->AddSampler("BasicSampler", sampler);
	blueTravertineMaterial->AddTextureSRV("SurfaceTexture", blueTravertineSRV);

	deepBlueTravertineMaterial = std::make_shared<Material>("Deep Blue Travertine", XMFLOAT4(0.25f, 0.35f, 1, 1), vertexShader, pixelShader);
	deepBlueTravertineMaterial->AddSampler("BasicSampler", sampler);
	deepBlueTravertineMaterial->AddTextureSRV("SurfaceTexture", blueTravertineSRV);

	comboMaterial = std::make_shared<Material>("Combination", XMFLOAT4(1, 1, 1, 1), vertexShader, combinePS);
	comboMaterial->AddSampler("BasicSampler", sampler);
	comboMaterial->AddTextureSRV("InitialTexture", blueTravertineSRV);
	comboMaterial->AddTextureSRV("CombineTexture", arcadeFloorSRV);

	// Put all the materials in a list
	/*materials.push_back(cyanMaterial);
	materials.push_back(magentaMaterial);
	materials.push_back(greenYellowMaterial);
	materials.push_back(uvsMaterial);
	materials.push_back(normalsMaterial);
	materials.push_back(customMaterial)*/;
	materials.push_back(arcadeFloorMaterial);
	materials.push_back(blackTealMarbleMaterial);
	materials.push_back(blueTravertineMaterial);
	materials.push_back(deepBlueTravertineMaterial);
	materials.push_back(comboMaterial);

	// Create pointers to each 3D entity & add to the list for drawing
	entities.push_back(std::make_shared<GameEntity>(cubeMesh, arcadeFloorMaterial));
	entities.push_back(std::make_shared<GameEntity>(cylinderMesh, blackTealMarbleMaterial));
	entities.push_back(std::make_shared<GameEntity>(helixMesh, blueTravertineMaterial));
	entities.push_back(std::make_shared<GameEntity>(quadMesh, comboMaterial));
	entities.push_back(std::make_shared<GameEntity>(doubleSidedQuadMesh, blackTealMarbleMaterial));
	entities.push_back(std::make_shared<GameEntity>(sphereMesh, deepBlueTravertineMaterial));
	entities.push_back(std::make_shared<GameEntity>(torusMesh, arcadeFloorMaterial));

	// Adjust the meshes' transforms to spread them out 
	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->GetTransform()->MoveAbsolute(float(-9 + 3 * i), 0, 0); // Cast to a float to remove warning
	}
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
	std::shared_ptr<Camera> &activeCamera)
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
	ImGui::Checkbox("Check it!", &check);

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
	// Create ImGui UI
	BuildUI(meshes, entities, cameraViews, activeCamera);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update Entity transforms
	//rectangle->GetTransform()->SetPosition(sin(totalTime)/2, 0, 0); // Move back & forth along x
	//heart->GetTransform()->Rotate(0, 0, deltaTime); // Spin about the origin
	//rgbTriangle->GetTransform()->SetScale(abs(cos(totalTime)), abs(cos(totalTime)), 1); // Bouncy scaling

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
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

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

			e->GetMaterial()->GetPixelShader()->SetFloat("Time", totalTime);

			e->Draw(activeCamera);
		}

		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
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

