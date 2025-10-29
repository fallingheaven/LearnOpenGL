#version 440 core
in vec4 worldPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    float distance = length(worldPos.xyz - lightPos);

    distance /= far_plane;

    gl_FragDepth = distance;
}