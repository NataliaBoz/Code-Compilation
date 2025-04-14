// Preprocessor include guard
#ifndef __SHADER_INCLUDE__ // Each .hlsli file needs a unique identifier!
#define __SHADER_INCLUDE__

// ALL the code pieces (structs, helper functions, static const variables etc.) go here!
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_SPECULAR_EXPONENT 256.0f

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float2 uv : TEXCOORD; //float4 color			: COLOR;        // RGBA color
    float3 normal : NORMAL;
    float3 tangent : TANGENT; // Can be used to compute the bi-tangent vector as well
};

// Struct representing the data we're sending down the pipeline
// - The output of our corresponding vertex shader should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
	//float4 color : COLOR;        // RGBA color
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT; 
	float3 worldPosition : POSITION;
};

// VertexToPixel struct for sky
struct VertexToPixel_Sky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

// Lights
struct Light
{
	// *NOTE: Order important here! Don't cross 16 byte boundary!*
    int Type; // Which kind of light? 0, 1 or 2 (see above)
    float3 Direction; // Directional and Spot lights need a direction
    float Range; // Point and Spot lights have a max range for attenuation
    float3 Position; // Point and Spot lights have a position in space
    float Intensity; // All lights need an intensity
    float3 Color; // All lights need a color
    float SpotInnerAngle; // Inner cone angle (in radians) – Inside this, full light!
    float SpotOuterAngle; // Outer cone angle (radians) – Outside this, no light!
    float2 Padding; // Purposefully padding to hit the 16-byte boundary
};

/* Lighting Helper Functions: */

float DiffuseLighting(float3 normal, float3 directToLight)
{   
    return saturate(dot(normal, directToLight)); // Make sure non-neg
}

// Phong BRDF
float SpecularReflect(float3 reflectVect, float3 viewVector, float roughness)
{
    // The bigger the power (Ex: 64), the smaller the shine
	// To replicate a fully white shine, don't mulyiply by textureColor
    return pow(max(dot(reflectVect, viewVector), 0), (1.0f - roughness) * MAX_SPECULAR_EXPONENT);
}

// Decrease light as it gets further away
float Attenuate(Light light, float3 worldPos)
{
    // Distance btw surface and the light
    float surfdistance = distance(light.Position, worldPos);

    float attenuation = (1.0f - (surfdistance * surfdistance / (light.Range * light.Range))); // Range-based 
    
	// Soft falloff
    return saturate(attenuation * attenuation); 
}

// Directional Light
float3 DirectLight(Light light, float3 normal, float3 cameraPos, float3 worldPos, float roughness, float3 surfaceColor)
{
    float diffuseTerm = DiffuseLighting(normal, normalize(-light.Direction));
	
    float3 reflectVect = reflect(-(normalize(-light.Direction)), normal);
    float3 viewVector = normalize(cameraPos - worldPos);
	
    float specularTerm = SpecularReflect(reflectVect, viewVector, roughness);

    //return (diffuseTerm + specularTerm) * light.Intensity * light.Color * textureColor.xyz; // Spec Reflect tinted
    return (specularTerm + diffuseTerm * surfaceColor) * light.Intensity * light.Color; // Spec Reflect NOT tinted
}

// Point Light
float3 PointLight(Light light, float3 normal, float3 cameraPos, float3 worldPos, float roughness, float3 surfaceColor)
{
    float attenuate = Attenuate(light, worldPos);
    
    //attenuate = 1; // *NOTE: Set to 0 = NO Light, Set to - = shining darkness*
        
    float diffuseTerm = DiffuseLighting(normal, normalize(light.Position - worldPos));
	
    float3 reflectVect = reflect(-(normalize(light.Position - worldPos)), normal);
    float3 viewVector = normalize(cameraPos - worldPos);
	
    float specularTerm = SpecularReflect(reflectVect, viewVector, roughness);
	
    //return (diffuseTerm + specularTerm) * light.Intensity * attenuate * light.Color * surfaceColor; // Spec Reflect tinted
    return (specularTerm + diffuseTerm * surfaceColor) * attenuate * light.Intensity * light.Color; // Spec Reflect NOT tinted
}

// Spot Light
float3 SpotLight(Light light, float3 normal, float3 cameraPos, float3 worldPos, float roughness, float3 surfaceColor)
{
    // Get cos(angle) between pixel and light direction
    float pixelAngle = saturate(dot(-(normalize(light.Position - worldPos)), light.Direction));
    
    // Get cosines of angles and calculate range
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    
    // Linear falloff over the range, clamp 0-1, apply to light calc
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    
    // Adjust a point light by the spotTerm to get a spot light
    return PointLight(light, normal, cameraPos, worldPos, roughness, surfaceColor) * spotTerm;
}

#endif