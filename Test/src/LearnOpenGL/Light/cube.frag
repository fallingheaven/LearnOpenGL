#version 440 core
out vec4 FragColor;

in vec3 Normal;
in vec3 worldPos;
in vec2 TexCoords;

struct Material {
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;

    sampler2D diffuse;
    sampler2D specular;

    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // 点光源
    float constant;
    float linear;
    float quadratic;

    // 聚光灯
    vec3  direction;
    float cutOff;
    float outerCutOff;
};

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

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
////    vec3 ambient = light.ambient * material.ambient;
//    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
//
//    vec3 normal = normalize(Normal);
//    vec3 lightDir = normalize(light.position - worldPos);
//    vec3 diffuse = max(0, dot(lightDir, normal)) * vec3(texture(material.diffuse, TexCoords));
//    diffuse = diffuse * light.diffuse;
//
//    vec3 viewDir = normalize(viewPos - worldPos);
////    vec3 reflectDir = reflect(-lightDir, normal);
////    float spec = pow(max(0, dot(viewDir, reflectDir)), 64);
////    vec3 specular = specularStrength * spec * lightColor;
//    vec3 halfDir = normalize(viewDir + lightDir);
//    float spec = pow(max(0, dot(halfDir, normal)), material.shininess);
//    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
//
//    vec3 result = ambient + diffuse + specular;
//
//    // 点光源衰减
////    float distance    = length(light.position - worldPos);
////    float attenuation = 1.0 /
////        (light.constant + light.linear * distance + light.quadratic * (distance * distance));
////    result = result * attenuation;
//
//    // 聚光灯
//    float theta = dot(lightDir, normalize(-light.direction));
//
//    float epsilon   = light.cutOff - light.outerCutOff;
//    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
//    result = result * intensity;
//
//    FragColor = vec4(result, 1.0);

    // 属性
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - worldPos);
    vec3 result = vec3(0, 0, 0);

    // 第一阶段：定向光照
    result = CalcDirLight(dirLight, norm, viewDir);
    // 第二阶段：点光源
    for(int i = 0; i < 2; i++)
        result += CalcPointLight(pointLights[i], norm, worldPos, viewDir);
    // 第三阶段：聚光
    result += CalcSpotLight(spotLight, norm, worldPos, viewDir);

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
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 worldPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - worldPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 衰减
    float distance    = length(light.position - worldPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
    light.quadratic * (distance * distance));
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
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
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 边缘衰减
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient  *= intensity;
    diffuse  *= intensity;
    specular *= intensity;
    return (ambient + diffuse + specular);
}