#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "Mesh.h"
#include "Material.h"

using namespace DirectX;

struct ConstantBuffer
{
    Matrix mWorld;
    Matrix mView;
    Matrix mProjection;

    Vector4 vLightDir;
    Vector4 vLightAmbient;
    Vector4 vLightDiffuse;
    Vector4 vLightSpecular;
    Vector4 vMaterialAmbient;
    Vector4 vMaterialDiffuse;
    Vector4 vMaterialSpecular;
    Vector3 vCameraPos;
    float fMaterialSpecularPower;
};

struct BoolBuffer
{
    bool useNormalMap;
    Vector3 pad1;
    bool useSpecular;
    Vector3 pad2;
};

class FBXRenderer {
public:
    FBXRenderer(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int width, int height);
    ~FBXRenderer();

    void Initialize();
    void ReleaseRenderResources();
    void Render(const std::vector<Mesh*>& meshes, const std::vector<Material>& materials, Vector3 cameraPos);
    void SetView(Matrix view);
    Matrix GetView();
    void SetProjection(Matrix projection);
    Matrix GetProjection();
	void SetWorld(Matrix world);

    void SetLightDirection(Vector3 lightDir);
    void SetLightAmbient(Vector4 lightAmbient);
    void SetLightDiffuse(Vector4 lightDiffuse);
    void SetLightSpecular(Vector4 lightSpecular);

    void SetMaterialAmbient(Vector4 materialAmbient);
    void SetMaterialDiffuse(Vector4 materialDiffuse);
    void SetMaterialSpecular(Vector4 materialSpecular);
    void SetMaterialSpecularPower(float materialSpecularPower);

    void SetCubeScale(Vector3 scale);
    void SetCubeRotation(Vector3 rotation);

    void SetUseNormalMap(bool useNormalMap);
    void SetSpecularMapEnabled(bool specularMapEnabled);
    void SetMeshTextures(const std::vector<MeshTexture>& meshTextures);


private:
    //HRESULT CreateTextureFromPng(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename, ID3D11ShaderResourceView** textureView);
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pDeviceContext = nullptr;

    ID3D11Buffer* m_pVertexBuffer = nullptr;
    ID3D11InputLayout* m_pInputLayout = nullptr;
    ID3D11VertexShader* m_pVertexShader = nullptr;
    ID3D11PixelShader* m_pPixelShader = nullptr;
    ID3D11Buffer* m_pIndexBuffer = nullptr;
    ID3D11Buffer* m_pConstantBuffer = nullptr;
    ID3D11Buffer* m_pBoolBuffer = nullptr;

    //ID3D11ShaderResourceView* m_pTextureRV = nullptr;
    //ID3D11ShaderResourceView* m_pNormalTextureRV = nullptr;
    //ID3D11ShaderResourceView* m_pSpecularTextureRV = nullptr;
    ID3D11SamplerState* m_pSamplerLinear = nullptr;

    UINT m_VertexBufferStride;
    UINT m_VertexBufferOffset;
    UINT m_nIndices;

    Matrix m_World;
    Matrix m_View;
    Matrix m_Projection;

    Vector3 m_LightDirection;
    Vector4 m_LightAmbient;
    Vector4 m_LightDiffuse;
    Vector4 m_LightSpecular;

    Vector4 m_MaterialAmbient;
    Vector4 m_MaterialDiffuse;
    Vector4 m_MaterialSpecular;
    float m_MaterialSpecularPower;

    Vector3 m_CubeScale;
    Vector3 m_CubeRotation;
    Vector3 m_CameraPos;

    BoolBuffer boolbuffer;
    bool m_bSpecularMapEnabled;

    std::vector<MeshTexture> m_MeshTextures;
};