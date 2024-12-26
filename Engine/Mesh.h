#pragma once
#include <vector>
#include "Helper.h"

struct Vertex
{
    Vector3 Pos;
    Vector3 Tangent;
    Vector3 Normal;
    Vector2 Tex;
};

struct MeshTexture
{
    ID3D11ShaderResourceView* diffuseTextureRV = nullptr;
    ID3D11ShaderResourceView* normalTextureRV = nullptr;
    ID3D11ShaderResourceView* specularTextureRV = nullptr;
};
 
class Mesh 
{
public:
    Mesh(std::vector<Vertex>* vertices, std::vector<unsigned short>* indices, Matrix transform);
    ~Mesh();
    const std::vector<Vertex>& GetVertices() const;
    const std::vector<unsigned short>& GetIndices() const;
    const Matrix GetTransform() const;

private:
    std::vector<Vertex>* m_Vertices;
    std::vector<unsigned short>* m_Indices;
    Matrix m_Transform;


private:
    Mesh(const Mesh& other) = delete;
    Mesh& operator=(const Mesh& other) = delete;
};