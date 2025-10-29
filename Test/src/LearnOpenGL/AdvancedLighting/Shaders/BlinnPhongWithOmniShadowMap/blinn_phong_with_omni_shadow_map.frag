#version 440 core
out vec4 FragColor;

in vec3 Normal;
in vec3 worldPos;
in vec2 TexCoords;
in mat3 TBN;

struct Material {
    sampler2D texture_diffuse1; // 0
//    sampler2D texture_specular1; // 1
    sampler2D texture_displacement; // 1
    sampler2D texture_normal1; // 2
    sampler2D texture_reflection1; // 3

    float shininess;

    bool useBlinnPhong;
};
uniform Material material;

uniform vec3 viewPos;

uniform float farPlane;

uniform samplerCube skybox; // 4

uniform samplerCube shadowMap; // 5

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

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
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 worldPos, vec3 viewDir);

struct SpotLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3  direction;
    float cutOff;
    float outerCutOff;
};
uniform SpotLight spotLight;
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 worldPos, vec3 viewDir);

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
//    float bias = 0.1;
    vec3 lightDir = normalize(lightToFrag);
    float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.005); // 夹角越大，bias越大
//    float depthDifference = currentDepth - bias - closestDepth;
//    float shadow = depthDifference > 0 ? 1.0 : 0.0;
//    float shadow = depthDifference > 0? exp(-depthDifference / 5) : 0.0;

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

void main()
{
    // 属性
    vec3 norm = vec3(0);
//    vec3 norm = normalize(Normal);
    norm = texture(material.texture_normal1, TexCoords).rgb;
    norm = length(norm) < 1e-5 ? normalize(Normal) : normalize(norm * 2.0 - 1.0);
    norm = normalize(TBN * norm);
//    vec3 norm = normalize(texture(material.texture_normal1, TexCoords).rgb);
    vec3 viewDir = normalize(viewPos - worldPos);
    vec3 result = vec3(0, 0, 0);

//    // 第一阶段：定向光照
//    result = CalcDirLight(dirLight, norm, viewDir);
    // 第二阶段：点光源
    result += CalcPointLight(pointLights[0], norm, worldPos, viewDir);
//    // 第三阶段：聚光
//    result += CalcSpotLight(spotLight, norm, worldPos, viewDir);

    // 天空盒反射
//    vec3 reflection = vec3(texture(material.texture_reflection1, TexCoords));
//    vec3 I = normalize(worldPos - viewPos);
//    vec3 R = reflect(I, norm);
//    vec3 reflectionColor = texture(skybox, R).rgb;
//    result = mix(result, reflectionColor, reflection.r * 0.8f);

    float shadow = ShadowCalculation(worldPos, norm, pointLights[0].position);
    result  = (1-shadow) * result;

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_diffuse1, TexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 worldPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - worldPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    float spec = 0;
    if (!material.useBlinnPhong)
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    else
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }
    // 衰减
    float distance    = length(light.position - worldPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
    light.quadratic * (distance * distance));
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_diffuse1, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 worldPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - worldPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    float spec = 0;
    if (!material.useBlinnPhong)
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    else
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }

    // 边缘衰减
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    intensity = clamp(exp(-length(worldPos - light.position)/5), 0.0, intensity);
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_diffuse1, TexCoords));
    ambient  *= intensity;
    diffuse  *= intensity;
    specular *= intensity;
    return (ambient + diffuse + specular);
}