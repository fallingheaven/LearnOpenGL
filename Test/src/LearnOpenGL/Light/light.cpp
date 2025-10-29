#include <common.h>

using namespace opengl;

unsigned int VAO, VBO, EBO;
unsigned int lightVAO;
Shader *cubeShader, *lightShader;
const int screenWidth = 800, screenHeight = 800;

unsigned int texture1, texture2;

glm::vec3 lightPos[2] = {{-1, 1, 2}, {1, -1, -2}};
glm::vec3 lightColor[2] = {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};

void prepareCube()
{
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
    };

    // 索引要求是uint
    unsigned int indices[] ={
        0, 1, 2,
        1, 2, 3
    };

    {
        glGenTextures(1, &texture1);
        glGenTextures(1, &texture2);
        int width, height, nrChannels;
        unsigned char *data = nullptr;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load("container2.png", &width, &height, &nrChannels, 0);
        if (!data)
        {
            std::cerr << "加载纹理出错" << std::endl;
            return;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
        if (!data)
        {
            std::cerr << "加载纹理出错" << std::endl;
            return;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);
}

void prepare_light()
{
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // 共用一个立方体顶点数据，不需要重新绑定VBO和EBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void drawCube()
{
    cubeShader->use();
    {
        glm::mat4 model(1.0f);
        // model = glm::rotate(model, 45.0f, glm::vec3(1.0, 0.0, 0.0));
        // model = glm::scale(model, glm::vec3(1.5f));
        // model = glm::translate(model, glm::vec3(0, -0.5, 0));

        camera* camera = opengl::system::getCamera();

        cubeShader->setVec3("viewPos", camera->Position);
        cubeShader->setMat4("model", model);
        cubeShader->setMat4("view", camera->getViewMatrix());
        cubeShader->setMat4("projection", camera->getProspectiveMatrix());

        cubeShader->setVec3("dirLight.ambient", 0.2f * lightColor[0]);
        cubeShader->setVec3("dirLight.diffuse", 0.5f * lightColor[0]);
        cubeShader->setVec3("dirLight.specular", 1.0f * lightColor[0]);

        cubeShader->setVec3("pointLights[0].ambient", 0.05f * lightColor[1]);
        cubeShader->setVec3("pointLights[0].diffuse", 0.5f * lightColor[1]);
        cubeShader->setVec3("pointLights[0].specular", 1.0f * lightColor[1]);

        cubeShader->setVec3("pointLights[1].ambient", 0.05f * lightColor[1]);
        cubeShader->setVec3("pointLights[1].diffuse", 0.5f * lightColor[1]);
        cubeShader->setVec3("pointLights[1].specular", 1.0f * lightColor[1]);

        cubeShader->setVec3("spotLight.ambient", 0.2f * lightColor[0]);
        cubeShader->setVec3("spotLight.diffuse", 0.5f * lightColor[0]);
        cubeShader->setVec3("spotLight.specular", 1.0f * lightColor[0]);

        // cubeShader->setVec3("light.position", lightPos);
        cubeShader->setVec3("spotLight.position", camera->Position);
        cubeShader->setVec3("spotLight.direction", camera->Front);

        glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
          };

        for(unsigned int i = 0; i < 10; i++)
        {
            model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
            float angle = 20.0f * (float)i;
            if (i % 3 == 0)
                model = glm::rotate(model, glm::radians(angle) + (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
            else
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            cubeShader->setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    lightShader->use();
    {
        for (int i = 0; i < 2; i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, lightPos[i]);
            model = glm::scale(model, glm::vec3(0.2f));

            camera* camera = opengl::system::getCamera();

            lightShader->setMat4("model", model);
            lightShader->setMat4("view", camera->getViewMatrix());
            lightShader->setMat4("projection", camera->getProspectiveMatrix());
            lightShader->setVec3("lightColor", lightColor[i]);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}

int main()
{
    system::init();
    if (!system::createWindow("light", screenWidth, screenHeight))
    {
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); // 确保创建上下文
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(opengl::system::getWindow()->getInstance(), true);
    ImGui_ImplOpenGL3_Init("#version 440");

    camera *camera = system::getCamera();
    glm::mat4 projection =
            glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    camera->setProspectiveMatrix(projection);

    prepareCube();
    cubeShader = new Shader("../src/LearnOpenGL/Light/cube.vert", "../src/LearnOpenGL/Light/cube.frag");
    cubeShader->use();
    // cubeShader->setVec3("material.ambient", {0.5f, 1.0f, 0.5f});
    // cubeShader->setVec3("material.diffuse",  {1.0f, 0.5f, 0.5});
    // cubeShader->setVec3("material.specular", {1.0f, 1.0f, 1.0f});
    cubeShader->setInt("material.diffuse", 0);
    cubeShader->setInt("material.specular", 1);
    cubeShader->setFloat("material.shininess", 16.0f);
    // cubeShader->setVec3("light.ambient", {0.2f, 0.2f, 0.2f});
    // cubeShader->setVec3("light.diffuse", {0.5f, 0.5f, 0.5f});
    // cubeShader->setVec3("light.specular", {1.0f, 1.0f, 1.0f});
    // cubeShader->setVec3("light.position", lightPos);
    // cubeShader->setFloat("light.constant",  1.0f);
    // cubeShader->setFloat("light.linear",    0.09f);
    // cubeShader->setFloat("light.quadratic", 0.032f);
    // cubeShader->setVec3("light.direction", {0, 0, -1.0f});
    // cubeShader->setFloat("light.cutOff",   glm::cos(glm::radians(12.5f)));
    // cubeShader->setFloat("light.outerCutOff",   glm::cos(glm::radians(17.5f)));
    cubeShader->setVec3("dirLight.direction", {-1, -1, -1});
    cubeShader->setVec3("pointLights[0].position", lightPos[0]);
    cubeShader->setVec3("pointLights[1].position", lightPos[1]);
    cubeShader->setFloat("pointLights[0].constant", 1.0f);
    cubeShader->setFloat("pointLights[0].linear",    0.09f);
    cubeShader->setFloat("pointLights[0].quadratic", 0.032f);
    cubeShader->setFloat("pointLights[1].constant", 1.0f);
    cubeShader->setFloat("pointLights[1].linear",    0.09f);
    cubeShader->setFloat("pointLights[1].quadratic", 0.032f);
    cubeShader->setFloat("spotLight.cutOff",   glm::cos(glm::radians(12.5f)));
    cubeShader->setFloat("spotLight.outerCutOff",   glm::cos(glm::radians(17.5f)));

    lightShader = new Shader("../src/LearnOpenGL/Light/light.vert", "../src/LearnOpenGL/Light/light.frag");

    while (!system::systemShouldEnd())
    {
        // system::update(drawCube);

        system::update([]() {
            drawCube();

            // 开始 ImGui 帧
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 添加光源颜色控制
            ImGui::SetNextWindowSize(ImVec2(300, 100));
            ImGui::Begin("Light Settings");
            ImGui::ColorEdit3("Light Color", (float*)&lightColor);
            ImGui::DragFloat3("Light Position", (float*)&lightPos);
            ImGui::End();

            // 渲染 ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        });
    }

    system::close();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return 0;
}