#include <common.h>
using namespace opengl;

int main()
{
    auto gameInstance = Game::getInstance();
    if (!gameInstance->createWindow("practice"))
    {
        return -1;
    }

    GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(Utility::glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    auto testObj = new Object(gameInstance->getScene());
    testObj->setPosition(glm::vec3(400, 400, 0));
    testObj->setScale(glm::vec3(100, 100, 1));
    auto spriteShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Practice/Shaders/sprite.vert"),
                                   FileSystem::getPath("src/LearnOpenGL/Practice/Shaders/sprite.frag"));
    auto texture = new opengl::Texture();
    texture->init(FileSystem::getPath("Assets/Materials/marble.jpg"), GL_RGB16, GL_RGB, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
    testObj->setRenderer(new SpriteRenderer(spriteShader, texture));

    while (!gameInstance->systemShouldEnd())
    {
        gameInstance->update([](){

        });


    }

    return 0;
}