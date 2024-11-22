//--------------------------------------------------------------------------------------
// File: Tutorial06.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txSpecular : register(t2);

SamplerState samLinear : register(s0);
//SamplerState samNormal : register(s1);
//SamplerState samSpecular : register(s2);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    
    float4 vLightDir;
    float4 vLightAmbient;
    float4 vLightDiffuse;
    float4 vLightSpecular;
    float4 vMaterialAmbient;
    float4 vMaterialDiffuse;
    float4 vMaterialSpecular;
    float3 vCameraPos;
    float fMaterialSpecularPower; 
};

cbuffer BoolBuffer : register(b1)
{
    bool useNormalMap;
    float3 padN;
    bool useSpecular;
    float3 padS;
}
// POSITION, TANGENT, BINORMAL, NORMAL, TEXCOORD.... => semantic
// semantic은 GPU에게 해당 변수가 어떤 역할을 하는지 알려주는 역할을 함.
// https://learn.microsoft.com/ko-kr/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Tangent : TANGENT;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Tangent : TANGENT;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD;
    float4 WorldPos : TEXCOORD1;
};