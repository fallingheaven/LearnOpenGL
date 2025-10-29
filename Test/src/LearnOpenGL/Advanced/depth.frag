#version 440 core
out vec4 FragColor;

float near = 0.1;
float far  = 5.0;

float LinearizeDepth(float z)
{
    float ndcZ = z * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    FragColor = vec4(vec3(LinearizeDepth(gl_FragCoord.z))/far, 1.0);
    FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}