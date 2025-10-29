#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceModel; // 实例模型矩阵

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 Normal;
out vec3 worldPos;
out vec2 TexCoords;
// 需要几何着色器时启用
out VS_OUT {
    vec3 Normal;
    vec3 worldPos;
    vec2 TexCoords;
} vs_out;

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;

void main()
{
    mat4 model = instanceModel; // 使用实例模型矩阵
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    // 缩放会破坏法向量，使其不与平面垂直，需要进行修复
    // 矩阵求逆是一项对于着色器开销很大的运算，因为它必须在场景中的每一个顶点上进行，所以应该尽可能地避免在着色器中进行求逆运算（在CPU计算后传入）
    Normal = mat3(transpose(inverse(model))) * aNormal;
//    Normal = aNormal;
    worldPos = (model * vec4(aPos, 1.0f)).xyz;
    TexCoords = aTexCoords;

    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.worldPos = (model * vec4(aPos, 1.0f)).xyz;
    vs_out.TexCoords = aTexCoords;
}