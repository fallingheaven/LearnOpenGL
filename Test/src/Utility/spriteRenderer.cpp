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
        float width = Game::getInstance()->getWindow()->getWidth();
        float height = Game::getInstance()->getWindow()->getHeight();
        shader->setMat4("orthoProjection", glm::ortho(0.0f, width, 0.0f, height, -1.0f, 1.0f));

        // 绘制四边形
        glBindVertexArray(Game::getInstance()->getCamera()->quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
}