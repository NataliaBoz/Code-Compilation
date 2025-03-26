cbuffer ExternalData : register(b0)
{
    float4 colorTint;
}

// Struct of data from earlier in the pipeline
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

// Very simple pseudo-random function for noise, which takes a 2D vector as input
float random(float2 s)
{
    return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}

// Return the normal data as RGBA color values for each pixel
float4 main(VertexToPixel input) : SV_TARGET
{
    return float4(random(input.screenPosition.xy) + random(input.screenPosition.xy),
        random(input.screenPosition.xy) + random(input.screenPosition.xy),
        random(input.screenPosition.xy) - random(input.screenPosition.xy),
        0);
}