#version 440 core
layout (location = 0) in vec3 aPos;


uniform mat4 cameraProjection;
uniform mat4 cameraView;

out vec3 sampleDir;

void main()
{
    mat4 rotView = mat4(mat3(cameraView));
    vec4 clipPos = cameraProjection * rotView * vec4(aPos, 1.0);
    sampleDir = aPos;
    gl_Position = clipPos.xyww;
}