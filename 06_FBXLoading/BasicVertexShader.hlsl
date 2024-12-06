#include "Shared.fxh"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.WorldPos = mul(input.Pos, World);
    output.Pos = mul(output.WorldPos, View);
    output.Pos = mul(output.Pos, Projection);
   
    // World Matrix에서 이동성분을 제외하고 적용하며, scale 있을수 있으므로 normalize 사용한다.
    output.Norm = normalize(mul(input.Norm, (float3x3) World));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) World));
    
    // 텍스처 좌표를 그대로 전달
    output.Tex = input.Tex;
    
    return output;
}