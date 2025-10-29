#version 440 core
out vec4 FragColor;

in vec3 Normal;
in vec3 worldPos;
in vec2 TexCoords;

struct Material {
    //    sampler2D diffuse;
    //    sampler2D specular;
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
//    sampler2D texture_height1;
    sampler2D texture_reflection1;

    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

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
uniform vec3 viewPos;

uniform samplerCube skybox;

void main()
{
    // 属性
//    vec3 norm = normalize(Normal);
    vec3 norm = normalize(texture(material.texture_normal1, TexCoords).rgb);
    vec3 viewDir = normalize(viewPos - worldPos);
    vec3 result = vec3(0, 0, 0);

    // 第一阶段：定向光照
    result = CalcDirLight(dirLight, norm, viewDir);
    // 第三阶段：聚光
    result += CalcSpotLight(spotLight, norm, worldPos, viewDir);

    vec3 reflection = vec3(texture(material.texture_reflection1, TexCoords));
    vec3 I = normalize(worldPos - viewPos);
    vec3 R = reflect(I, norm);
    vec3 reflectionColor = texture(skybox, R).rgb;
    result = mix(result, reflectionColor, reflection.r * 0.8f);

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
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
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
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    ambient  *= intensity;
    diffuse  *= intensity;
    specular *= intensity;
    return (ambient + diffuse + specular);
}