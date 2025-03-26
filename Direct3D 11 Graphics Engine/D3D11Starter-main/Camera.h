#pragma once

#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>
#include <memory>

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 initPos, float moveSpeed, float lookSpeed, float fov, float aspRatio, 
		float nearClip = 0.01f, float farClip = 200.0f);

	~Camera();

	// Update methods
	void Update(float deltaTime);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	// Getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	std::shared_ptr<Transform> GetTransform();

private:
	std::shared_ptr<Transform> transform;
	// Camera matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	float movementSpeed;
	float mouseLookSpeed;
	float fieldOfView; // Angle in radians (btw 45 & 90 degr a.k.a. PI/4 & PI/2)
	float aspectRatio;
	float nearClipDist;
	float farClipDist;
};

