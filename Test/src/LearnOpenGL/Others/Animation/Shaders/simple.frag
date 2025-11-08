#version 440 core
out vec4 FragColor;

in vec2 TexCoords;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;

    float shininess;
};

uniform Material material;

void main()
{
    vec4 result = texture(material.texture_diffuse1, TexCoords);
    FragColor = result;
}