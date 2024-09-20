#include "Shared.fxh"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
    return txDiffuse.Sample(samLinear, input.Tex);
    //return float4(1.f, 0.f, 0.f, 1.0f);

}
