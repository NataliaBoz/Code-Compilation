// Preprocessor include guard
#ifndef __SHADER_INCLUDE__ // Each .hlsli file needs a unique identifier!
#define __SHADER_INCLUDE__

// ALL the code pieces (structs, helper functions, static const variables etc.) go here!
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_SPECULAR_EXPONENT 256.0f

// CONSTANTS ===================
// A constant Fresnel value for non-metals (glass and plastic have values of about 0.04)
static const float F0_NON_METAL = 0.04f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

// Handy to have this as a constant
static const float PI = 3.14159265359f;

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
    float4 shadowMapPos : SHADOW_POSITION;
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

// Decrease light as it gets further away
float Attenuate(Light light, float3 worldPos)
{
	// Distance btw surface and the light
    float surfdistance = distance(light.Position, worldPos);

    float attenuation = (1.0f - (surfdistance * surfdistance / (light.Range * light.Range))); // Range-based

	// Soft falloff
    return saturate(attenuation * attenuation);
}

// PBR FUNCTIONS ================
// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}

/* Calculates diffuse amount based on energy conservation */
// diffuse   - Diffuse amount
// F         - Fresnel result from microfacet BRDF
// metalness - surface metalness amount 
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}
 
/* Normal Distribution Function: GGX (Trowbridge-Reitz)  */
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness)
{
	// Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	// Can go to zero if roughness is 0 and NdotH is 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;

	// Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}

/* Fresnel term - Schlick approx. */
// v - View vector
// h - Half vector
// f0 - Value when l = n
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
    float VdotH = saturate(dot(v, h));

	// Final value
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

/* Geometric Shadowing - Schlick-GGX */
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
// n - Normal
// v - View vector
//
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
//
// Full G(n,v,l,a) term = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
	// End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));

	// Final value
	// Note: Numerator should be NdotV (or NdotL depending on parameters).
	// However, these are also in the BRDF's denominator, so they'll cancel!
	// We're leaving them out here AND in the BRDF function as the
	// dot products can get VERY small and cause rounding errors.
    return 1 / (NdotV * (1 - k) + k);
}

/* Cook-Torrance Microfacet BRDF (Specular) */
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - parts of the denominator are canceled out by numerator (see below)
//
// D() - Normal Distribution Function - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 F_out)
{
	// Other vectors
    float3 h = normalize(v + l);

	// Run numerator functions
    float D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
	
	// Pass F out of the function for diffuse balance
    F_out = F;

	// Final specular formula
	// Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
	// canceled out by our G() term.  As such, they have been removed
	// from BOTH places to prevent floating point rounding errors.
    float3 specularResult = (D * F * G) / 4;

	// One last non-obvious requirement: According to the rendering equation,
	// specular must have the same NdotL applied as diffuse!  We'll apply
	// that here so that minimal changes are required elsewhere.
    return specularResult * max(dot(n, l), 0);
}

/* Types of PBR Lights */
// Directional Light
float3 DirectLight(Light light, float3 normal, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
    //float3 reflectVect = reflect(normalize(light.Direction), normal);
    float3 viewVector = normalize(cameraPos - worldPos);
    float3 F;
    
    float diffuseTerm = DiffusePBR(normal, normalize(-light.Direction));
    float3 specularTerm = MicrofacetBRDF(normal, normalize(-light.Direction), viewVector, roughness, specularColor, F);
    
    // Calculate diffuse with energy conservation, including cutting diffuse for metals (Reflected light NOT diffused)
    float3 balancedDiff = DiffuseEnergyConserve(diffuseTerm, F, metalness);
    
    // Combine the final diffuse and specular values for this light
    return (balancedDiff * surfaceColor + specularTerm) * light.Intensity * light.Color;
    //return (diffuseTerm + specularTerm) * light.Intensity * light.Color * textureColor.xyz; // Spec Reflect tinted
    //return (specularTerm + diffuseTerm * surfaceColor) * light.Intensity * light.Color; // Spec Reflect NOT tinted
}

// Point Light
float3 PointLight(Light light, float3 normal, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
    //float3 reflectVect = reflect(-(normalize(light.Position - worldPos)), normal);
    float3 viewVector = normalize(cameraPos - worldPos);
    float3 F;
    
    float attenuate = Attenuate(light, worldPos);
    
    //attenuate = 1; // *NOTE: Set to 0 = NO Light, Set to - = shining darkness*
        
    float diffuseTerm = DiffusePBR(normal, normalize(light.Position - worldPos));

    float3 specularTerm = MicrofacetBRDF(normal, normalize(light.Position - worldPos), viewVector, roughness, specularColor, F);
	
    // Calculate diffuse with energy conservation, including cutting diffuse for metals (Reflected light NOT diffused)
    float3 balancedDiff = DiffuseEnergyConserve(diffuseTerm, specularTerm, metalness);

    // Combine the final diffuse and specular values for this light
    return (balancedDiff * surfaceColor + specularTerm) * attenuate * light.Intensity * light.Color;
    //return (diffuseTerm + specularTerm) * light.Intensity * attenuate * light.Color * surfaceColor; // Spec Reflect tinted
    //return (specularTerm + diffuseTerm * surfaceColor) * attenuate * light.Intensity * light.Color; // Spec Reflect NOT tinted
}

// Spot Light
float3 SpotLight(Light light, float3 normal, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
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
    return PointLight(light, normal, cameraPos, worldPos, roughness, metalness, surfaceColor, specularColor) * spotTerm;
}

#endif