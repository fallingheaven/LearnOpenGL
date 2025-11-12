#version 440 core
out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 Bitangent;
} fs_in;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;

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

uniform Material material;

uniform vec3 viewPos;
uniform sampler2DArray shadowMap;
uniform float near;
uniform float far;
uniform float cascadeSplits[5];

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

layout (std140, binding = 1) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[4];
};

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float calcShadow(vec3 normal, vec3 lightDir)
{
    float depthValue = LinearizeDepth(gl_FragCoord.z);
    depthValue = (depthValue - near) / (far - near); // 0.0 - 1.0

    int cascadeIndex = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (depthValue <= cascadeSplits[i+1])
        {
            cascadeIndex = i;
            break;
        }
    }

    vec4 ndc = lightSpaceMatrices[cascadeIndex] * vec4(fs_in.WorldPos, 1.0);
    ndc /= ndc.w;
    vec4 uv = ndc * 0.5 + 0.5;
    if (uv.z > 1.0)
    {
        FragColor = vec4(vec3(0.0), 1.0);
        return 0.0;
    }

//    float bias = 0.005f;
    float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.0001);
    float closestDepth = texture(shadowMap, vec3(uv.xy, cascadeIndex)).r;
//    return float(cascadeIndex) / 4.0;
    if (uv.z > closestDepth + bias)
    {
        return 0.2f;
    }
    else
    {
        return 1.0f;
    }
}

void main()
{
    // 属性
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.WorldPos);
    vec4 result = vec4(0, 0, 0, 1);

    vec4 texColor = texture(material.texture_diffuse1, fs_in.TexCoords);
    result.xyz = CalcDirLight(dirLight, norm, viewDir);

    float shadow = calcShadow(norm, normalize(-dirLight.direction));
    result.xyz *= shadow;
//    FragColor = vec4(vec3(shadow), 1);
//    return;

    FragColor = result;
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
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));
    return (ambient + diffuse + specular);
}