#pragma once

#include <d3d11.h> //  Direct3D "stuff"
#include <wrl/client.h> // ComPtrs for Direct3D objects
#include "Graphics.h" // Starter code’s Graphics::Device & Graphics::Context objects
#include "Vertex.h" // Access the custom Vertex struct
#include <vector>
#include <fstream> 
#include <stdexcept>


class Mesh
{
public:
	// Constructor
	Mesh(Vertex* vertices, unsigned int* indices, unsigned int vertCount, unsigned int indCount); //, const char* name);
	// Second mesh construct
	Mesh(const char* name, const char* objFile);

	// Destructor
	~Mesh();

	// Getters
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer(); // Returns the vertex buffer ComPtr
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer(); // Returns the index buffer ComPtr
	unsigned int GetIndexCount(); // Returns the # of indices this mesh contains
	unsigned int GetVertexCount(); // Returns the # of vertices this mesh contains
	const char* GetMeshName(); // Return the identifying string of this mesh

	// Methods
	void CreateVertIndBuffers(Vertex* vertices, unsigned int vertCount, unsigned int* indices, unsigned int indCount);
	void SetAndDrawBuffers(); // Sets the buffers and draws using the correct number of indices

private:
	// ComPtrs for this mesh's buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indBuffer;
	// # of indices in this mesh's index buffer
	unsigned int indCount = 0; 
	// # of vertices in this mesh's vertex buffer
	unsigned int vertCount = 0; 
	const char* name;
};

