#version 440 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 Normal;
    vec3 worldPos;
    vec2 TexCoords;
} vs_out[];

out vec3 Normal;
out vec3 worldPos;
out vec2 TexCoords;

uniform float time;

vec3 getNormal()
{
    vec3 pos1 = gl_in[0].gl_Position.xyz;
    vec3 pos2 = gl_in[1].gl_Position.xyz;
    vec3 pos3 = gl_in[2].gl_Position.xyz;
    vec3 normal = normalize(cross(pos2 - pos1, pos3 - pos1));
    return normal;
}

void main()
{
    vec3 explodeDir = getNormal();
    float amplitude = 0.05 * sin(time);

    for (int i = 0; i < 3; i++)
    {
        gl_Position = gl_in[i].gl_Position + vec4(explodeDir * amplitude, 0.0);

        Normal = vs_out[i].Normal;
        worldPos = vs_out[i].worldPos;
        TexCoords = vs_out[i].TexCoords;

        EmitVertex();
    }
    EndPrimitive();
}