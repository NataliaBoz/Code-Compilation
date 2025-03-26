cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
}

Texture2D InitialTexture : register(t0); // "t" registers for textures
Texture2D CombineTexture : register(t1);

SamplerState BasicSampler : register(s0); // "s" registers for samplers

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

float4 main(VertexToPixel input) : SV_TARGET
{	
	// Adjust the uv coords by scale & offset for repeating textures
    input.uv = input.uv * uvScale + uvOffset;

	// Sample the texture color and apply the tint
    float4 initTextureColor = InitialTexture.Sample(BasicSampler, input.uv);
    float4 comboTextureColor = CombineTexture.Sample(BasicSampler, input.uv);
    float4 textureColor = initTextureColor - comboTextureColor; // * = tint, + = brighter, - = darker
    textureColor *= colorTint;
    return textureColor;
}