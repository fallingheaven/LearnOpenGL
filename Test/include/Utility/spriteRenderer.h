#pragma once
#include <stb_image.h>
#include <iostream>
#include <string>
#include <glad/glad.h>
#include <spriteRenderer.h>
#include <game.h>
#include <types.h>

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
            width = 0;
            height = 0;
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
        virtual ~Renderer() = default;

        Renderer(Shader* shader)
        {
            this->shader = shader;
            this->owner = nullptr;
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

        void Draw() override;
    private:
        Texture *texture;
    };

    class ParticleRenderer : public Renderer
    {
    public:
        ParticleRenderer(Shader *shader, Texture* texture) : Renderer(shader)
        {
            this->texture = texture;
        };

        void Draw() override;
    private:
        Texture *texture;
    };

    class TextRenderer : public Renderer
    {
    public:
        TextRenderer(Shader *shader, unsigned int VAO, unsigned int VBO, std::map<wchar_t, Character> *characters) : Renderer(shader)
        {
            this->textVAO = VAO;
            this->textVBO = VBO;
            this->Characters = characters;
        };

        void Draw() override;
    private:
        std::map<wchar_t, Character>* Characters;
        unsigned int textVAO, textVBO;
    };
}