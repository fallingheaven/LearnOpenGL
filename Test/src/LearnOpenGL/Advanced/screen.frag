#version 440 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;

uniform bool hdr;

vec3 acesFilmicToneMapping(vec3 color);
vec3 uncharted2ToneMapping(vec3 color);

void main()
{
    const float gamma = 2.2;
    vec3 mapped = vec3(0.0);
    if (!hdr)
    {
        vec3 color = texture(screenTexture, TexCoords).rgb;
        vec3 bloom = texture(bloomBlur, TexCoords).rgb;
//        color = mix(color, bloom, length(bloom));
        color += bloom;
        mapped = clamp(color, 0.0, 1.0);
    }
    else
    {
        vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
        vec3 bloom = texture(bloomBlur, TexCoords).rgb;
//        hdrColor = mix(hdrColor, bloom, length(bloom));
        hdrColor += bloom;

//        // Reinhard色调映射
//        mapped = hdrColor / (hdrColor + vec3(1.0));
        // 曝光色调映射
        mapped = vec3(1.0) - exp(-hdrColor * 1.0);
//        mapped = acesFilmicToneMapping(hdrColor);
//        mapped = uncharted2ToneMapping(hdrColor);

        FragColor = vec4(mapped, 1.0);
    }

    // Gamma校正
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}

// ACES Filmic 色调映射 (近似版，由 Krzysztof Narkowicz 提供)
// 能产生非常自然的对比度和色彩
vec3 acesFilmicToneMapping(vec3 color)
{
    color *= 0.6; // 降低输入，适应ACES曲线
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}


// Uncharted 2 色调映射 (John Hable)
// 以其在《神秘海域2》中的使用而闻名
vec3 F(vec3 x)
{
    const float A = 0.22f;
    const float B = 0.30f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.01f;
    const float F = 0.30f;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2ToneMapping(vec3 color)
{
    const float WHITE = 11.2f;
    return F(color) / F(vec3(WHITE));
}