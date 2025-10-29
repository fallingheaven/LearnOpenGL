#version 440 core
out vec4 FragColor;

in vec3 Normal;
in vec3 worldPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

struct Material {
    //    sampler2D diffuse;
    //    sampler2D specular;
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
//    sampler2D texture_height1;
    sampler2D texture_reflection1;

    float shininess;

    bool useBlinnPhong;
};
uniform Material material;

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

uniform vec3 viewPos;

uniform samplerCube skybox; // 4

uniform sampler2D shadowMap; // 5

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
//    float bias = 0.005;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.005);
//    float bias = -max(0.1 * (1.0 - dot(normal, lightDir)), 0.05); // 正面剔除使用
//    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
//    float depthDifference = currentDepth - bias - closestDepth;
//    float shadow = depthDifference > 0? exp(-depthDifference / 5) : 0.0;

    float shadow = 0.0;
    // PCF百分比渐进滤波，也就是模糊操作
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
//            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            float depthDifference = currentDepth - bias - pcfDepth;
            shadow += depthDifference > 0? exp(-depthDifference / 5) : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0 || projCoords.z < 0.0) return 0.0; // 超出光源视锥外

    return shadow;
}

void main()
{
    // 属性
    vec3 norm = normalize(Normal);
//    vec3 norm = normalize(texture(material.texture_normal1, TexCoords).rgb);
    vec3 viewDir = normalize(viewPos - worldPos);
    vec3 result = vec3(0, 0, 0);

    // 第一阶段：定向光照
    result = CalcDirLight(dirLight, norm, viewDir);
//    // 第二阶段：点光源
//    result += CalcPointLight(pointLights[0], norm, worldPos, viewDir);
//    // 第三阶段：聚光
//    result += CalcSpotLight(spotLight, norm, worldPos, viewDir);

    // 天空盒反射
    vec3 reflection = vec3(texture(material.texture_reflection1, TexCoords));
    vec3 I = normalize(worldPos - viewPos);
    vec3 R = reflect(I, norm);
    vec3 reflectionColor = texture(skybox, R).rgb;
    result = mix(result, reflectionColor, reflection.r * 0.8f);

    float shadow = ShadowCalculation(FragPosLightSpace, norm, normalize(-dirLight.direction));
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