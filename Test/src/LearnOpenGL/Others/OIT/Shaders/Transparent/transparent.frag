#version 440 core
layout (location = 0) out vec4 accumulation;
layout (location = 1) out vec4 revealage;

in VS_OUT
{
    vec2 TexCoord;
} fs_in;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform vec4 color;

float calculateWeight(float alpha)
{
    return 1 - gl_FragCoord.z;
}

void main()
{
    accumulation = vec4(color.rgb * color.a, color.a) * calculateWeight(color.a);

    revealage = vec4(color.a);
}