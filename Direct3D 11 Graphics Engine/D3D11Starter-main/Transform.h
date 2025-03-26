#pragma once

#include <DirectXMath.h>

// *NOTE: NO using namespace in .h files!*

class Transform
{
public:
	Transform();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 rotation); // XMFLOAT4 for quaternion
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 scale);

	// Transformers
	void MoveAbsolute(float x, float y, float z); // World space
	void MoveAbsolute(DirectX::XMFLOAT3 offset); 
	void MoveRelative(float x, float y, float z); // Local space
	void MoveRelative(DirectX::XMFLOAT3 offset); 
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll(); // XMFLOAT4 GetRotation() for quaternion 
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetForward();


private:
	// Transformation data
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll; // Could use a quaternion as XMFLOAT4
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 right; 
	DirectX::XMFLOAT3 up;
	DirectX::XMFLOAT3 forward;

	// Matrices
	bool dirty;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTranspose;

};

