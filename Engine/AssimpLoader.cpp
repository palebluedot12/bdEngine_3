#include "pch.h"
#include "AssimpLoader.h"
#include <filesystem>
#include <Directxtk/DDSTextureLoader.h>
#include <DirectXTex.h>

AssimpLoader::AssimpLoader(ID3D11Device* device, ID3D11DeviceContext* deviceContext) 
    :m_pDevice(device), m_pDeviceContext(deviceContext)
{
}

AssimpLoader::~AssimpLoader() 
{
    for (auto& texture : m_MeshTextures)
    {
        SAFE_RELEASE(texture.diffuseTextureRV);
        SAFE_RELEASE(texture.normalTextureRV);
        SAFE_RELEASE(texture.specularTextureRV);
    }
}

bool AssimpLoader::saveEmbeddedTexture(const aiTexture* embeddedTexture, const std::string& directory_)
{
    if (!embeddedTexture)
        return false;

    std::filesystem::path path = embeddedTexture->mFilename.C_Str();
    std::string filename = directory_ + "/" + path.filename().string();


    if (embeddedTexture->mHeight == 0)
    {
        // Save a compressed texture of mWidth bytes
        std::ofstream file(filename.c_str(), std::ios::binary);
        file.write(reinterpret_cast<const char*>(embeddedTexture->pcData), embeddedTexture->mWidth);
        file.close();
    }
    else
    {
        // Save an uncompressed ARGB8888 embedded texture
        std::ofstream file(filename.c_str(), std::ios::binary);
        file.write(reinterpret_cast<const char*>(embeddedTexture->pcData), embeddedTexture->mWidth * embeddedTexture->mHeight * 4);
        file.close();
    }
    return true;
}

bool AssimpLoader::LoadModel(const std::string& filePath, Matrix initialTransform)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        return false;
    }

    ProcessNode(scene->mRootNode, scene, initialTransform);
    return true;
}

void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, Matrix parentTransform)
{

    std::vector<Vertex>* vertices = new std::vector<Vertex>();
    std::vector<unsigned short>* indices = new std::vector<unsigned short>();

    // vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // positions
        vertex.Pos.x = mesh->mVertices[i].x;
        vertex.Pos.y = mesh->mVertices[i].y;
        vertex.Pos.z = mesh->mVertices[i].z;

        // normals
        vertex.Normal.x = mesh->mNormals[i].x;
        vertex.Normal.y = mesh->mNormals[i].y;
        vertex.Normal.z = mesh->mNormals[i].z;

        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            vertex.Tex.x = mesh->mTextureCoords[0][i].x;
            vertex.Tex.y = mesh->mTextureCoords[0][i].y;
        }
        else {
            vertex.Tex = Vector2(0.0f, 0.0f);
        }

        // tangents
        if (mesh->mTangents)
        {
            vertex.Tangent.x = mesh->mTangents[i].x;
            vertex.Tangent.y = mesh->mTangents[i].y;
            vertex.Tangent.z = mesh->mTangents[i].z;
        }

        vertices->push_back(vertex);
    }

    // indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices->push_back(face.mIndices[j]);
        }
    }

    // materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    Material newMaterial;
    aiString texturePath;
    MeshTexture meshTexture;

    std::string baseTexturePath = "../Resource/";

    // Diffuse Texture
    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) 
    {
        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') 
        {
            int textureIndex = atoi(texturePath.C_Str() + 1);
            if (textureIndex < scene->mNumTextures) 
            {
                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
                if (embeddedTexture) 
                {
                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) 
                    {
                        newMaterial.diffuseTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
                    }
                }
            }
        }
        else 
        {
            std::filesystem::path fullTexturePath = texturePath.C_Str();
            newMaterial.diffuseTexturePath = baseTexturePath + fullTexturePath.filename().string();  // 파일 이름만 추출
        }

        HRESULT hr = S_OK;
        std::wstring texturePathStr = StringToWString(newMaterial.diffuseTexturePath);
        if (texturePathStr.rfind(L".dds") != std::wstring::npos)
            hr = CreateDDSTextureFromFile(m_pDevice, texturePathStr.c_str(), nullptr, &meshTexture.diffuseTextureRV);
        else if (texturePathStr.rfind(L".png") != std::wstring::npos)
            hr = CreateTextureFromPng(m_pDevice, m_pDeviceContext, texturePathStr.c_str(), &meshTexture.diffuseTextureRV);
       
    }

    // Normal Texture
    if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) 
    {
        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') 
        {
            int textureIndex = atoi(texturePath.C_Str() + 1);
            if (textureIndex < scene->mNumTextures) 
            {
                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
                if (embeddedTexture) 
                {
                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) 
                    {
                        newMaterial.normalTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
                    }
                }
            }
        }
        else 
        {
            std::filesystem::path fullTexturePath = m_ModelDirectory;
            fullTexturePath /= texturePath.C_Str();
            newMaterial.normalTexturePath = std::filesystem::absolute(fullTexturePath).string(); // 절대 경로로 변경
        }


    }
    else 
    {
        newMaterial.normalTexturePath = ""; // 텍스처가 없으면 빈 문자열로 설정
    }

    // Specular Texture
    if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) 
    {
        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') 
        {
            int textureIndex = atoi(texturePath.C_Str() + 1);
            if (textureIndex < scene->mNumTextures) 
            {
                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
                if (embeddedTexture) 
                {
                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) 
                    {
                        newMaterial.specularTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
                    }
                }
            }
        }
        else 
        {
            std::filesystem::path fullTexturePath = m_ModelDirectory;
            fullTexturePath /= texturePath.C_Str();
            newMaterial.specularTexturePath = std::filesystem::absolute(fullTexturePath).string(); // 절대 경로로 변경
        }
    }
    else 
    {
        newMaterial.specularTexturePath = ""; // 텍스처가 없으면 빈 문자열로 설정
    }

    m_Materials.push_back(newMaterial);
    m_MeshTextures.push_back(meshTexture);
    m_Meshes.push_back(new Mesh(vertices, indices, parentTransform));

}

