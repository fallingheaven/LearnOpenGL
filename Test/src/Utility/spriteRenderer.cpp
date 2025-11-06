#include <spriteRenderer.h>

namespace opengl
{
    void Texture::init(std::string path, GLuint internalFormat, GLenum format, GLenum dataType, GLenum type)
    {
        this->internalFormat = internalFormat;
        this->format = format;
        this->dataType = dataType;
        this->Type = type;

        glGenTextures(1, &ID);
        glBindTexture(type, ID);

        int nrChannels;
        unsigned char *data = stbi_load(path.c_str(), (int*)&width, (int*)&height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(type, 0, internalFormat, width, height, 0, format, dataType, data);
            glGenerateMipmap(type);

            glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            std::cerr << "Failed to load texture at path: " << path << std::endl;
        }
        stbi_image_free(data);
    }

    void Texture::bind() const
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(Type, ID);
    }

    void Renderer::Draw()
    {
        // 基类不实现具体绘制逻辑
    }

    void SpriteRenderer::Draw()
    {
        // 绑定纹理
        texture->bind();

        // 使用着色器
        shader->use();
        shader->setInt("image", 0);
        shader->setMat4("model", owner->transform.getModelMatrix());
        shader->setVec4("spriteColor", owner->color);
        float width = Game::getInstance()->getWindow()->getWidth();
        float height = Game::getInstance()->getWindow()->getHeight();
        shader->setMat4("orthoProjection", glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f));

        // 绘制四边形
        glBindVertexArray(Game::getInstance()->getCamera()->quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void ParticleRenderer::Draw()
    {
        // 绑定纹理
        texture->bind();

        // 使用着色器
        shader->use();
        shader->setInt("image", 0);
        shader->setVec2("offset", glm::vec2(owner->transform.position.x, owner->transform.position.y));
        shader->setVec4("color", owner->color);
        float width = Game::getInstance()->getWindow()->getWidth();
        float height = Game::getInstance()->getWindow()->getHeight();
        shader->setMat4("orthoProjection", glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f));

        // 绘制四边形
        glBindVertexArray(Game::getInstance()->getCamera()->quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void TextRenderer::Draw()
    {
        auto textOwner = dynamic_cast<TextObject*>(this->owner);
        std::wstring text;
        glm::vec3 color;
        if (textOwner == nullptr) return;
        else
        {
            text = textOwner->getText();
            color = textOwner->getColor();
        }

        float width = Game::getInstance()->getWindow()->getWidth();
        float height = Game::getInstance()->getWindow()->getHeight();
        glm::mat4 projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

        shader->use();
        shader->setVec3("textColor", color);
        shader->setMat4("projection", projection);
        shader->setInt("text", 0);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(textVAO);
        glActiveTexture(GL_TEXTURE0);
        int x = owner->getPosition().x;
        int y = owner->getPosition().y;
        // 假设字体加载时设置的像素大小为 100
        float scale = textOwner->getScale().y / 50.0f;
        float lineHeight = 48.0f * scale * 1.2f; // 设置行高，例如字体大小的1.2倍
        // 遍历文本中所有的字符
        for (wchar_t c : text)
        {
            Character ch = (*Characters)[c];
            if (c == L'\n')
            {
                y += lineHeight;
                x = owner->getPosition().x;
                continue;
            }

            // GLfloat scale = owner->getScale().y;
            GLfloat xpos = x + ch.Bearing.x * scale;
            GLfloat ypos = y + ((*this->Characters)[L'H'].Bearing.y - ch.Bearing.y) * ch.Bearing.x * scale;
            // 这里由于正交投影矩阵上下翻转，所以需要调整y坐标

            GLfloat w = ch.Size.x * scale;
            GLfloat h = ch.Size.y * scale;
            // std::cout << "Rendering char '" << (char)c << "' at (" << xpos << ", " << ypos << ") with size (" << w << ", " << h << ")\n";
            // 为每个字符更新VBO
            GLfloat vertices[6][4] = {
                { xpos,     ypos + h,   0.0, 1.0 },
                { xpos + w, ypos,       1.0, 0.0 },
                { xpos,     ypos,       0.0, 0.0 },

                { xpos,     ypos + h,   0.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 1.0 },
                { xpos + w, ypos,       1.0, 0.0 }
            };
            // 在四边形上绘制字形纹理
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            // 更新VBO内存的内容
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // 绘制四边形
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // 更新位置到下一个字形的原点，注意单位是1/64像素
            glm::ivec2 offset = ch.Advance;
            x += (offset.x >> 6) * scale; // 位偏移6个单位来获取单位为像素的值 (2^6 = 64)
            // y += (offset.y >> 6) * scale; // 同时更新y坐标
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glEnable(GL_DEPTH_TEST);
    }

}

