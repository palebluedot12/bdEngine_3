#include "Texture.h"

#include "stb_image.h"
#include <stdexcept>
#include <iostream>

Texture::Texture(const char* fileLoc)
{
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
	fileLocation = fileLoc;
}

bool Texture::LoadTexture()
{
	unsigned char* texData = stbi_load(fileLocation, &width, &height, &bitDepth, 0);
	if (!texData)
	{
		printf("Failed to find: %s\n", fileLocation);
		return false;
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	// 1ä�� �ؽ�ó�� ��� 3ä�η� �ٽ� �ε�
	if (bitDepth == 1)
	{
		stbi_image_free(texData);  // ���� ������ ����
		texData = stbi_load(fileLocation, &width, &height, &bitDepth, STBI_rgb);
		if (!texData)
		{
			std::cerr << "Failed to reload texture as 3-channel: " << fileLocation << std::endl;
			return false;
		}
		bitDepth = 3;  // ä�� ���� 3�� ����
	}

	// �ؽ����� ä�� ���� ���� �´� �������� ���ֱ�
	GLenum const textureFormat[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
	glTexImage2D(
		GL_TEXTURE_2D, 0, textureFormat[bitDepth - 1],
		width, height, 0,
		textureFormat[bitDepth - 1], GL_UNSIGNED_BYTE, texData);


	//glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(texData);
	return true;
}

void Texture::UseTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::ClearTexture()
{
	glDeleteTextures(1, &textureID);
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
	fileLocation = "";
}


Texture::~Texture()
{
	ClearTexture();
}