#version 440 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec3 gray = vec3(0.299, 0.587, 0.114);
    FragColor = vec4(vec3(dot(gray, texture(screenTexture, TexCoords).rgb)), 1.0);
}