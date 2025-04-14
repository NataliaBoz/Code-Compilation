#include "ShaderInclude.hlsli" // Contains all necessary structs, helper functions, etc.

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
    float roughness;
	float3 currentCamPos;
    float3 ambientColor;
    Light lights[5]; // Max # of lights (must change before going over)
}

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);

SamplerState BasicSampler : register(s0); // "s" registers for samplers

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(1, 1, 1, 1) // White

	//float2 uvScale = float2(5, 5); // Multiple tiles of same texture
	//float2 uvOffset = float2(time, 0); // Texture looks like it moves

	// Interpolation of normals across a triangle face results in non-unit vectors, so normalize
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent); // Normalize to ensure orthogonal (90 degrees apart)
	
	// Adjust the uv coords by scale & offset for repeating textures
    input.uv = input.uv * uvScale + uvOffset;
	
	// Add normal map for the appearence of depth	
	// Unpack the normal (Convert it from [0 – 1] (how colors are stored in textures) to [-1 – 1] range 
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal); // Don’t forget to normalize!
	
	// Create the 3x3 rotation matrix for converting tangent to world space
    float3 N = input.normal; 
    float3 T = normalize(input.tangent - N * dot(input.tangent, N)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);

	// Assumes that input.normal is the normal later in the shader
    input.normal = mul(unpackedNormal, TBN); // Note multiplication order!

	// Sample the texture color and apply the tint
    float4 textureColor = SurfaceTexture.Sample(BasicSampler, input.uv);
    textureColor *= colorTint;
    //return textureColor;
	
	// Apply the ambient lighting to the surface color
	float3 totalLight = ambientColor * textureColor.xyz;
	
	// Loop thru & add all the lights
    for (int i = 0; i < 5; i++)
    {
		// Make extra sure the light's direction is normalized
		Light light = lights[i];
		light.Direction = normalize(light.Direction);
		
        switch (light.Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += DirectLight(light, input.normal, currentCamPos, input.worldPosition, roughness, textureColor.xyz);
                break;
			
            case LIGHT_TYPE_POINT:
                totalLight += PointLight(light, input.normal, currentCamPos, input.worldPosition, roughness, textureColor.xyz);
                break;
			
            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(light, input.normal, currentCamPos, input.worldPosition, roughness, textureColor.xyz);
                break;
        }
    }
	
    return float4(totalLight, 1);
	
	//return input.color;
	//return float4(input.normal, 1);
	//return float4(input.uv, 0, 1);
    //return colorTint;
}