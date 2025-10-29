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

out VS_OUT {
    vec3 Normal;
    vec3 WorldPos;
    vec2 TexCoords;
//    vec3 TangentLightPos;
//    vec3 TangentViewPos;
//    vec3 TangentWorldPos;
} vs_out;

out mat3 TBN;

void main()
{
    mat4 model = instanceModel; // 使用实例模型矩阵
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.WorldPos = (model * vec4(aPos, 1.0f)).xyz;
    vs_out.TexCoords = aTexCoords;

    vec3 T = normalize(mat3(model) * aTangent);
    vec3 N = normalize(vs_out.Normal);
    T = normalize(T - dot(T, N) * N); // 重正交化，保证顶点的法线和切线正交
    vec3 B = normalize(cross(N, T));
    TBN = transpose(mat3(T, B, N)); // 正交矩阵，转置即逆矩阵
    // 世界空间转换到切线空间

//    vs_out.TangentWorldPos = TBN * vs_out.WorldPos; // 片元位置
}