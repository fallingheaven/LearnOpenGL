#version 440 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 Normal;
    vec3 WorldPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentWorldPos;
} fs_in;

in mat3 TBN;

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

uniform float farPlane;

uniform samplerCube skybox; // 4

uniform samplerCube shadowMap; // 5

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 lightDir, vec3 viewDir, float lightDis, vec2 texCoords);

float GetRandom(vec3 seed, int i)
{
    vec4 seed4 = vec4(seed, i);
    float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
    return sin(fract(dot_product) * 43758.5453);
}

float ShadowCalculation(vec3 worldPos, vec3 normal, vec3 lightPos)
{
    vec3 lightToFrag = worldPos - lightPos;
    //    float closestDepth = texture(shadowMap, normalize(lightToFrag)).r * farPlane;
    float currentDepth = length(lightToFrag);
    vec3 lightDir = normalize(lightToFrag);
//    float bias = 0.05;
    float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.005); // 夹角越大，bias越大

    float shadow = 0.0;
    float offset = 0.01;
    for (int i = 0; i < 20; ++i)
    {
        vec3 randomOffset = vec3(GetRandom(worldPos, i), GetRandom(worldPos, i + 1), GetRandom(worldPos, i + 2)) * offset;
        vec3 sampleDir = normalize(lightToFrag + randomOffset);
        float closestDepth = texture(shadowMap, sampleDir).r * farPlane;
        float depthDifference = currentDepth - bias - closestDepth;
        //        shadow += depthDifference > 0? exp(-depthDifference / 5) : 0.0;
        shadow += depthDifference > 0 ? 1.0 : 0.0;
    }
    shadow /= 20.0;

    return shadow;
}

//vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
//{
//    float height = texture(material.texture_displacement1, texCoords).r;
//    if (length(height) < 1e-5) return texCoords;
//    float heightScale = 0.1; // 控制height到位移的映射比例
//    vec2 offset = viewDir.xy / viewDir.z * height * heightScale;
//    return texCoords - offset;
//}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height = texture(material.texture_displacement1, texCoords).r;
    if (length(height) < 1e-5) return texCoords;

//    const float layerNum = 10.0;
    float layerNum = mix(10.0, 32.0, abs(dot(vec3(0.0, 0.0, 1.0), normalize(viewDir))));
    float layerDepth = 1 / layerNum;
//    float layerDepth = (1-height) / layerNum;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy / viewDir.z * 0.1;
    vec2 deltaTexCoords = P * layerDepth;
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(material.texture_displacement1, currentTexCoords).r;

    float l = 0, r = height;
    for (int i = 0; i < 10; i++)
    {
        currentLayerDepth = (l + r) / 2.0;
        currentTexCoords = texCoords - P * currentLayerDepth;
        currentDepthMapValue = texture(material.texture_displacement1, currentTexCoords).r;
        if (currentDepthMapValue > currentLayerDepth)
            l = currentLayerDepth;
        else
            r = currentLayerDepth;
    }
//    while (currentLayerDepth < currentDepthMapValue)
//    {
//        currentTexCoords -= deltaTexCoords;
//        currentDepthMapValue = texture(material.texture_displacement1, currentTexCoords).r;
//        currentLayerDepth += layerDepth;
//    }

//    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
//    float afterDepth  = currentDepthMapValue - currentLayerDepth;
//    float beforeDepth = texture(material.texture_displacement1, prevTexCoords).r - (currentLayerDepth - layerDepth);
//    float weight = afterDepth / (afterDepth - beforeDepth);
//    currentTexCoords = mix(currentTexCoords, prevTexCoords, weight);

    return currentTexCoords;
}

void main()
{
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentWorldPos);
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentWorldPos);
    float lightDis = length(fs_in.WorldPos - pointLights[0].position);

    // 属性
    vec2 displacedTexCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
    //    vec2 displacedTexCoords = fs_in.TexCoords;
    if(displacedTexCoords.x > 1.0 || displacedTexCoords.y > 1.0 || displacedTexCoords.x < 0.0 || displacedTexCoords.y < 0.0) discard;

//    FragColor = texture(material.texture_displacement1, fs_in.TexCoords);
//    return;

//    vec3 norm = texture(material.texture_normal1, fs_in.TexCoords).rgb;
    vec3 norm = texture(material.texture_normal1, displacedTexCoords).rgb; // 切线空间法线
    norm = length(norm) < 1e-5 ? normalize(TBN*fs_in.Normal) : normalize(norm * 2.0 - 1.0);

    vec3 result = vec3(0, 0, 0);

    result += CalcPointLight(pointLights[0], norm, lightDir, viewDir, lightDis, displacedTexCoords);

    // 天空盒反射
    //    vec3 reflection = vec3(texture(material.texture_reflection1, TexCoords));
    //    vec3 I = normalize(worldPos - viewPos);
    //    vec3 R = reflect(I, norm);
    //    vec3 reflectionColor = texture(skybox, R).rgb;
    //    result = mix(result, reflectionColor, reflection.r * 0.8f);

    float shadow = ShadowCalculation(fs_in.WorldPos, transpose(TBN)*norm, pointLights[0].position);
    result  = (1-shadow) * result;

    FragColor = vec4(result, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 lightDir, vec3 viewDir, float lightDis, vec2 texCoords)
{
    vec3 lightDirNorm = normalize(lightDir);
    // 漫反射着色
    float diff = max(dot(normal, lightDirNorm), 0.0);
    // 镜面光着色
    float spec = 0;
    if (!material.useBlinnPhong)
    {
        vec3 reflectDir = reflect(-lightDirNorm, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    else
    {
        vec3 halfwayDir = normalize(lightDirNorm + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }
    // 衰减
    float distance    = lightDis;
    float attenuation = 1.0 / (light.constant + light.linear * distance +
    light.quadratic * (distance * distance));
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, texCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, texCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_diffuse1, texCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}