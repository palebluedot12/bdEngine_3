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

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices);
    ~Mesh();
    const std::vector<Vertex>& GetVertices() const;
    const std::vector<unsigned short>& GetIndices() const;

private:
    std::vector<Vertex> m_Vertices;
    std::vector<unsigned short> m_Indices;
};