#pragma once

namespace opengl
{
    enum PropsType
    {
        None,
        SPEED_UP,
        STICKY,
        PASS_THROUGH,
        PAD_SIZE_INCREASE,
        CONFUSE,
        CHAOS
    };

    struct Character {
        GLuint     TextureID;  // 字形纹理的ID
        glm::ivec2 Size;       // 字形大小
        glm::ivec2 Bearing;    // 从基准线到字形左部/顶部的偏移值
        glm::ivec2 Advance;    // 原点距下一个字形原点的二维距离
    };
}