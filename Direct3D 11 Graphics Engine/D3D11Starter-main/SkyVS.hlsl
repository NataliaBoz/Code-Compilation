#include "ShaderInclude.hlsli" // Contains all necessary structs, helper functions, etc.

cbuffer ExternalData : register(b0) 
{
    matrix view;
    matrix projection;	
}

VertexToPixel_Sky main(VertexShaderInput input) 
{
    // Create a variable to be returned
    VertexToPixel_Sky output; 
    
    // Create a copy of the view matrix and set the translation portion of that copy to all zeros
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    // Apply projection & updated view to the input position & save the result as the output position
    output.position = mul(mul(projection, viewNoTranslation), float4(input.localPosition, 1.0f));
    
    // Ensure that the output depth of each vertex will be exactly 1.0 after the shader
    // Set the output position’s Z value equal to its W value to ensure after the automatic 
    // perspective divide that occurs in the rasterizer the depth will end up being 1.0
    output.position.z = output.position.w;
    
    // Figure out the sample direction for this vertex from the center of the object
    output.sampleDir = input.localPosition;
    
    // Return the variable
    return output;
}