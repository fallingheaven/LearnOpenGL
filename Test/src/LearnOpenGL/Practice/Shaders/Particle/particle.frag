#version 440 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;
uniform vec4 color;

void main()
{
    vec4 texColor = texture(image, TexCoords);
    FragColor = color * texColor;
}