// node의 행렬을 계산하고 자식 노드에게 해당 행렬을 넘겨준다
void AssimpLoader::ProcessNode(aiNode* node, const aiScene* scene, Matrix parentTransform)
{
    Matrix transform = parentTransform;
    aiMatrix4x4 nodeTransform = node->mTransformation;
    transform = XMMatrixMultiply(XMMatrixSet(nodeTransform.a1, nodeTransform.b1, nodeTransform.c1, nodeTransform.d1,
        nodeTransform.a2, nodeTransform.b2, nodeTransform.c2, nodeTransform.d2,
        nodeTransform.a3, nodeTransform.b3, nodeTransform.c3, nodeTransform.d3,
        nodeTransform.a4, nodeTransform.b4, nodeTransform.c4, nodeTransform.d4), transform);

    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene, transform);
    }

    // Then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, transform);
    }
}

HRESULT AssimpLoader::CreateTextureFromPng(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename, ID3D11ShaderResourceView** textureView)
{
    HRESULT hr = S_OK;
    DirectX::ScratchImage image;

    hr = LoadFromWICFile(filename, DirectX::WIC_FLAGS_NONE, nullptr, image);
    if (FAILED(hr)) {
        return hr;
    }

    DirectX::TexMetadata metadata = image.GetMetadata();

    DirectX::ScratchImage d3dImage;
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, DirectX::TEX_FILTER_DEFAULT, 0, d3dImage);
    if (FAILED(hr)) {
        return hr;
    }

    hr = DirectX::CreateShaderResourceView(device, d3dImage.GetImages(), d3dImage.GetImageCount(), d3dImage.GetMetadata(), textureView);

    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}

std::vector<Mesh*> AssimpLoader::GetMeshes() const
{
    return m_Meshes;
}

std::vector<Material> AssimpLoader::GetMaterials() const
{
    return m_Materials;
}

std::vector<MeshTexture> AssimpLoader::GetMeshTextures() const
{
    return m_MeshTextures;
}