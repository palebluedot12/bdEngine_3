#include "pch.h"
#include <iostream>
#include "Model.h"
//#include <GL/glew.h>

void Model::LoadModel(const std::string& fileName)
{
	// �� �̸� ����
	int firstSlashIdx = fileName.find('/', 0);
	modelName = fileName.substr(0, firstSlashIdx);

	Assimp::Importer importer;

	unsigned int importFlags = aiProcess_Triangulate |    // vertex �ﰢ�� ���� ���. quad ���� ������ triangle�� �ɰ�
		aiProcess_GenNormals |							  // Normal ���� ����  
		aiProcess_GenUVCoords |							  // �ؽ�ó ��ǥ ����
		aiProcess_CalcTangentSpace |					  // ź��Ʈ ���� ����
		aiProcess_ConvertToLeftHanded;					  // DX�� �޼���ǥ�� ��ȯ
	//aiProcess_PreTransformVertices					  // ����� ��ȯ����� ������ ���ؽ� �����Ѵ�.  *StaticMesh�� ó���Ҷ���

	const aiScene* scene = importer.ReadFile("Resources/" + fileName,
		importFlags);

	if (!scene)
	{
		std::cout << fileName << " Model �ε� ���� : " << importer.GetErrorString() << std::endl;
		return;
	}

	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
}

void Model::RenderModel()
{
	// LoadMesh �Լ����� ä������ meshList�� ��ȸ�ϸ� �޽õ��� �������Ѵ�.
	for (size_t i = 0; i < meshList.size(); i++)
	{
		// �޽ÿ� �ش��ϴ� ��Ƽ������ ���� �ؽ��ĸ� ������ ����Ѵ�.
		unsigned int materialIndex = meshToTex[i];
		if (materialIndex < textureList.size() && textureList[materialIndex])
			textureList[materialIndex]->UseTexture();

		meshList[i]->RenderMesh();
	}
}

void Model::ClearModel()
{
	for (size_t i = 0; i < meshList.size(); i++)
	{
		if (meshList[i])
		{
			delete meshList[i];
			meshList[i] = nullptr;
		}
	}

	for (size_t i = 0; i < textureList.size(); i++)
	{
		if (textureList[i])
		{
			delete textureList[i];
			textureList[i] = nullptr;
		}
	}
}

void Model::LoadNode(aiNode* node, const aiScene* scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		// node->mMeshes[i] : �޽� ��ü�� �ƴϰ�, �޽��� ID�� �ǹ��Ѵ�.
		// ���� �޽ô� scene�� ����Ǿ��ֱ� ������ �̷��� �����ϰ� �ȴ�.
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	// �ڽ� ������ ���ȣ���� ���� ��ȸ�ϸ� �޽ø� �� �ε��Ѵ�.
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

// ������ VBO, IBO�� ���� �������� �����Ѵ�.
void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		// position
		vertices.push_back(mesh->mVertices[i].x);
		vertices.push_back(mesh->mVertices[i].y);
		vertices.push_back(mesh->mVertices[i].z);

		// texture
		if (mesh->mTextureCoords[0])
		{
			vertices.push_back(mesh->mTextureCoords[0][i].x);
			vertices.push_back(mesh->mTextureCoords[0][i].y);
		}
		else // �������� ���� ��� �׳� 0�� �־��ֱ�
		{
			vertices.push_back(0.f);
			vertices.push_back(0.f);
		}

		// normal (aiProcess_GenSmoothNormals�� �����߱� ������ ���� ���� ����.)
		vertices.push_back(mesh->mNormals[i].x);
		vertices.push_back(mesh->mNormals[i].y);
		vertices.push_back(mesh->mNormals[i].z);
	}

	// indices ä���ֱ�
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	Mesh* newMesh = new Mesh();
	newMesh->CreateMesh(vertices, indices); // GPU�� VBO, IBO�� ���ؽ� ������ ���ش�.
	meshList.push_back(newMesh);

	// meshList�� mesh�� ä���ܰ� ���ÿ�, meshToTex���� �� mesh�� materialIndex�� ä���ش�.
	// �̷��� meshList�� meshToTex�� ������ ä�������ν� mesh�� �´� material�� �ս��� ã�� �� �ִ�.
	meshToTex.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene* scene)
{
	textureList.resize(scene->mNumMaterials);
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		textureList[i] = nullptr;

		// �ؽ��İ� �����ϴ� �� ���� Ȯ��
		if (material->GetTextureCount(aiTextureType_BASE_COLOR))
		{
			aiString texturePath;
			// �ؽ��� ��θ� �������� �� �����ߴٸ�
			if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) == aiReturn_SUCCESS)
			{
				// Ȥ�ó� �ؽ��� ��ΰ� ���� ��η� �Ǿ��ִٸ� �׿� ���� ó��
				int idx = std::string(texturePath.data).rfind("/");
				std::string textureName = std::string(texturePath.data).substr(idx + 1);

				std::string texPath = "Models/" + modelName + "/textures/" + textureName;

				textureList[i] = new Texture(texPath.c_str());

				// �ؽ��ĸ� ��ũ���� �޸𸮷� �ε�, GPU�� ���ش�.
				if (!textureList[i]->LoadTexture())
				{ // ���� ��
					std::cout << "�ؽ��� �ε� ���� : " << texPath << std::endl;
					delete textureList[i];
					textureList[i] = nullptr;
				}
			}
		}

		// textureList�� �ؽ��ĸ� ��µ� �����ߴٸ�
		if (!textureList[i])
		{
			textureList[i] = new Texture("plain.png"); // ��� �ؽ��ķ� ��ü
		}
	}
}

Model::~Model()
{
	ClearModel();
}