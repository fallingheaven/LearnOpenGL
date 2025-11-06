#version 440 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;
const float outlineThickness = 1.5; // 描边厚度
const vec3 outlineColor = vec3(0.0, 1.0, 1.0); // 描边颜色
void main()
{
    float alpha = texture(text, TexCoords).r;

    float outlineAlpha = 0.0;
    float pi = 3.14159265359;
    int samples = 8; // 采样点数量
    for (int i = 0; i < samples; i++) {
        float angle = float(i) / float(samples) * 2.0 * pi;
        vec2 offset = vec2(cos(angle), sin(angle)) * outlineThickness / textureSize(text, 0);
        outlineAlpha = max(outlineAlpha, texture(text, TexCoords + offset).r);
    }

    vec3 finalColor = vec3(0);
    if (outlineAlpha > 0)
    {
        finalColor = outlineColor; // 描边颜色
    }
    if (alpha > 0)
    {
        finalColor = textColor; // 字符颜色
    }

    FragColor = vec4(finalColor, max(outlineAlpha, alpha)); // 字符的alpha也会影响最终颜色
}