#include "pch.h"
#include "FBXRenderer.h"
#include "..\\Engine\\Helper.h"
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>

FBXRenderer::FBXRenderer(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int width, int height)
    :m_pDevice(device), m_pDeviceContext(deviceContext), m_LightDirection(0.0f, 0.0f, 1.0f),
    m_LightAmbient(0.0f, 0.0f, 0.0f, 1.0f), m_LightDiffuse(1.0f, 1.0f, 1.0f, 1.0f), m_LightSpecular(1.0f, 1.0f, 1.0f, 1.0f),
    m_MaterialAmbient(1.0f, 1.0f, 1.0f, 1.0f), m_MaterialDiffuse(1.0f, 1.0f, 1.0f, 1.0f), m_MaterialSpecular(1.0f, 1.0f, 1.0f, 1.0f), m_MaterialSpecularPower(2000.0f),
    m_CubeScale(1.0f, 1.0f, 1.0f), m_CubeRotation(0.0f, 0.0f, 0.0f), m_bSpecularMapEnabled(true)
{
    Initialize();

}
FBXRenderer::~FBXRenderer()
{
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pVertexShader);
    SAFE_RELEASE(m_pPixelShader);
    SAFE_RELEASE(m_pInputLayout);
    SAFE_RELEASE(m_pIndexBuffer);
    SAFE_RELEASE(m_pConstantBuffer);
    SAFE_RELEASE(m_pBoolBuffer);
    SAFE_RELEASE(m_pTextureRV);
    SAFE_RELEASE(m_pNormalTextureRV);
    SAFE_RELEASE(m_pSpecularTextureRV);
    SAFE_RELEASE(m_pSamplerLinear);
}

void FBXRenderer::Initialize()
{
    HRESULT hr = 0; // 결과값.
    ID3D10Blob* errorMessage = nullptr;	 // 에러 메시지를 저장할 버퍼.

    // 1. Render() 에서 파이프라인에 바인딩할 InputLayout 생성 	
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3D10Blob* vertexShaderBuffer = nullptr;
    HR_T(CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
    HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
        vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));

    // 2. Render() 에서 파이프라인에 바인딩할  버텍스 셰이더 생성
    HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));

    SAFE_RELEASE(vertexShaderBuffer);

    // 3. Render() 에서 파이프라인에 바인딩할 픽셀 셰이더 생성
    ID3D10Blob* pixelShaderBuffer = nullptr;
    HR_T(CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
    HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
        pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));
    SAFE_RELEASE(pixelShaderBuffer);

    // 4. Render() 에서 파이프라인에 바인딩할 상수 버퍼 생성	
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer));

    bd.ByteWidth = sizeof(BoolBuffer);
    HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pBoolBuffer));

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HR_T(m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear));

    // 초기값설정
    m_World = XMMatrixIdentity();
}

