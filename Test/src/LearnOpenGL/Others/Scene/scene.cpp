#include <common.h>
using namespace opengl;

std::map<std::string, Model*> models;
std::map<std::string, Entity*> entities;
std::map<std::string, Shader*> shaders;

Entity* root;

void loadModel()
{
    auto XiLianModel = new Model(FileSystem::getPath("Assets/Models/XiLian/星穹铁道—昔涟5.pmx").data());
    models["XiLian"] = XiLianModel;

    for (int i = 0; i < 5; i++)
    {
        auto XiLian = new Entity(XiLianModel);
        entities["XiLian"+std::to_string(i)] = XiLian;

        if (i == 0)
        {
            root = XiLian;
            XiLian->setScale({0.01f, 0.01f, 0.01f});
        }
        else
        {
            entities["XiLian"+std::to_string(i-1)]->addChild(XiLian);
            XiLian->setPosition({20.0f, 0.0f, 0.0f});
            XiLian->setScale({0.75f, 0.75f, 0.75f});
        }
    }
}

void loadShader()
{
    auto simpleShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Others/Animation/Shaders/simple.vert"),
                            FileSystem::getPath("src/LearnOpenGL/Others/Animation/Shaders/simple.frag"));
    shaders["simple"] = simpleShader;
}

void drawEntity(Entity* entity, Shader* shader)
{
    if (entity == nullptr) return;

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw Entity");
    shader->setMat4("model", entity->getModelMatrix());

    entity->Draw(*shader);
    glPopDebugGroup();
    for (auto child : entity->getChildren())
    {
        drawEntity(child, shader);
    }
}

void updateTransform()
{
    // for (int i = 1; i < 5; i++)
    // {
    //     auto XiLian = entities["XiLian" + std::to_string(i)];
    //     float angle = (float)glfwGetTime() * 50.0f + i * 360.0f / 5.0f;
    //     float radius = 20.0f;
    //     float x = radius * cos(glm::radians(angle));
    //     float y = radius * sin(glm::radians(angle));
    //     XiLian->setPosition({x, y, 0});
    // }
}

int main()
{
    system::init();
    if (!system::createWindow("Scene Class Test")) return -1;

    loadModel();
    loadShader();

    while (!system::systemShouldEnd())
    {
        updateTransform();

        system::update([]() {
            auto shader = shaders["simple"];
            shader->use();
            drawEntity(root, shader);
        });
    }
    return 0;
}