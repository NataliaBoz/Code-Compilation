// Post Process Effect: Bloom - Extract step
cbuffer externalData : register(b0)
{
    float brightnessThreshold;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D Pixels : register(t0);
SamplerState BloomSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	// Grab the color of this pixel
    float3 pixelColor = Pixels.Sample(BloomSampler, input.uv).rgb;

    // Calculate the luminance ("brightness") of a pixel and compare that against the threshold.
    float luminance = dot(pixelColor, float3(0.2126, 0.7152, 0.0722));

	// Return this pixel's color if its luminance is above the threshold
    return float4(luminance >= brightnessThreshold ? pixelColor : float3(0, 0, 0), 1);
}