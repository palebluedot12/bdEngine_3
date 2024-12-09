#pragma once

#include <GL\glew.h>

#include "stb_image.h"

class Texture
{
public:
	Texture(const char* fileLoc);

	/// <summary>
	/// 텍스쳐 메모리로 로드, GPU로 쏴주기
	/// </summary>
	bool LoadTexture();

	/// <summary>
	/// 0번 텍스쳐 유닛에 텍스쳐 물려주기
	/// </summary>
	void UseTexture();

	void ClearTexture(); // 메모리에서 텍스쳐 내리기

	~Texture();

private:
	GLuint textureID;
	int width, height, bitDepth;

	const char* fileLocation;
};