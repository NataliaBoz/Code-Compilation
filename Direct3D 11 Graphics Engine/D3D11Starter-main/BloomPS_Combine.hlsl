// Post Process Effect: Bloom - Combine step
cbuffer externalData : register(b0)
{
    float intensityLvl;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D OrigPixels : register(t0);
Texture2D BloomPixels : register(t1);
SamplerState BloomSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    float4 totalColor = OrigPixels.Sample(BloomSampler, input.uv);
    totalColor += BloomPixels.Sample(BloomSampler, input.uv) * intensityLvl;

    return totalColor;
}