#include "pch.h"
#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex>* vertices, std::vector<unsigned short>* indices)
    : m_Vertices(vertices), m_Indices(indices)
{
}

Mesh::~Mesh() 
{
    delete m_Vertices;
    delete m_Indices;
}

const std::vector<Vertex>& Mesh::GetVertices() const
{
    return *m_Vertices;
}

const std::vector<unsigned short>& Mesh::GetIndices() const
{
    return *m_Indices;
}