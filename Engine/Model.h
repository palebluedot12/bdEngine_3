#pragma once

#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Texture.h"

class Model
{
public:
	void LoadModel(const std::string& fileName); // ���� �޸𸮿� �ø���.
	void RenderModel(); // �������Ѵ�.
	void ClearModel(); // �޸𸮿��� ������.

	~Model();

private:
	// ���ȣ��Ǹ� ��带 ��ȸ�ϱ� ���� �Լ�
	void LoadNode(aiNode* node, const aiScene* scene);
	// ���� �޽� ������Ʈ�� �����Ͽ� ���ؽ� ������ �ε��Ѵ�.
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	// �ؽ��ĸ� �ε��Ѵ�.
	void LoadMaterials(const aiScene* scene);

	std::vector<Mesh*> meshList;
	std::vector<Texture*> textureList;
	std::vector<unsigned int> meshToTex; // �޽ÿ� �´� ��Ƽ������ �ε����� �����Ѵ�.

	std::string modelName;
};