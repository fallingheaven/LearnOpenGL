#version 440 core
out vec4 FragColor;

in vec3 inputColor;
in vec2 TexCoord;

// uniform vec4 inputColor;

// uniform sampler2D ourTexture;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    // FragColor = vec4(inputColor.xyz, 1.0f);
    // FragColor = texture(ourTexture, TexCoord);
    // FragColor = texture(ourTexture, TexCoord) * vec4(inputColor, 1.0);
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
}