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
   
    // World Matrix���� �̵������� �����ϰ� �����ϸ�, scale ������ �����Ƿ� normalize ����Ѵ�.
    output.Norm = normalize(mul(input.Norm, (float3x3) World));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) World));
    
    // �ؽ�ó ��ǥ�� �״�� ����
    output.Tex = input.Tex;
    
    return output;
}