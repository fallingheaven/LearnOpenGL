#version 440 core
out vec4 FragColor;

in vec2 TexCoord;

layout (binding = 0) uniform sampler2D accumTex;
layout (binding = 1) uniform sampler2D revealTex;

const float EPSILON = 0.00001f;

bool isApproximatelyEqual(float a, float b)
{
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

void main()
{
    float revealage = texture(revealTex, TexCoord).r;

    if (isApproximatelyEqual(revealage, 1.0f)) discard;

    vec4 accum = texture(accumTex, TexCoord);

    if (isinf(max3(abs(accum.rgb))))
            accum.rgb = vec3(accum.a);

    // accum.rgb是加权和，accum.a是权重和
    vec3 average_color = accum.rgb / max(accum.a, EPSILON);

    FragColor = vec4(average_color, 1.0f - revealage);
}