void FBXRenderer::Render(const std::vector<Mesh>& meshes, const std::vector<Material>& materials, Vector3 cameraPos)
{

    if (meshes.empty() || materials.empty()) return;

    // 큐브 회전 행렬
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(m_CubeRotation.x),
        XMConvertToRadians(m_CubeRotation.y),
        XMConvertToRadians(m_CubeRotation.z));

    // 큐브 스케일 행렬
    XMMATRIX scaleMatrix = XMMatrixScaling(m_CubeScale.x, m_CubeScale.y, m_CubeScale.z);

    m_World = scaleMatrix * rotationMatrix;
    m_CameraPos = cameraPos;

    // 월드 공간에서의 카메라 위치
    m_CameraPos = cameraPos;

    for (size_t i = 0; i < meshes.size(); ++i) {

        const Mesh& mesh = meshes[i];
        const Material& material = materials[i];

        const std::vector<Vertex>& vertices = mesh.GetVertices();
        const std::vector<unsigned short>& indices = mesh.GetIndices();

        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = sizeof(Vertex) * vertices.size();
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA vbData = {};
        vbData.pSysMem = vertices.data();
        HR_T(m_pDevice->CreateBuffer(&bd, &vbData, &m_pVertexBuffer));

        // 버텍스 버퍼 바인딩.
        m_VertexBufferStride = sizeof(Vertex);
        m_VertexBufferOffset = 0;


        D3D11_BUFFER_DESC ibd = {};
        ibd.ByteWidth = sizeof(unsigned short) * indices.size();
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.Usage = D3D11_USAGE_DEFAULT;
        ibd.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA ibData = {};
        ibData.pSysMem = indices.data();
        HR_T(m_pDevice->CreateBuffer(&ibd, &ibData, &m_pIndexBuffer));

        m_nIndices = indices.size();

        // Load the Texture
        if (!material.diffuseTexturePath.empty())
            HR_T(CreateDDSTextureFromFile(m_pDevice, StringToWString(material.diffuseTexturePath).c_str(), nullptr, &m_pTextureRV));
        else {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            m_pDeviceContext->PSSetShaderResources(0, 1, &nullSRV);
        }

        // Normal
        if (!material.normalTexturePath.empty())
            HR_T(CreateDDSTextureFromFile(m_pDevice, StringToWString(material.normalTexturePath).c_str(), nullptr, &m_pNormalTextureRV));
        else {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            m_pDeviceContext->PSSetShaderResources(1, 1, &nullSRV);
        }

        // Specular
        if (!material.specularTexturePath.empty())
            HR_T(CreateDDSTextureFromFile(m_pDevice, StringToWString(material.specularTexturePath).c_str(), nullptr, &m_pSpecularTextureRV));
        else {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            m_pDeviceContext->PSSetShaderResources(2, 1, &nullSRV);
        }

        // Render the cube
        m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
        m_pDeviceContext->IASetInputLayout(m_pInputLayout);
        m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
        m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
        m_pDeviceContext->VSSetConstantBuffers(1, 1, &m_pBoolBuffer);
        m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
        m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
        m_pDeviceContext->PSSetConstantBuffers(1, 1, &m_pBoolBuffer);
        m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureRV);
        m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

        // Normal On/Off
        // On/Off 여기서 처리하지 않고 Pixel Shader에서 하는 방식으로 변경
        m_pDeviceContext->PSSetShaderResources(1, 1, &m_pNormalTextureRV);

        // SpecularMap On/Off
        if (m_bSpecularMapEnabled)
        {
            m_pDeviceContext->PSSetShaderResources(2, 1, &m_pSpecularTextureRV);
        }
        else
        {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            m_pDeviceContext->PSSetShaderResources(2, 1, &nullSRV);  // Unbind the specular map
        }

        // Update matrix variables and lighting variables
        ConstantBuffer cb1;
        cb1.mWorld = XMMatrixTranspose(m_World);
        cb1.mView = XMMatrixTranspose(m_View);
        cb1.mProjection = XMMatrixTranspose(m_Projection);
        cb1.vLightDir = XMFLOAT4(m_LightDirection.x, m_LightDirection.y, m_LightDirection.z, 1.0f);
        cb1.vLightAmbient = m_LightAmbient;
        cb1.vLightDiffuse = m_LightDiffuse;
        cb1.vLightSpecular = m_LightSpecular;
        cb1.vMaterialAmbient = m_MaterialAmbient;
        cb1.vMaterialDiffuse = m_MaterialDiffuse;
        cb1.vMaterialSpecular = m_MaterialSpecular;
        cb1.fMaterialSpecularPower = m_MaterialSpecularPower;
        cb1.vCameraPos = m_CameraPos;

        m_pDeviceContext->UpdateSubresource(m_pBoolBuffer, 0, nullptr, &boolbuffer, 0, 0);
        m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
        m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);

        SAFE_RELEASE(m_pVertexBuffer);
        SAFE_RELEASE(m_pIndexBuffer);
    }

}

void FBXRenderer::SetView(Matrix view)
{
    m_View = view;
}

void FBXRenderer::SetProjection(Matrix projection)
{
    m_Projection = projection;
}

void FBXRenderer::SetLightDirection(Vector3 lightDir)
{
    m_LightDirection = lightDir;
}

void FBXRenderer::SetLightAmbient(Vector4 lightAmbient)
{
    m_LightAmbient = lightAmbient;
}

void FBXRenderer::SetLightDiffuse(Vector4 lightDiffuse)
{
    m_LightDiffuse = lightDiffuse;
}

void FBXRenderer::SetLightSpecular(Vector4 lightSpecular)
{
    m_LightSpecular = lightSpecular;
}

void FBXRenderer::SetMaterialAmbient(Vector4 materialAmbient)
{
    m_MaterialAmbient = materialAmbient;
}

void FBXRenderer::SetMaterialDiffuse(Vector4 materialDiffuse)
{
    m_MaterialDiffuse = materialDiffuse;
}

void FBXRenderer::SetMaterialSpecular(Vector4 materialSpecular)
{
    m_MaterialSpecular = materialSpecular;
}

void FBXRenderer::SetMaterialSpecularPower(float materialSpecularPower)
{
    m_MaterialSpecularPower = materialSpecularPower;
}

void FBXRenderer::SetCubeScale(Vector3 scale)
{
    m_CubeScale = scale;
}

void FBXRenderer::SetCubeRotation(Vector3 rotation)
{
    m_CubeRotation = rotation;
}

void FBXRenderer::SetUseNormalMap(bool useNormalMap) {
    boolbuffer.useNormalMap = useNormalMap;
}

void FBXRenderer::SetSpecularMapEnabled(bool specularMapEnabled) {
    m_bSpecularMapEnabled = specularMapEnabled;
}