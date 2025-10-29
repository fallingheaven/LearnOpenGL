#version 440 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
//    FragColor = vec4(1.0 - gl_FragCoord.rgb, 1.0);
    FragColor = vec4(1-texture(screenTexture, TexCoords).rgb, 1.0);
}