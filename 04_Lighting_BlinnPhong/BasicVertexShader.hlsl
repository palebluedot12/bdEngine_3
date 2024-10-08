#include "Shared.fxh"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    
    //output.Norm = mul(float4(input.Norm, 1), World).xyz;
    
    // World Matrix에서 이동성분을 제외하고 적용하며,  scale 있을수 있으므로 normalize 사용한다.
    output.Norm = normalize(mul(input.Norm, (float3x3) World));
    
    // 텍스처 좌표를 그대로 전달
    output.Tex = input.Tex;
    
    return output;
}