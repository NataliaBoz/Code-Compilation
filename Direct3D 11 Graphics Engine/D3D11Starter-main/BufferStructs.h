#pragma once

struct VertexShaderData 
{
	DirectX::XMFLOAT4 tint; // *NOTE: Watch that 16-byte boundary!*
	DirectX::XMFLOAT4X4 world; //DirectX::XMFLOAT3 offset;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};