#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include "Mesh.h"
#include "Material.h"

using namespace DirectX;

class AssimpLoader {
public:
    AssimpLoader(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    ~AssimpLoader();
    bool saveEmbeddedTexture(const aiTexture* embeddedTexture, const std::string& directory_);
    bool LoadModel(const std::string& filePath);
    void ProcessNode(aiNode* node, const aiScene* scene, Matrix parentTransform = XMMatrixIdentity());
    std::vector<Mesh*> GetMeshes() const;
    std::vector<Material> GetMaterials() const;
    std::vector<MeshTexture> GetMeshTextures() const;

private:
    HRESULT CreateTextureFromPng(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename, ID3D11ShaderResourceView** textureView);
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pDeviceContext = nullptr;

    std::vector<Mesh*> m_Meshes;
    std::vector<Material> m_Materials;
    std::vector<MeshTexture> m_MeshTextures;

    std::string m_ModelDirectory;

    void ProcessMesh(aiMesh* mesh, const aiScene* scene, Matrix parentTransform = XMMatrixIdentity());
    //void LoadTextures(const std::vector<Material>& materials);
};