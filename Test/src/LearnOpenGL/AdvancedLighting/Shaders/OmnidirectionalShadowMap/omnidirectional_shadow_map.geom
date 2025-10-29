#version 440 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

//in VS_OUT {
//    vec3 normal;
//    vec3 worldPos;
//    vec2 texCoords;
//} vs_in;
out vec4 worldPos;

uniform mat4 shadowMatrices[6];

void main()
{
    for (int i = 0; i < 6; i++)
    {
        gl_Layer = i;
        for (int j = 0; j < 3; j++)
        {
            worldPos = gl_in[j].gl_Position;
            gl_Position = shadowMatrices[i] * worldPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}