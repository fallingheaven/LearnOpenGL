#version 440 core
in  vec2  TexCoords;
out vec4  FragColor;

uniform sampler2D scene;
uniform vec2      offsets[9];
uniform int       edge_kernel[9];
uniform float     blur_kernel[9];

uniform bool chaos;
uniform bool confuse;
uniform bool shake;


void main()
{
    FragColor = vec4(0.0f);

    if(chaos)
    {
        for(int i = 0; i < 9; i++)
        {
            FragColor += vec4(texture(scene, TexCoords + offsets[i]).rgb * edge_kernel[i], 0);
        }
        FragColor.a = 1.0f;
    }
    else if (shake)
    {
        for(int i = 0; i < 9; i++)
        {
            FragColor += vec4(texture(scene, TexCoords + offsets[i]).rgb * blur_kernel[i], 0);
        }
//        FragColor = vec4(texture(scene, TexCoords + offsets[5]).rgb * blur_kernel[5], 0);
        FragColor.a = 1.0f;
        return;
    }
    else if (confuse)
    {
        FragColor = vec4(1.0 - texture(scene, TexCoords).rgb, 1.0);
    }
    else
    {
        FragColor =  texture(scene, TexCoords);
    }
}