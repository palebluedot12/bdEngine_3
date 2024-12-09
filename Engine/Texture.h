#pragma once

#include <GL\glew.h>

#include "stb_image.h"

class Texture
{
public:
	Texture(const char* fileLoc);

	/// <summary>
	/// �ؽ��� �޸𸮷� �ε�, GPU�� ���ֱ�
	/// </summary>
	bool LoadTexture();

	/// <summary>
	/// 0�� �ؽ��� ���ֿ� �ؽ��� �����ֱ�
	/// </summary>
	void UseTexture();

	void ClearTexture(); // �޸𸮿��� �ؽ��� ������

	~Texture();

private:
	GLuint textureID;
	int width, height, bitDepth;

	const char* fileLocation;
};