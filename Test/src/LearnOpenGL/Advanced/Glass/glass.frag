#version 440 core
out vec4 FragColor;

in vec3 Normal;
in vec3 worldPos;

uniform vec3 viewPos;
uniform samplerCube skybox;

void main()
{
    // 反射
//    vec3 I = normalize(worldPos - viewPos);
//    vec3 R = reflect(I, normalize(Normal));

    // 折射
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(worldPos - viewPos);
    vec3 R = refract(I, normalize(Normal), ratio);

    R.y *= -1;
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}