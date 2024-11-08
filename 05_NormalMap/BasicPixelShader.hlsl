#include "Shared.fxh"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

// SV_Target : 픽셀 셰이더의 출력을 나타내는 시맨틱
float4 main(PS_INPUT input) : SV_Target
{
    float4 texColor = txDiffuse.Sample(samLinear, input.Tex);
    
    float3 normal = normalize(input.Norm);
    float3 tangent = normalize(input.Tangent);
    //float3 binormal = normalize(input.BiNormal);
    float3 binormal = normalize(cross(normal, tangent));
    float3 normalFromMap = txNormal.Sample(samLinear, input.Tex).rgb * 2 - 1;
    normal = normalize(mul(normalFromMap, float3x3(tangent, binormal, normal)));
    
    
    float3 viewDir = normalize(vCameraPos - input.WorldPos.xyz);
    float3 lightDir = normalize(-vLightDir.xyz);
    float3 halfVector = normalize(lightDir + viewDir);

   // Ambient
    float3 ambient = vLightAmbient.rgb * vMaterialAmbient.rgb;
    
    // Diffuse
    float diffuseFactor = max(dot(normal, lightDir), 0);
    float3 diffuse = vLightDiffuse.rgb * vMaterialDiffuse.rgb * diffuseFactor;
    
    // Specular
    float specularFactor = pow(max(dot(normal, halfVector), 0), fMaterialSpecularPower);
    float3 specular = (vLightSpecular.rgb * vMaterialSpecular.rgb) * specularFactor * txSpecular.Sample(samLinear, input.Tex).rgb;

    float3 finalColor = ambient + diffuse + specular;
    
    return texColor * float4(finalColor, 1);
}

