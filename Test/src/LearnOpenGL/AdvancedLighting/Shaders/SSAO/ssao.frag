#version 440 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D ssaoNoiseTex;

uniform vec3 sampleKernel[64];
uniform mat4 projection;
uniform mat4 view;

const vec2 noiseScale = vec2(800.0/4.0, 800.0/4.0);
const int kernelSize = 64;
const float radius = 0.5;

void main()
{
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    float depth = texture(gPosition, TexCoords).a;

    vec3 randomVec = vec3(normalize(texture(ssaoNoiseTex, TexCoords * noiseScale).xy), 0);

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = normalize(cross(normal, tangent));
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float result = 0;
    for (int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = worldPos + TBN * sampleKernel[i] * radius;

        vec4 offset = projection * view * vec4(samplePos, 1.0);
        offset.xyz /= offset.w; // 到 NDC
        offset.xyz = offset.xyz * 0.5 + 0.5; // NDC 到 纹理坐标

        float sampleDepth = texture(gPosition, offset.xy).a;

//        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(depth - sampleDepth));
//        result += (sampleDepth < depth - 0.05 ? 1.0 : 0.0) * rangeCheck;
        result += (sampleDepth < depth - 0.05 ? 1.0 : 0.0);
    }

    result = 1.0 - (result / kernelSize);

    FragColor = vec4(vec3(result), 1.0);
}