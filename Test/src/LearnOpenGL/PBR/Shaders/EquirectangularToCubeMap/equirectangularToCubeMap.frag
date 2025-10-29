#version 440 core
out vec4 FragColor;

in vec3 sampleDir;

uniform sampler2D equirectangularMap;

// invAtan = vec2(1/(2π), 1/π)
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    // uv.x \in [-PI, PI], uv.y \in [-PI/2, PI/2]
    // 将其映射到 [0,1] 范围内
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(sampleDir));
    vec3 color = texture(equirectangularMap, uv).rgb;

    FragColor = vec4(color, 1.0);
}