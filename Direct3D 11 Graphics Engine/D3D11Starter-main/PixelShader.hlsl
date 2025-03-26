cbuffer ExternalData : register(b0)
{
	float4 colorTint;
    float2 uvScale;
    float2 uvOffset;
	//float3 cameraPos;
}

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	//float4 color			: COLOR;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

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

	//input.normal = normalize(...);
	
	// Adjust the uv coords by scale & offset for repeating textures
    input.uv = input.uv * uvScale + uvOffset;

	// Sample the texture color and apply the tint
    float4 textureColor = SurfaceTexture.Sample(BasicSampler, input.uv);
    textureColor *= colorTint;
    return textureColor;

	// *** Will have to make adjustments to this ***
	/*float3 totalLight = float3(0, 0, 0);
	float3 lightColor = float3(1, 1, 1);
	float lightIntens = 1.0f;
	float3 lightDir = float3(1, 0, 0);

	// Diffuse calc
	float3 diffuseTerm =
		max(dot(input.normal, -lightDir), 0) * lightIntens * lightColor  * textureColor; // *** Need to be normalized ***

	// Specular calc
	float3 reflect = reflect(lightDir, input.normal);
	float3 viewVector = normalize(camperaPosition - input.worldPos);

	float3 specularTerm = pow(max(dot(reflect, viewVector), 0), 64) * lightIntens * lightColor * textureColor; // The bigger the power, the smaller the shine
	// To replicate a fully white shine, don't mulyiply by textureColor

	// Ambient
	float3 ambientColor = float3(0.5f, 0.5f, 0.5f);
	float3 ambientTerm = ambientColor * textureColor;

	// Combine ALL lights
	totalLight += ambientTerm + diffuseTerm + specularTerm;

	return float4(totalLight, 1);*/

	//return input.color;
	//return float4(input.normal, 1);
	//return float4(input.uv, 0, 1);
    //return colorTint;
}