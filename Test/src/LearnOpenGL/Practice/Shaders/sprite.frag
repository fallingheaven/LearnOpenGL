#version 440 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;
uniform vec3 spriteColor;

void main()
{
    vec4 texColor = texture(image, TexCoords);

    FragColor = vec4(texColor.rgb * spriteColor, texColor.a);
}