#include "pch.h"
#include "AssimpLoader.h"
#include "..\\Engine\\Logger.h"
#include <filesystem>

AssimpLoader::AssimpLoader() {}

AssimpLoader::~AssimpLoader() {}

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

bool AssimpLoader::LoadModel(const std::string& filePath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        //Logger::write(std::cout, "ERROR::ASSIMP::" + std::string(importer.GetErrorString()), LOG_LEVEL::LOG_LEVEL_ERROR);
        return false;
    }

    ProcessNode(scene->mRootNode, scene);

    return true;
}

void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;

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

        vertices.push_back(vertex);
    }

    // indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    Material newMaterial;
    aiString texturePath;

    std::string baseTexturePath = "Resource/";

    // Diffuse Texture
    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') {
            int textureIndex = atoi(texturePath.C_Str() + 1);
            if (textureIndex < scene->mNumTextures) {
                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
                if (embeddedTexture) {
                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) {
                        newMaterial.diffuseTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
                    }
                }
            }
        }
        else {
            std::filesystem::path fullTexturePath = m_ModelDirectory;
            fullTexturePath /= texturePath.C_Str();
            newMaterial.diffuseTexturePath = std::filesystem::absolute(fullTexturePath).string(); // 절대 경로로 변경

        }
    }
    else {
        newMaterial.diffuseTexturePath = ""; // 텍스처가 없으면 빈 문자열로 설정
    }

    // Normal Texture
    if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') {
            int textureIndex = atoi(texturePath.C_Str() + 1);
            if (textureIndex < scene->mNumTextures) {
                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
                if (embeddedTexture) {
                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) {
                        newMaterial.normalTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
                    }
                }
            }
        }
        else {
            std::filesystem::path fullTexturePath = m_ModelDirectory;
            fullTexturePath /= texturePath.C_Str();
            newMaterial.normalTexturePath = std::filesystem::absolute(fullTexturePath).string(); // 절대 경로로 변경
        }
    }
    else {
        newMaterial.normalTexturePath = ""; // 텍스처가 없으면 빈 문자열로 설정
    }

    // Specular Texture
    if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) {
        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') {
            int textureIndex = atoi(texturePath.C_Str() + 1);
            if (textureIndex < scene->mNumTextures) {
                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
                if (embeddedTexture) {
                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) {
                        newMaterial.specularTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
                    }
                }
            }
        }
        else {
            std::filesystem::path fullTexturePath = m_ModelDirectory;
            fullTexturePath /= texturePath.C_Str();
            newMaterial.specularTexturePath = std::filesystem::absolute(fullTexturePath).string(); // 절대 경로로 변경
        }
    }
    else {
        newMaterial.specularTexturePath = ""; // 텍스처가 없으면 빈 문자열로 설정
    }

    m_Materials.push_back(newMaterial);
    m_Meshes.emplace_back(vertices, indices);
}

//void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
//{
//    // ...
//    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
//
//    Material newMaterial;
//    aiString texturePath;
//
//    std::string baseTexturePath = "Resource/";
//
//    // Diffuse Texture
//    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
//        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') {
//            int textureIndex = atoi(texturePath.C_Str() + 1);
//            if (textureIndex < scene->mNumTextures) {
//                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
//                if (embeddedTexture) {
//                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
//                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) {
//                        newMaterial.diffuseTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
//                    }
//                }
//            }
//        }
//        else {
//            newMaterial.diffuseTexturePath = baseTexturePath + std::string(texturePath.C_Str());
//        }
//    }
//    else {
//        newMaterial.diffuseTexturePath = ""; // 텍스처가 없으면 빈 문자열로 설정
//    }
//
//    // Normal Texture
//    if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
//        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') {
//            int textureIndex = atoi(texturePath.C_Str() + 1);
//            if (textureIndex < scene->mNumTextures) {
//                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
//                if (embeddedTexture) {
//                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
//                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) {
//                        newMaterial.normalTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
//                    }
//                }
//            }
//        }
//        else {
//            newMaterial.normalTexturePath = baseTexturePath + std::string(texturePath.C_Str());
//        }
//    }
//    else {
//        newMaterial.normalTexturePath = ""; // 텍스처가 없으면 빈 문자열로 설정
//    }
//
//
//    // Specular Texture
//    if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) {
//        if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') {
//            int textureIndex = atoi(texturePath.C_Str() + 1);
//            if (textureIndex < scene->mNumTextures) {
//                aiTexture* embeddedTexture = scene->mTextures[textureIndex];
//                if (embeddedTexture) {
//                    std::filesystem::path embeddedTexturePath = embeddedTexture->mFilename.C_Str();
//                    if (saveEmbeddedTexture(embeddedTexture, baseTexturePath)) {
//                        newMaterial.specularTexturePath = baseTexturePath + embeddedTexturePath.filename().string();
//                    }
//                }
//            }
//        }
//        else {
//            newMaterial.specularTexturePath = baseTexturePath + std::string(texturePath.C_Str());
//        }
//    }
//    else {
//        newMaterial.specularTexturePath = ""; // 텍스처가 없으면 빈 문자열로 설정
//    }
//
//
//    m_Materials.push_back(newMaterial);
//}
//
//
//
void AssimpLoader::ProcessNode(aiNode* node, const aiScene* scene)
{
    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
    }

    // Then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

//void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
//{
//    std::vector<Vertex> vertices;
//    std::vector<unsigned short> indices;
//
//    // vertices
//    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
//    {
//        Vertex vertex;
//        // positions
//        vertex.Pos.x = mesh->mVertices[i].x;
//        vertex.Pos.y = mesh->mVertices[i].y;
//        vertex.Pos.z = mesh->mVertices[i].z;
//
//        // normals
//        vertex.Normal.x = mesh->mNormals[i].x;
//        vertex.Normal.y = mesh->mNormals[i].y;
//        vertex.Normal.z = mesh->mNormals[i].z;
//
//        // texture coordinates
//        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
//        {
//            vertex.Tex.x = mesh->mTextureCoords[0][i].x;
//            vertex.Tex.y = mesh->mTextureCoords[0][i].y;
//        }
//        else {
//            vertex.Tex = Vector2(0.0f, 0.0f);
//        }
//
//        // tangents
//        if (mesh->mTangents)
//        {
//            vertex.Tangent.x = mesh->mTangents[i].x;
//            vertex.Tangent.y = mesh->mTangents[i].y;
//            vertex.Tangent.z = mesh->mTangents[i].z;
//        }
//
//        vertices.push_back(vertex);
//    }
//
//    // indices
//    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
//    {
//        aiFace face = mesh->mFaces[i];
//        for (unsigned int j = 0; j < face.mNumIndices; j++)
//        {
//            indices.push_back(face.mIndices[j]);
//        }
//    }
//
//    // materials
//    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
//
//    Material newMaterial;
//    aiString texturePath;
//    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
//        newMaterial.diffuseTexturePath = std::string(texturePath.C_Str());
//    }
//    if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
//        newMaterial.normalTexturePath = std::string(texturePath.C_Str());
//    }
//    if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) {
//        newMaterial.specularTexturePath = std::string(texturePath.C_Str());
//    }
//
//    m_Materials.push_back(newMaterial);
//    m_Meshes.emplace_back(vertices, indices);
//}

std::vector<Mesh> AssimpLoader::GetMeshes() const
{
    return m_Meshes;
}

std::vector<Material> AssimpLoader::GetMaterials() const
{
    return m_Materials;
}