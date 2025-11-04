#version 440 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
uniform vec2 offset;
uniform mat4 orthoProjection;

void main()
{
    float scale = 10;
    TexCoords = aTexCoords;
    gl_Position = orthoProjection * vec4(aPos*scale + offset, 0, 1);
}