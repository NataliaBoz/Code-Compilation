// Post Process Effect: Gaussian Blur - Used in Bloom PP
cbuffer externalData : register(b0)
{
    float2 pixelUVSize;
    float2 blurDirect;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D pixels : register(t0);
SamplerState samplerOptions : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
#define NUM_SAMPLES 15

	// Gaussian weights & offsets (in pixels) (Sigma = 4 & Kernel size = 15 for weights)
    const float weights[NUM_SAMPLES] = { 0.023089, 0.034587, 0.048689, 0.064408, 0.080066, 0.093531, 0.102673, 0.105915, 0.102673, 0.093531, 0.080066, 0.064408, 0.048689, 0.034587, 0.023089 };
    const float offsets[NUM_SAMPLES] = { -13.5f, -11.5f, -9.5f, -7.5f, -5.5f, -3.5f, -1.5f, 0, 1.5f, 3.5f, 5.5f, 7.5f, 9.5f, 11.5f, 13.5f };

	// Color total as neighboring pixels get sampled
    float3 totalColor = float3(0, 0, 0);

	// Offset of single pixel in desired direction in UV space
    float2 uvOffset = blurDirect * pixelUVSize;

	// Loop thru offsets & sample
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
		// UV of neighboring pixel to sample
        float2 uv = input.uv + (uvOffset * offsets[i]);
        totalColor += pixels.Sample(samplerOptions, uv).rgb * weights[i];
    }
	// Final blurred color
    return float4(totalColor, 1);
}