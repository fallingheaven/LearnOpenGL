#version 440 core
out vec4 FragColor;

in VS_OUT
{
    vec3 Normal;
    vec3 WorldPos;
    vec2 TexCoords;
    mat3 TBN;
    flat int InstanceID;
} fs_in;

struct Material
{
    sampler2D texture_albedo1;
    sampler2D texture_normal1;
    sampler2D texture_metallic1;
    sampler2D texture_roughness1;
    sampler2D texture_ao1;
};
uniform Material material;

struct PointLight
{
    vec3 color;
    vec3 position;

    float quadratic;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform vec3 viewPos;
uniform vec2 metallicRoughnessScale[50];

vec3 fresnelSchlick(float cosTheta, vec3 F0, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

const float PI = 3.14159265359;

uniform samplerCube irradianceMap;
uniform samplerCube specularMap;
uniform sampler2D   brdfLUT;
uniform bool useIBL;
uniform bool useSpecularIBL;

void main()
{
//    vec3 N = normalize(fs_in.Normal);
    vec3 N = texture(material.texture_normal1, fs_in.TexCoords).rgb;
    N = normalize(fs_in.TBN * (N * 2.0 - 1.0));
//    N = normalize(fs_in.TBN * N);

    vec3 V = normalize(viewPos - fs_in.WorldPos);
    vec3 R = reflect(-V, N);

    vec3 albedo = pow(texture(material.texture_albedo1, fs_in.TexCoords).rgb, vec3(2.2));
    vec2 metallicRoughness = metallicRoughnessScale[fs_in.InstanceID];
    float metallic = texture(material.texture_metallic1, fs_in.TexCoords).b * metallicRoughness.x;
    float roughness = texture(material.texture_metallic1, fs_in.TexCoords).g * metallicRoughness.y;
//    float ao = texture(material.texture_ao1, fs_in.TexCoords).r;
//    vec3 albedo = vec3(0.5, 0.0, 0.0);
//    float metallic = metallicRoughness.x;
//    float roughness = metallicRoughness.y;
    float ao = 1;


    vec3 Lo = vec3(0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    for (int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        vec3 L = normalize(pointLights[i].position - fs_in.WorldPos);
        vec3 H = normalize(V + L);
        // 计算衰减后的入射辐射度
        float distance = length(pointLights[i].position - fs_in.WorldPos);
        float attenuation = 1.0 / (pointLights[i].quadratic * (distance * distance));
        vec3 radiance = pointLights[i].color * attenuation;

        // 菲涅尔系数
        vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0, roughness);

        // 法线分布函数和几何函数
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);

        // 计算镜面反射项
        vec3 nominator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular     = nominator / denominator;

        // 通过菲涅尔计算漫反射和镜面反射的比例
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        // 根据金属度降低漫反射比例
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    // 环境光+AO
    vec3 ambient = vec3(0.0);
    vec3 F = fresnelSchlick(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    if (useIBL)
    {
        vec3 kD = (1.0 - kS) * (1 - metallic);
        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse    = irradiance * albedo;
        ambient    = kD * diffuse;
    }
    else
    {
        ambient = vec3(0.03) * albedo;
    }

    vec3 specular = vec3(0.0);
    if (useSpecularIBL)
    {
        // IBL specular
        //    vec3 specular = texture(specularMap, R, roughness * 4).rgb * kS;
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(specularMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
        vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    }


    vec3 color   = (ambient + Lo + specular)*ao;


    FragColor = vec4(color, 1.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
//    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}