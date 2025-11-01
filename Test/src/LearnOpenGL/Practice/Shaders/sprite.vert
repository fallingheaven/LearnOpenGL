#version 440 core
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 orthoProjection;

//layout (std140, binding = 0) uniform Matrices
//{
//    mat4 projection;
//    mat4 view;
//};

void main()
{
    TexCoords = aTexCoords;
    gl_Position = orthoProjection * model * vec4(aPosition, 0.0, 1.0);
}
