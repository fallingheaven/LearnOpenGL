#version 440 core
layout (location = 0) in vec3 aPos;

uniform mat4 cameraProjection;
uniform mat4 cameraView;

out vec3 sampleDir;

void main()
{
    gl_Position = cameraProjection * cameraView * vec4(aPos, 1.0);
    sampleDir = aPos;
}