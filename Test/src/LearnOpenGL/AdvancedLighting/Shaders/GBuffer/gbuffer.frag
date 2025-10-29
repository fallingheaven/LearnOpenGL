#version 440 core
layout(location = 0) out vec4 worldPos;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec4 albedoSpec;

struct Material {
    sampler2D texture_diffuse1; // 0
    //    sampler2D texture_specular1; // 1
    sampler2D texture_displacement1; // 1
    sampler2D texture_normal1; // 2
    sampler2D texture_reflection1; // 3

    float shininess;

    bool useBlinnPhong;
};
uniform Material material;

in VS_OUT {
    vec3 Normal;
    vec3 WorldPos;
    vec2 TexCoords;
//    vec3 TangentLightPos;
//    vec3 TangentViewPos;
//    vec3 TangentWorldPos;
} fs_in;

const float near_plane = 0.1; // 投影矩阵的近平面
const float far_plane = 50.0f; // 投影矩阵的远平面

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // 回到NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    normal = vec4(normalize(fs_in.Normal), 1);
//    normal = normalize(fs_in.Normal) * 0.5 + 0.5;
    worldPos = vec4(fs_in.WorldPos, LinearizeDepth(gl_FragCoord.z));
    albedoSpec.rgb = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    albedoSpec.a = material.shininess;
}