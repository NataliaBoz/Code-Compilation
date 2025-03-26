#include "Mesh.h"

// For the DirectX Math library
using namespace DirectX;

Mesh::Mesh(Vertex* vertices, unsigned int* indices, unsigned int vertCount, unsigned int indCount) //, const char* name)
{
	this->vertCount = vertCount;
	this->indCount = indCount; 
	//this->name = name;

	CreateVertIndBuffers(vertices, vertCount, indices, indCount);
}

// Second mesh construct
Mesh::Mesh(const char* name, const char* objFile)  :
	name(name)
{
	// Author: Chris Cascioli
	// Purpose: Basic .OBJ 3D model loading, supporting positions, uvs and normals
	// 
	// - You are allowed to directly copy/paste this into your code base
	//   for assignments, given that you clearly cite that this is not
	//   code of your own design.

	// *************************************
	//      IMPLEMENTATION NOTES (1/2)
	//
	//  - You'll need to #include both
	//      <fstream> and <stdexcept>
	//
	//  - This is code you will need to 
	//      integrate into a function or
	//      constructor of your own making
	//
	//  - There is MORE TO DO after pasting 
	//      this code in - see the bottom for
	//      what to do *after* including 
	//      this code in your mesh class
	//
	// *************************************

	// File input object
	std::ifstream obj(objFile);

	// Check for successful open
	if (!obj.is_open())
		throw std::invalid_argument("Error opening file: Invalid file path or file is inaccessible");

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;	// Positions from the file
	std::vector<XMFLOAT3> normals;		// Normals from the file
	std::vector<XMFLOAT2> uvs;		// UVs from the file
	std::vector<Vertex> verts;		// Verts we're assembling
	std::vector<UINT> indices;		// Indices of these verts
	int vertCounter = 0;			// Count of vertices
	int indexCounter = 0;			// Count of indices
	char chars[100];			// String for line reading

	// Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			// NOTE: This assumes the given obj file contains
			//  vertex positions, uv coordinates AND normals.
			unsigned int i[12];
			int numbersRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// If we only got the first number, chances are the OBJ
			// file has no UV coordinates.  This isn't great, but we
			// still want to load the model without crashing, so we
			// need to re-read a different pattern (in which we assume
			// there are no UVs denoted for any of the vertices)
			if (numbersRead == 1)
			{
				// Re-read with a different pattern
				numbersRead = sscanf_s(
					chars,
					"f %d//%d %d//%d %d//%d %d//%d",
					&i[0], &i[2],
					&i[3], &i[5],
					&i[6], &i[8],
					&i[9], &i[11]);

				// The following indices are where the UVs should 
				// have been, so give them a valid value
				i[1] = 1;
				i[4] = 1;
				i[7] = 1;
				i[10] = 1;

				// If we have no UVs, create a single UV coordinate
				// that will be used for all vertices
				if (uvs.size() == 0)
					uvs.push_back(XMFLOAT2(0, 0));
			}

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Flip Z (LH vs. RH)
			v1.Position.z *= -1.0f;
			v2.Position.z *= -1.0f;
			v3.Position.z *= -1.0f;

			// Flip normal's Z
			v1.Normal.z *= -1.0f;
			v2.Normal.z *= -1.0f;
			v3.Normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);
			vertCounter += 3;

			// Add three more indices
			indices.push_back(indexCounter); indexCounter += 1;
			indices.push_back(indexCounter); indexCounter += 1;
			indices.push_back(indexCounter); indexCounter += 1;

			// Was there a 4th face?
			// - 12 numbers read means 4 faces WITH uv's
			// - 8 numbers read means 4 faces WITHOUT uv's
			if (numbersRead == 12 || numbersRead == 8)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal's Z
				v4.UV.y = 1.0f - v4.UV.y;
				v4.Position.z *= -1.0f;
				v4.Normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);
				vertCounter += 3;

				// Add three more indices
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
			}
		}
	}

	// Close the file
	obj.close();

	// *************************************
	//      IMPLEMENTATION NOTES (2/2)
	//
	// - At this point, "verts" is a std::vector 
	//     of Vertex structs, and can be used
	//     to create your mesh's vertex buffer
	//
	// - NOTE: Use &verts[0] for the address of 
	//     the first vertx, NOT JUST &verts
	//
	// - The vector "indices" is similar. It's 
	//     a std::vector of unsigned ints and
	//     can be used to create the index 
	//     buffer (again, &indices[0] is the 
	//     address of the first integer)
	//
	// - Make sure your mesh class actually SAVES
	//     the number of vertices and indices, or
	//     drawing may have unintended problems.
	//     - "vertCounter" is the number of vertices
	//     - "indexCounter" is the number of indices
	//
	// - If you dig into the code, you may notice 
	//     that  "vertCounter" and "indexCounter" 
	//     end up being the same.  Recall that OBJs 
	//     do not index entire vertices, making
	//     it complex to detect duplicate vertices.
	//     This also means an index buffer isn't
	//     doing much for us.  We could try to 
	//     detect duplicate vertices ourselves, 
	//     but at that point it would be better 
	//     to use a more sophisticated mesh loading
	//     library like TinyOBJLoader or 
	//     The Open Asset Importer Library, both
	//     of which are unnecessary for now.
	//
	// *************************************

	// Create the actual buffers
	CreateVertIndBuffers(&verts[0], vertCounter, &indices[0], indexCounter);
}

