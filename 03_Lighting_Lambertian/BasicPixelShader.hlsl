//#include "Shared.fxh"

////--------------------------------------------------------------------------------------
//// Pixel Shader
////--------------------------------------------------------------------------------------
//float4 main(PS_INPUT input) : SV_Target
//{
//    return txDiffuse.Sample(samLinear, input.Tex);
//    //return float4(1.f, 0.f, 0.f, 1.0f);

//}

#include "Shared.fxh"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
    // �ؽ�ó ���� ���ø�
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    
    // ����ȭ�� ��� ����
    float3 normal = normalize(input.Norm);

    // �� ���� ������ ����
    float3 lightDir1 = normalize(vLightDir[0].xyz); // ù ��° ������ ����
    float3 lightDir2 = normalize(vLightDir[1].xyz); // �� ��° ������ ����
    
    // ù ��° ����� ��� ������ ���� ���
    float diff1 = saturate(dot(normal, lightDir1));
    float3 diffuseColor1 = diff1 * vLightColor[0].rgb;

    // �� ��° ����� ��� ������ ���� ���
    float diff2 = saturate(dot(normal, lightDir2));
    float3 diffuseColor2 = diff2 * vLightColor[1].rgb;

    // �� ������ ��ǻ�� ������ �ջ�
    float3 totalDiffuse = diffuseColor1 + diffuseColor2;

    // ���� ���� = �ؽ�ó ���� * ���� ����
    color.rgb *= totalDiffuse;

    // Output color
    return color;
}

