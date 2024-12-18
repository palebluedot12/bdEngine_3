#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include "Mesh.h"
#include "Material.h"

class AssimpLoader {
public:
    AssimpLoader();
    ~AssimpLoader();
    bool saveEmbeddedTexture(const aiTexture* embeddedTexture, const std::string& directory_);
    bool LoadModel(const std::string& filePath);
    void ProcessNode(aiNode* node, const aiScene* scene);
    std::vector<Mesh> GetMeshes() const;
    std::vector<Material> GetMaterials() const;

private:
    std::vector<Mesh> m_Meshes;
    std::vector<Material> m_Materials;
    std::string m_ModelDirectory;

    void ProcessMesh(aiMesh* mesh, const aiScene* scene);
};