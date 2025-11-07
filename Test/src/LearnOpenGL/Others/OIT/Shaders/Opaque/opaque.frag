#version 440 core
out vec4 FragColor;

in VS_OUT
{
    vec2 TexCoord;
} fs_in;

uniform vec4 color;

void main()
{
    FragColor = color;
}