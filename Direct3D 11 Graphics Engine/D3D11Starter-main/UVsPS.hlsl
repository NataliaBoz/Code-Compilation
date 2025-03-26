// Struct of data from earlier in the pipeline
struct VertexToPixel
{
    float4 screenPosition   : SV_POSITION;
    float2 uv               : TEXCOORD;
    float3 normal           : NORMAL;
};

// Return the UV data as RGBA color values for each pixel
float4 main(VertexToPixel input) : SV_TARGET
{	
	return float4(input.uv, 0, 1); 
}