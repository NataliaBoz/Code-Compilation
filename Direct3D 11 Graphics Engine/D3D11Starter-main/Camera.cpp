#include "Camera.h" 

using namespace DirectX; 

Camera::Camera(DirectX::XMFLOAT3 initPos, float moveSpeed, float lookSpeed, float fov, float aspRatio, 
	float nearClip, float farClip) :

	movementSpeed(moveSpeed),
	mouseLookSpeed(lookSpeed),
	fieldOfView(fov),
	aspectRatio(aspRatio),
	nearClipDist(nearClip),
	farClipDist(farClip)
{
	transform = std::make_shared<Transform>();
	transform->SetPosition(initPos);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
	
}


void Camera::Update(float deltaTime)
{
	// Scale speed to make movement indep. of framerate
	float scaledSpeed = movementSpeed * deltaTime;

	if (Input::KeyDown('W')) { transform->MoveRelative(0, 0, scaledSpeed); }
	if (Input::KeyDown('S')) { transform->MoveRelative(0, 0, -scaledSpeed); }
	if (Input::KeyDown('A')) { transform->MoveRelative(-scaledSpeed, 0, 0); }
	if (Input::KeyDown('D')) { transform->MoveRelative(scaledSpeed, 0, 0); }
	if (Input::KeyDown(' ')) { transform->MoveAbsolute(0, scaledSpeed, 0); }
	if (Input::KeyDown('X')) { transform->MoveAbsolute(0, -scaledSpeed, 0); }

	// "Virtual-Key Codes"
	//if (Input::KeyDown(VK_SHIFT)) { /* Shift is down */ }
	//if (Input::KeyDown(VK_CONTROL)) { /* Control is down */ }

	// Only rotate if the mouse is held down
	if (Input::MouseLeftDown())
	{
		// Get change in mouse position
		//Input::GetMouseXDelta();
		//Input::GetMouseYDelta();
		float xRotate = Input::GetMouseXDelta() * mouseLookSpeed;
		float yRotate = Input::GetMouseYDelta() * mouseLookSpeed;

		transform->Rotate(yRotate, xRotate, 0);

		// Prevent looking completely up
		XMFLOAT3 rot = transform->GetPitchYawRoll();
		if (rot.x > XM_PIDIV2) { rot.x = XM_PIDIV2 - 0.0001f; } // Clamp to 1/2 pi, -1/2 pi
		if (rot.x < -XM_PIDIV2) { rot.x = -XM_PIDIV2 + 0.0001f; }
	}

	// Ensure view matrix actually matches camera’s current transform
	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = transform->GetPosition();
	XMFLOAT3 direction = transform->GetForward(); // Forward direction
	XMFLOAT3 worldUp = XMFLOAT3(0, 1, 0);

	XMStoreFloat4x4(&viewMatrix, 
		XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&direction), XMLoadFloat3(&worldUp))
	);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	// Set the given aspect ratio
	this->aspectRatio = aspectRatio;

	XMStoreFloat4x4(&projMatrix, 
		XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, nearClipDist, farClipDist)
	);
}

// Getters
DirectX::XMFLOAT4X4 Camera::GetView() { return viewMatrix; }
DirectX::XMFLOAT4X4 Camera::GetProjection() { return projMatrix; }
std::shared_ptr<Transform> Camera::GetTransform() { return transform; }
