#pragma once
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/detail/type_quat.hpp>
#include <iostream>
#include <stb_image.h>
#include <random>
#include <texture.h>
#include <Windows.h>
#include <cfloat>


namespace Utility
{
    constexpr float PI = 3.14159265359;
    float angle2Radian(float angle);
    float radian2Angle(float radian);

    glm::mat4 convertMatrixToGLMFormat(const aiMatrix4x4& aiMat);
    glm::vec3 getGLMVec(const aiVector3D& vec);
    glm::quat getGLMQuat(const aiQuaternion& pOrientation);

    // 根据投影矩阵和视图矩阵获取视锥体的8个角点的世界坐标
    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
    glm::mat4 createOrthoProjectionMatrixFromFrustumCorners(const std::vector<glm::vec4>& frustumCorners);

    unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
    unsigned int TextureFromMemory(const aiTexture* texture, bool gamma);

    std::string getFileExtension(const std::string& filePath);

    glm::vec3 GetRandomVec3(float min, float max);
    unsigned int GenModelMatBuffer(int amount, glm::vec3 scale = glm::vec3(1.0f), glm::vec2 range = glm::vec2(-5.0f, 5.0f)); // 批量生成模型矩阵
    unsigned int GenModelMatBuffer(glm::vec3 position, glm::vec3 scale); // 生成单个模型矩阵
    unsigned int GenModelMatBuffer(std::vector<glm::vec3> &position, glm::vec3 scale); // 生成指定的模型矩阵

    std::vector<glm::vec3> genSSAOKernel(int kernelSize); // 生成SSAO采样核
    unsigned int genSSAONoiseTexture(int texSize);// 生成单位半球随机向量纹理

    enum Direction {
        UP,
        RIGHT,
        DOWN,
        LEFT
    };
    struct Collision
    {
        bool isCollided;
        Direction collisionDir;
        glm::vec2 difference; // 碰撞时的差矢量，用于回退位置，防止重叠
    };
    Collision checkCollisionAABB(glm::vec3 pos1, glm::vec3 size1, glm::vec3 pos2, glm::vec3 size2); // 轴对齐包围盒碰撞检测
    Collision checkCollisionAABB(glm::vec3 pos1, float radius, glm::vec3 pos2, glm::vec3 size2); // 轴对齐包围盒与圆形碰撞检测
    Direction VectorDirection(glm::vec2 target);

    void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
                            GLsizei length, const char *message, const void *userParam); // OpenGL调试输出
}
