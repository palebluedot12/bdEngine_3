#include "pch.h"
#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices)
    : m_Vertices(vertices), m_Indices(indices) {
}

Mesh::~Mesh() {}

const std::vector<Vertex>& Mesh::GetVertices() const
{
    return m_Vertices;
}

const std::vector<unsigned short>& Mesh::GetIndices() const
{
    return m_Indices;
}