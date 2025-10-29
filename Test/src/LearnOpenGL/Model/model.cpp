#include <common.h>

const int screenWidth = 800, screenHeight = 800;
glm::vec3 lightPos[2] = {{-1, 1, 2}, {1, -1, -2}};
glm::vec3 lightColor[2] = {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};

int main()
{
    opengl::system::init();
    if (!opengl::system::createWindow("model", screenWidth, screenHeight))
    {
        return -1;
    }

    camera *camera = opengl::system::getCamera();
    glm::mat4 projection =
            glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    camera->setProspectiveMatrix(projection);

    stbi_set_flip_vertically_on_load(true);
    Shader *shader = new Shader("../src/LearnOpenGL/Model/cube.vert", "../src/LearnOpenGL/Model/cube.frag");
    Model *model = new Model(FileSystem::getPath("Assets/Models/backpack/backpack.obj").data());
    shader->use();

    shader->setFloat("material.shininess", 16.0f);
    shader->setVec3("dirLight.direction", {-1, -1, -1});
    shader->setFloat("spotLight.cutOff",   glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight.outerCutOff",   glm::cos(glm::radians(17.5f)));

    while (!opengl::system::systemShouldEnd())
    {
        opengl::system::update([model, shader, camera]() {
            shader->setVec3("viewPos", camera->Position);
            shader->setMat4("model", glm::mat4(1.0f));
            shader->setMat4("view", camera->getViewMatrix());
            shader->setMat4("projection", camera->getProspectiveMatrix());

            shader->setVec3("dirLight.ambient", 0.2f * lightColor[0]);
            shader->setVec3("dirLight.diffuse", 0.5f * lightColor[0]);
            shader->setVec3("dirLight.specular", 1.0f * lightColor[0]);

            shader->setVec3("spotLight.ambient", 0.2f * lightColor[0]);
            shader->setVec3("spotLight.diffuse", 0.5f * lightColor[0]);
            shader->setVec3("spotLight.specular", 1.0f * lightColor[0]);
            shader->setVec3("spotLight.position", camera->Position);
            shader->setVec3("spotLight.direction", camera->Front);

            model->Draw(*shader);
        });
    }

    opengl::system::clear();
    return 0;
}