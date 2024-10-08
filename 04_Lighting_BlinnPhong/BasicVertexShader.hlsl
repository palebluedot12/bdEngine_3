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
    
    // World Matrix���� �̵������� �����ϰ� �����ϸ�,  scale ������ �����Ƿ� normalize ����Ѵ�.
    output.Norm = normalize(mul(input.Norm, (float3x3) World));
    
    // �ؽ�ó ��ǥ�� �״�� ����
    output.Tex = input.Tex;
    
    return output;
}