#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec3 worldPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    // 缩放会破坏法向量，使其不与平面垂直，需要进行修复
    // 矩阵求逆是一项对于着色器开销很大的运算，因为它必须在场景中的每一个顶点上进行，所以应该尽可能地避免在着色器中进行求逆运算（在CPU计算后传入）
    Normal = mat3(transpose(inverse(model))) * aNormal;
//    Normal = aNormal;
    worldPos = (model * vec4(aPos, 1.0f)).xyz;
    TexCoords = aTexCoords;
}