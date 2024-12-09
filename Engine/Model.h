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
	void LoadModel(const std::string& fileName); // 모델을 메모리에 올린다.
	void RenderModel(); // 렌더링한다.
	void ClearModel(); // 메모리에서 내린다.

	~Model();

private:
	// 재귀호출되며 노드를 순회하기 위한 함수
	void LoadNode(aiNode* node, const aiScene* scene);
	// 실제 메시 오브젝트를 참조하여 버텍스 정보를 로드한다.
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	// 텍스쳐를 로드한다.
	void LoadMaterials(const aiScene* scene);

	std::vector<Mesh*> meshList;
	std::vector<Texture*> textureList;
	std::vector<unsigned int> meshToTex; // 메시에 맞는 머티리얼의 인덱스를 저장한다.

	std::string modelName;
};