#pragma once
#include <stb_image.h>
#include <iostream>
#include <string>
#include <glad/glad.h>
#include <spriteRenderer.h>
#include <game.h>

namespace opengl
{
    class Object;
    struct Transform;

    class Texture
    {
    public:
        GLuint ID;
        GLenum Type;

        Texture() : ID(0), Type(GL_TEXTURE_2D)
        {
            width = height = 0;
            internalFormat = GL_RGB;
            format = GL_RGB;
            dataType = GL_UNSIGNED_BYTE;
        }

        void init(std::string path, GLuint internalFormat, GLenum format, GLenum dataType, GLenum type = GL_TEXTURE_2D);
        void bind() const;
    private:
        GLuint width, height;
        GLuint internalFormat;
        GLenum format;
        GLenum dataType;
    };

    class Renderer
    {
    public:
        Renderer(Shader* shader)
        {
            this->shader = shader;
        }
        void virtual Draw();
        Object *owner;
    protected:
        Shader *shader;
    };

    class SpriteRenderer : public Renderer
    {
    public:
        SpriteRenderer(Shader *shader, Texture* texture) : Renderer(shader)
        {
            this->texture = texture;
        };
        ~SpriteRenderer() = default;

        void Draw() override;
    private:
        Texture *texture;
    };
}