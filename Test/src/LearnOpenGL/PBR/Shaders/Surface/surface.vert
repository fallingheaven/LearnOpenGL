#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in mat4 instanceModel; // 实例模型矩阵

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out VS_OUT
{
    vec3 Normal;
    vec3 WorldPos;
    vec2 TexCoords;
    mat3 TBN;
    flat int InstanceID;
} vs_out;

void main()
{
    mat4 model = instanceModel; // 使用实例模型矩阵
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.WorldPos = (model * vec4(aPos, 1.0f)).xyz;
    vs_out.TexCoords = aTexCoords;

    // 计算切线空间矩阵 TBN
    vec3 T = normalize(mat3(model) * aTangent);
    vec3 N = normalize(vs_out.Normal);
    T = normalize(T - dot(T, N) * N); // 正交化切线
    vec3 B = normalize(cross(N, T));
    vs_out.TBN = mat3(T, B, N); // 切线空间 -> 世界空间

    vs_out.InstanceID = gl_InstanceID; // 传递实例ID
}