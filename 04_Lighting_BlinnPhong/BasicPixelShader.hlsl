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
    // 텍스처 색상 샘플링
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    
    float3 normal = normalize(input.Norm);

    float3 lightDir1 = normalize(vLightDir[0].xyz); // 첫 번째 조명의 방향
    float3 lightDir2 = normalize(vLightDir[1].xyz); // 두 번째 조명의 방향
    
    // 첫 번째 조명과 normal의 내적
    float diff1 = saturate(dot(normal, lightDir1));
    float3 diffuseColor1 = diff1 * vLightColor[0].rgb;

    // 두 번째 조명과...
    float diff2 = saturate(dot(normal, lightDir2));
    float3 diffuseColor2 = diff2 * vLightColor[1].rgb;

    // 두 조명의 diffuse 합산
    float3 totalDiffuse = diffuseColor1 + diffuseColor2;

    // 최종 = 텍스처 색상 * 조명 색상
    color.rgb *= totalDiffuse;

    // Output color
    return color;
}

