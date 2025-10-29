#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in mat4 instanceModel; // 实例模型矩阵

// 不需要传递这些值，不需要接口块
//out VS_OUT {
//    vec3 normal;
//    vec3 worldPos;
//    vec2 texCoords;
//} vs_out;

void main()
{
    gl_Position = instanceModel * vec4(aPos, 1.0);
//    vs_out.worldPos = (instanceModel * vec4(aPos, 1.0)).xyz;
//    vs_out.normal = vec3(transpose(inverse(instanceModel)) * vec4(aNormal, 1.0));
//    vs_out.texCoords = aTexCoords;
}