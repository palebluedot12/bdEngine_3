#pragma once
#include <string>

class Material {
public:
    Material();
    ~Material();

    // �ؽ��� ���� ���
    std::string diffuseTexturePath;
    std::string normalTexturePath;
    std::string specularTexturePath;
};