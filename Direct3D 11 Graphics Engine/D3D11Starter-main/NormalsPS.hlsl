#include "ShaderInclude.hlsli"

// Return the normal data as RGBA color values for each pixel
float4 main(VertexToPixel input) : SV_TARGET
{
    return float4(input.normal, 1);
}