#version 440 core
layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

void main()
{
    for (int i = 0; i < 3; i++)
    {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();

        gl_Position = gl_in[i].gl_Position + vec4(gs_in[i].normal * 0.2, 0.0);
        EmitVertex();

        EndPrimitive();
    }
}