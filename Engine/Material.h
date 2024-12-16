#pragma once
#include <string>

class Material {
public:
    Material();
    ~Material();

    // 텍스쳐 파일 경로
    std::string diffuseTexturePath;
    std::string normalTexturePath;
    std::string specularTexturePath;
};