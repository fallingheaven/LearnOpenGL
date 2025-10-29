#version 440 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
//    FragColor = vec4(vec3(texture(skybox, TexCoords).r), 1.0);
    FragColor = texture(skybox, TexCoords);
//    FragColor = textureLod(skybox, TexCoords, 1.2);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}