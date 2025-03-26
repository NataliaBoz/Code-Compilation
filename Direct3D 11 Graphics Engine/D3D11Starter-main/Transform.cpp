#include "Transform.h"

using namespace DirectX;

Transform::Transform() : 
    position(0, 0, 0),
    pitchYawRoll(0, 0, 0),
    scale(1, 1, 1),
    dirty(false),
    right(1, 0, 0),
    up(0, 1, 0),
    forward(0, 0, 1)
{
    XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
}

void Transform::SetPosition(float x, float y, float z)
{
    position.x = x;
    position.y = y;
    position.z = z;
    dirty = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
    SetPosition(position.x, position.y, position.z);
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
    pitchYawRoll.x = pitch;
    pitchYawRoll.y = yaw;
    pitchYawRoll.z = roll;
    dirty = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
    SetRotation(rotation.x, rotation.y, rotation.z);
}

void Transform::SetScale(float x, float y, float z)
{
    scale.x = x;
    scale.y = y;
    scale.z = z;
    dirty = true;

}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
    SetScale(scale.x, scale.y, scale.z);
}

// Adds the specified x, y & z values to the existing position
void Transform::MoveAbsolute(float x, float y, float z)
{
    XMStoreFloat3(&position, XMLoadFloat3(&position) + XMVectorSet(x, y, z, 0));
    dirty = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
    MoveAbsolute(offset.x, offset.y, offset.z);
}

// Move along the "local" space
void Transform::MoveRelative(float x, float y, float z)
{
    XMVECTOR movement = XMVectorSet(x, y, z, 0);

    XMVECTOR rotateQuatern = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));

    XMVECTOR direction = XMVector3Rotate(movement, rotateQuatern);

    XMStoreFloat3(&position, XMLoadFloat3(&position) + direction);
    dirty = true;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
    MoveRelative(offset.x, offset.y, offset.z);
}

// Adds the specified pitch, yaw & roll values to the current
void Transform::Rotate(float pitch, float yaw, float roll)
{
    XMStoreFloat3(&pitchYawRoll, XMLoadFloat3(&pitchYawRoll) + XMVectorSet(pitch, yaw, roll, 0));
    dirty = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
    Rotate(rotation.x, rotation.y, rotation.z);
}

void Transform::Scale(float x, float y, float z)
{
    XMStoreFloat3(&scale, XMLoadFloat3(&scale) * XMVectorSet(x, y, z, 0));
    dirty = true;
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
    Scale(scale.x, scale.y, scale.z);
}

// Getters
DirectX::XMFLOAT3 Transform::GetPosition() { return position; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return pitchYawRoll; }
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
    // Check if any of the transformations have changed
    if (dirty)
    {
        XMMATRIX translMatrix = XMMatrixTranslation(position.x, position.y, position.z);
        XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
        XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
        // Note: Overloaded operators are defined in the DirectX namespace!
        // Alternatively, you can call XMMatrixMultiply(XMMatrixMultiply(s, r), t))
        XMMATRIX world = scaleMatrix * rotMatrix * translMatrix; // S*R*T

        XMStoreFloat4x4(&worldMatrix, scaleMatrix * rotMatrix * translMatrix);
        dirty = false;
    }

    return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
    // Check if any of the transformations have changed
    if (dirty)
    {
        XMMATRIX translMatrix = XMMatrixTranslation(position.x, position.y, position.z);
        XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
        XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

        XMMATRIX world = scaleMatrix * rotMatrix * translMatrix;

        // *NOTE: Not a fan of the duplic. code here but can't convert XMFLOAT4X4 to MATRIX for Inv() & Transp()*
        XMStoreFloat4x4(&worldInverseTranspose,
            XMMatrixInverse(0, XMMatrixTranspose(scaleMatrix * rotMatrix * translMatrix)));
        dirty = false;
    }

    return worldInverseTranspose;
}

DirectX::XMFLOAT3 Transform::GetRight() 
{
    // Check for any changes
    if (dirty)
    {
        XMVECTOR rotateQuatern = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
        XMStoreFloat3(&right, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), rotateQuatern));
        dirty = false;
    }

    return right;
}

DirectX::XMFLOAT3 Transform::GetUp()  
{
    if (dirty)
    {
        XMVECTOR rotateQuatern = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
        XMStoreFloat3(&up, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rotateQuatern));
        dirty = false;
    }

    return up;
}

DirectX::XMFLOAT3 Transform::GetForward() 
{
    if (dirty)
    {
        XMVECTOR rotateQuatern = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
        XMStoreFloat3(&forward, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rotateQuatern));
        dirty = false;
    }

    return forward;
}