Mesh::~Mesh()
{

}

// Returns the vertex buffer ComPtr
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer() { return vertBuffer; }

// Returns the index buffer ComPtr
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer() { return indBuffer; }

// Returns the number of indices this mesh contains
unsigned int Mesh::GetIndexCount() { return indCount; }

// Returns the number of vertices this mesh contains
unsigned int Mesh::GetVertexCount() { return vertCount; }

// Returns the name of this mesh as an identifier
const char* Mesh::GetMeshName() { return name; }

void Mesh::CreateVertIndBuffers(Vertex* vertices, unsigned int vertCount, unsigned int* indices, unsigned int indCount)
{
	// Create a VERTEX BUFFER to hold vertex data of triangles for a single object
	// Created on the GPU where the data needs to be if we want the GPU to draw it to the screen
	// Describe the buffer we want Direct3D to make on the GPU
	D3D11_BUFFER_DESC vertBuffDescr = {};
	vertBuffDescr.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	vertBuffDescr.ByteWidth = sizeof(Vertex) * vertCount; // = # of vertices in the buffer
	vertBuffDescr.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
	vertBuffDescr.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	vertBuffDescr.MiscFlags = 0;
	vertBuffDescr.StructureByteStride = 0;
	// Create the proper struct to hold the initial vertex data for the buffer
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertices; // pSysMem = Pointer to System Memory
	// Actually create the buffer on the GPU with the initial data
	Graphics::Device->CreateBuffer(&vertBuffDescr, &initialVertexData, vertBuffer.GetAddressOf());

	// Create an INDEX BUFFER to hold indices to elements in the vertex buffer
	// Created on the GPU where the data needs to be if we want the GPU to draw it to the screen
	// Describe the buffer (like the vertex buffer) BUT with:
	//  - Byte Width (3 unsigned integers vs. 3 whole vertices)
	//  - Bind Flag (used as an index buffer instead of a vertex buffer) 
	D3D11_BUFFER_DESC indBuffDescr = {};
	indBuffDescr.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	indBuffDescr.ByteWidth = sizeof(unsigned int) * indCount;	// = # of indices in the buffer
	indBuffDescr.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
	indBuffDescr.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	indBuffDescr.MiscFlags = 0;
	indBuffDescr.StructureByteStride = 0;
	// Specify the initial data for this buffer, similar to above
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices; // pSysMem = Pointer to System Memory
	// Actually create the buffer with the initial data
	Graphics::Device->CreateBuffer(&indBuffDescr, &initialIndexData, indBuffer.GetAddressOf());

	// Store the vertex & index counts
	this->vertCount = (unsigned int)vertCount;
	this->indCount = (unsigned int)indCount;
}

// Sets the buffers and draws using the correct number of indices
void Mesh::SetAndDrawBuffers()
{
	// Refer to Game::Draw() to see the code necessary for setting buffers and drawing
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, vertBuffer.GetAddressOf(), &stride, &offset);
	Graphics::Context->IASetIndexBuffer(indBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	Graphics::Context->DrawIndexed(
		indCount, // The number of indices to use (we could draw a subset if we wanted) ***
		0, // Offset to the first index we want to use
		0); // Offset to add to each index when looking up vertices
}

// Calc Tangents
