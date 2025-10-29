#version 440 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

uniform vec3 viewPos;

uniform bool useSSAO;

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

void main()
{
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float shininess = texture(gAlbedoSpec, TexCoords).a;
    float ambientOcclusion = useSSAO? texture(ssao, TexCoords).r : 1.0;

    vec3 result = vec3(0);

    for (int i = 0; i < 1; i++)
    {
        vec3 lightDir = normalize(pointLights[i].position - worldPos);
        float diffuseStrength = max(dot(lightDir, normal), 0.0);
        vec3 viewDir = normalize(viewPos - worldPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), shininess);

        float distance = length(pointLights[i].position - worldPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * (distance * distance));

        vec3 ambient = pointLights[i].ambient * albedo * ambientOcclusion;
        vec3 diffuse = pointLights[i].diffuse * diffuseStrength * albedo;
        vec3 specular = pointLights[i].specular * specularStrength * albedo;
        result += (ambient + diffuse + specular) * attenuation;
    }

    FragColor = vec4(result, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}