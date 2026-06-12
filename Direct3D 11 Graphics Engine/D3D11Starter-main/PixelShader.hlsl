#include "ShaderInclude.hlsli" // Contains all necessary structs, helper functions, etc.

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
	float3 currentCamPos;
    //float3 ambientColor;
    Light lights[6]; // Max # of lights (must change before going over)
}

Texture2D SurfaceTexture : register(t0); // Albedo
Texture2D NormalMap : register(t1); // "t" registers for textures
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);

SamplerState BasicSampler : register(s0); // "s" registers for samplers
SamplerComparisonState ShadowSampler : register(s1);

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
	// Perform the perspective divide (divide by W) ourselves
    input.shadowMapPos /= input.shadowMapPos.w;
    
    // Convert the normalized device coordinates to UVs for sampling
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; // Flip the Y
    
    // Grab the distances we need: light-to-pixel and closest-surface
    float distToLight = input.shadowMapPos.z;
    // Get a ratio of comparison results using SampleCmpLevelZero()
    float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, distToLight).r;
    //float distShadowMap = ShadowMap.Sample(BasicSampler, shadowUV).r;
    
    // For testing, just return black where there are shadows.
    /*if (distShadowMap < distToLight)
        return float4(0, 0, 0, 1);*/
	
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(1, 1, 1, 1) // White

	//float2 uvScale = float2(5, 5); // Multiple tiles of same texture
	//float2 uvOffset = float2(time, 0); // Texture looks like it moves

	// Perspective divide shadow map
	//input.shadowMapPos.xyz /= input.shadowMapPos.w;
	//float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
	//shadowUV.y = 1.0f - shadowUV.y;

	////float shadowMapDist = ShadowMap.Sample(BasicSampler, shadowUV).r;
	//float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, inputshadowMapPos.z);

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
    
	// Grab the red channel after sampling the roughness & metalness textures
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    
	// Sample the texture color & apply the tint for lighting 
    float4 textureColor = pow(SurfaceTexture.Sample(BasicSampler, input.uv).rgba, 2.2f);
    //return textureColor;
    float3 totalLight = 0;
	
	// Specular color determination -----------------
	// Assume albedo texture is actually holding specular color where metalness == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, textureColor.rgb, metalness);
	
	// Apply the ambient lighting to the surface color
	//float3 totalLight = ambientColor * textureColor.xyz;
    
	// Loop thru & add all the lights
    for (int i = 0; i < 6; i++)
    {
		// Make extra sure the light's direction is normalized
		Light light = lights[i];
		light.Direction = normalize(light.Direction);
		
        switch (light.Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                // Calculate directional light
                float3 lightResult = DirectLight(light, input.normal, currentCamPos, input.worldPosition, roughness, metalness, textureColor.xyz, specularColor);

                // If this is the first light, apply the shadowing result
                if (i == 0)
                {
                    lightResult *= shadowAmount;
                }
                // Add this light's result to the total light for this pixel
                totalLight += lightResult;
                break;
			
            case LIGHT_TYPE_POINT:
                totalLight += PointLight(light, input.normal, currentCamPos, input.worldPosition, roughness, metalness, textureColor.xyz, specularColor);
                break;
			
            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(light, input.normal, currentCamPos, input.worldPosition, roughness, metalness, textureColor.xyz, specularColor);
                break;
        }
    }
	
    return float4(pow(totalLight, 1.0f / 2.2f), 1); // With gamma correction
    //return float4(totalLight, 1);
	
	//return input.color;
	//return float4(input.normal, 1);
	//return float4(input.uv, 0, 1);
    //return colorTint;
}