#version 440 core
out vec4 FragColor;

in vec3 sampleDir;

uniform samplerCube cubeMap;

const float PI = 3.14159265359;

void main()
{
    vec3 N = normalize(sampleDir);
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 B = normalize(cross(N, up));
    vec3 T = normalize(cross(B, N));
    mat3 TBN = mat3(T, B, N);

    vec3 irradiance = vec3(0.0);

    float sampleDelta = 0.025;
    float sampleCount = 0;
    for (float phi = 0; phi < 2 * PI; phi += sampleDelta)
    {
        for (float theta = 0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 offset = vec3(sin(theta)*vec2(cos(phi), sin(phi)), cos(theta));
            vec3 sampleVec = TBN * offset;

//            irradiance += texture(cubeMap, sampleVec).rgb * cos(theta) * sin(theta);
            irradiance += textureLod(cubeMap, sampleVec, 0).rgb * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    irradiance = irradiance * PI / sampleCount;

    FragColor = vec4(irradiance, 1.0);
}