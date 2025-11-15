#include <common.h>
using namespace opengl;

std::map<std::string, Model*> models;
std::map<std::string, Entity*> entities;
std::map<std::string, Shader*> shaders;

Entity* root;

Frustum camFrustum;

int modelCount = 10;

void loadModel()
{
    auto XiLianModel = new Model(FileSystem::getPath("Assets/Models/XiLian/星穹铁道—昔涟5.pmx").data());
    models["XiLian"] = XiLianModel;

    auto volume1 = new SphereVolume(glm::vec3(0.0f), 15.0f);
    auto volume2 = new AABBVolume(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    for (int i = 0; i < modelCount; i++)
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
            XiLian->setPosition({50.0f, 0.0f, 0.0f});
            XiLian->setScale({0.9f, 0.9f, 0.9f});
        }
        XiLian->setVolume(volume1);
    }
}

void loadShader()
{
    auto simpleShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Others/Animation/Shaders/simple.vert"),
                            FileSystem::getPath("src/LearnOpenGL/Others/Animation/Shaders/simple.frag"));
    shaders["simple"] = simpleShader;
}

int count = 0;

void drawEntity(Entity* entity, Shader* shader)
{
    if (entity == nullptr) return;

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw Entity");
    shader->setMat4("model", entity->getModelMatrix());

    if (entity->isOnFrustum(camFrustum))
    {
        count++;
        entity->Draw(*shader);
    }

    glPopDebugGroup();
    for (auto child : entity->getChildren())
    {
        drawEntity(child, shader);
    }
}

void updateTransform()
{
    for (int i = 1; i < modelCount; i++)
    {
        auto XiLian = entities["XiLian" + std::to_string(i)];
        float angle = (float)glfwGetTime() * 50.0f + i * 360.0f / 5.0f;
        float radius = 50.0f;
        float x = radius * cos(glm::radians(angle));
        float z = radius * sin(glm::radians(angle));
        XiLian->setPosition({x, 0, z});
    }
}

int main()
{
    system::init();
    if (!system::createWindow("Scene Class Test")) return -1;

    loadModel();
    loadShader();

    float aspect = (float)window::getWidth() / (float)window::getHeight();

    while (!system::systemShouldEnd())
    {
        // updateTransform();
        camFrustum = createFrustumFromCamera(*system::getCamera(), aspect, glm::radians(system::getCamera()->Zoom), system::getCamera()->getNearPlane(), system::getCamera()->getFarPlane());

        system::update([]() {
            auto shader = shaders["simple"];
            shader->use();
            count = 0;
            drawEntity(root, shader);
            std::cout << "绘制数量：" << count << std::endl;
        });
    }
    return 0;
}