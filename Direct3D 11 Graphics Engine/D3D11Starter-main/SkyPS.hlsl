#include "ShaderInclude.hlsli" // Contains all necessary structs, helper functions, etc.

TextureCube SkyTexture : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    // Sample the cube map in the correct direction and return the result
    return SkyTexture.Sample(BasicSampler, input.sampleDir);
}