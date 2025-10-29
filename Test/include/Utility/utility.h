#pragma once
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <stb_image.h>
#include <random>
#include <texture.h>


namespace Utility
{
    constexpr float PI = 3.14159265359;
    float angle2Radian(float angle);
    float radian2Angle(float radian);

    unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
    unsigned int TextureFromMemory(const aiTexture* texture, bool gamma);

    std::string getFileExtension(const std::string& filePath);

    glm::vec3 GetRandomVec3(float min, float max);
    unsigned int GenModelMatBuffer(int amount, glm::vec3 scale = glm::vec3(1.0f), glm::vec2 range = glm::vec2(-5.0f, 5.0f)); // 批量生成模型矩阵
    unsigned int GenModelMatBuffer(glm::vec3 position, glm::vec3 scale); // 生成单个模型矩阵
    unsigned int GenModelMatBuffer(std::vector<glm::vec3> &position, glm::vec3 scale); // 生成指定的模型矩阵

    std::vector<glm::vec3> genSSAOKernel(int kernelSize); // 生成SSAO采样核
    unsigned int genSSAONoiseTexture(int texSize);// 生成单位半球随机向量纹理
}
