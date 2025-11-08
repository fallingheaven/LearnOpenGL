#version 440 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 weights;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

out vec2 TexCoords;

void main()
{
    gl_Position =  projection * view * model * vec4(aPos, 1.0f);
    TexCoords = aTexCoord;
}