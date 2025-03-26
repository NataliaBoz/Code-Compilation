
// Constant buffer (every vertex gets/reads same data from buffer) (Name is irrevelant)
cbuffer ExternalData : register(b0) // b0-b14 of buffer indeices
{
	// *NOTE: Order listed matters!
	matrix world; //float3 offset; //matrix transform;
	matrix view;
	matrix projection;
	matrix worldInvTransp;
	
}

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
	float3 localPosition	: POSITION;     // XYZ position
	float2 uv				: TEXCOORD; //float4 color			: COLOR;        // RGBA color
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
	//float4 color			: COLOR;        // RGBA color
	float2 uv				: TEXCOORD; 
	float3 normal			: NORMAL;
	//float3 worldPos			: POSITION;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	
	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
	//output.screenPosition = float4(input.localPosition, 1.0f);
	//output.screenPosition = mul(transform, float4(input.localPosition, 1.0f));
	//output.screenPosition = float4(input.localPosition + offset, 1.0f); // Moves all vertices off by the offset
    // Multiply the three matrices together first
    matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	//output.worldPos = mul(world, float4(input.localPosition, 1.0f));

	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
	// - We don't need to alter it here, but we do need to send it to the pixel shader
	//output.color = input.color;
	//output.color = input.color * 2; // Makes the colors brighter!
	//output.color = input.color * colorTint; // Tints all the colors 
    //output.color = colorTint; 

	output.uv = input.uv; 
	output.normal = input.normal;
	//output.normal = normalize(mul((float3x3)worldInvTransp, input.normal)); // Treating normal as lighting instead of just color
	//output.tangent = normalize(mul((float3x3)world, input.tangent));

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}