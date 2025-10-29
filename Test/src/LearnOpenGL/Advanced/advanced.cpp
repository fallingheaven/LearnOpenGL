#include <common.h>

using camera = opengl::camera;

const int screenWidth = 800, screenHeight = 800;
glm::vec3 lightPos[2] = {{-1, 1, 2}, {1, -1, -2}};
glm::vec3 lightColor[2] = {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};

unsigned int vegetationVAO;
unsigned int grassTexture;

unsigned int fbo;
unsigned int quadVAO;
unsigned int colorBuffer;

void PrepareGrass()
{
    float vertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  0.0f,

        0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  1.0f
    };

    // unsigned int vegetationVAO;
    glGenVertexArrays(1, &vegetationVAO);
    glBindVertexArray(vegetationVAO);
    unsigned int vegetationVBO;
    glGenBuffers(1, &vegetationVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 模型坐标系下的坐标
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);

    // unsigned int grassTexture;
    glGenTextures(1, &grassTexture);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    int width, height, nrChannels;
    unsigned char *data = nullptr;
    stbi_set_flip_vertically_on_load(true); // png
    // data = stbi_load(FileSystem::getPath("Assets/Materials/grass.png").c_str(), &width, &height, &nrChannels, 0);
    data = stbi_load(FileSystem::getPath("Assets/Materials/red_window.png").c_str(), &width, &height, &nrChannels, 0);

    if (!data)
    {
        std::cerr << "加载纹理出错" << std::endl;
        return;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    stbi_image_free(data);
}
void DrawGrass(Shader* shader, camera *camera)
{
    // glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glBindVertexArray(vegetationVAO);

    std::pmr::vector<glm::vec3> vegetation;
    vegetation.emplace_back(-1.5f,  0.0f, -0.48f);
    vegetation.emplace_back( 1.5f,  0.0f,  0.51f);
    vegetation.emplace_back( 0.0f,  0.0f,  0.7f);
    vegetation.emplace_back(-0.3f,  0.0f, -2.3f);
    vegetation.emplace_back( 0.5f,  0.0f, -0.6f);

    std::sort(vegetation.begin(), vegetation.end(), [&](auto a, auto b) {
        return glm::length(camera->Position - a) > glm::length(camera->Position - b);
    });

    shader->use();
    shader->setVec3("viewPos", camera->Position);
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("projection", camera->getProspectiveMatrix());
    for(auto pos : vegetation)
    {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // glDepthMask(GL_TRUE);
}

// void BindFrameBuffer()
// {
//     // unsigned int fbo;
//     glGenFramebuffers(1, &fbo);
//     glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//
//     // 颜色纹理附件
//     // unsigned int colorBuffer;
//     glGenTextures(1, &colorBuffer);
//     glBindTexture(GL_TEXTURE_2D, colorBuffer);
//
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
//
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
//
//     // 深度&模板纹理附件
//     unsigned int depthStencilBuffer;
//     // glGenTextures(1, &depthStencilBuffer);
//     // glBindTexture(GL_TEXTURE_2D, depthStencilBuffer);
//     // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
//     // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilBuffer, 0);
//
//     unsigned int rbo;
//     glGenRenderbuffers(1, &rbo);
//     glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
//
//     if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//         std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
//
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }
//
// void PrepareQuad()
// {
//     float quadVertices[] = {
//         // positions   // texCoords
//         -1.0f,  1.0f,  0.0f, 1.0f,
//         -1.0f, -1.0f,  0.0f, 0.0f,
//          1.0f, -1.0f,  1.0f, 0.0f,
//
//         -1.0f,  1.0f,  0.0f, 1.0f,
//          1.0f, -1.0f,  1.0f, 0.0f,
//          1.0f,  1.0f,  1.0f, 1.0f
//     };
//
//     glGenVertexArrays(1, &quadVAO);
//     unsigned int quadVBO;
//     glGenBuffers(1, &quadVBO);
//     glBindVertexArray(quadVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)nullptr);
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
// }
//
// void DrawFullScreen(Shader* screenShader, unsigned int quadVAO)
// {
//     glDisable(GL_BLEND);
//     glDisable(GL_DEPTH_TEST);
//
//     glBindFramebuffer(GL_FRAMEBUFFER, 0); // 返回默认
//     glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
//     glClear(GL_COLOR_BUFFER_BIT);
//
//     screenShader->use();
//     glBindVertexArray(quadVAO);
//     glDisable(GL_DEPTH_TEST);
//     glBindTexture(GL_TEXTURE_2D, colorBuffer);
//     glDrawArrays(GL_TRIANGLES, 0, 6);
//
//     glEnable(GL_BLEND);
//     glEnable(GL_DEPTH_TEST);
// }

// std::string faces[6] = {
//     FileSystem::getPath("Assets/Materials/skybox/right.jpg"),
//     FileSystem::getPath("Assets/Materials/skybox/left.jpg"),
//     FileSystem::getPath("Assets/Materials/skybox/bottom.jpg"),
//     FileSystem::getPath("Assets/Materials/skybox/top.jpg"),
//     FileSystem::getPath("Assets/Materials/skybox/front.jpg"),
//     FileSystem::getPath("Assets/Materials/skybox/back.jpg")
// };
//
// unsigned int LoadSkyBox()
// {
//     unsigned int skybox;
//     glGenTextures(1, &skybox);
//     glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
//
//     int width, height, nrChannels;
//     for (int i = 0; i < 6; i++)
//     {
//         if (unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0))
//         {
//             glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//                          0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
//             );
//             stbi_image_free(data);
//         }
//         else
//         {
//             std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
//             stbi_image_free(data);
//         }
//     }
//
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//     return skybox;
// }
//
// unsigned int PrepareSkybox()
// {
//     float skyboxVertices[] = {
//         // positions
//         -1.0f,  1.0f, -1.0f,
//         -1.0f, -1.0f, -1.0f,
//          1.0f, -1.0f, -1.0f,
//          1.0f, -1.0f, -1.0f,
//          1.0f,  1.0f, -1.0f,
//         -1.0f,  1.0f, -1.0f,
//
//         -1.0f, -1.0f,  1.0f,
//         -1.0f, -1.0f, -1.0f,
//         -1.0f,  1.0f, -1.0f,
//         -1.0f,  1.0f, -1.0f,
//         -1.0f,  1.0f,  1.0f,
//         -1.0f, -1.0f,  1.0f,
//
//          1.0f, -1.0f, -1.0f,
//          1.0f, -1.0f,  1.0f,
//          1.0f,  1.0f,  1.0f,
//          1.0f,  1.0f,  1.0f,
//          1.0f,  1.0f, -1.0f,
//          1.0f, -1.0f, -1.0f,
//
//         -1.0f, -1.0f,  1.0f,
//         -1.0f,  1.0f,  1.0f,
//          1.0f,  1.0f,  1.0f,
//          1.0f,  1.0f,  1.0f,
//          1.0f, -1.0f,  1.0f,
//         -1.0f, -1.0f,  1.0f,
//
//         -1.0f,  1.0f, -1.0f,
//          1.0f,  1.0f, -1.0f,
//          1.0f,  1.0f,  1.0f,
//          1.0f,  1.0f,  1.0f,
//         -1.0f,  1.0f,  1.0f,
//         -1.0f,  1.0f, -1.0f,
//
//         -1.0f, -1.0f, -1.0f,
//         -1.0f, -1.0f,  1.0f,
//          1.0f, -1.0f, -1.0f,
//          1.0f, -1.0f, -1.0f,
//         -1.0f, -1.0f,  1.0f,
//          1.0f, -1.0f,  1.0f
//     };
//
//     unsigned int skyboxVAO;
//     glGenVertexArrays(1, &skyboxVAO);
//     glBindVertexArray(skyboxVAO);
//     unsigned int skyboxVBO;
//     glGenBuffers(1, &skyboxVBO);
//     glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0); // 三维UV
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
//
//     return skyboxVAO;
// }
//
// void DrawSkyBox(Shader* skyboxShader, unsigned int skyboxVAO, unsigned int skyboxTex)
// {
//     glDepthMask(GL_FALSE);
//     glBindVertexArray(skyboxVAO);
//
//     glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
//
//     skyboxShader->use();
//     auto camera = opengl::system::getCamera();
//     auto view = glm::mat4(glm::mat3(camera->getViewMatrix())); // 去掉平移部分
//     skyboxShader->setVec3("viewPos", camera->Position);
//     skyboxShader->setMat4("view", view);
//     skyboxShader->setMat4("projection", camera->getProspectiveMatrix());
//     glDrawArrays(GL_TRIANGLES, 0, 36);
//
//     glDepthMask(GL_TRUE);
// }

struct MatrixBuffer
{
    glm::mat4 projection;
    glm::mat4 view;
};

unsigned int BindUniformBlock(unsigned int bindingPoint)
{
    unsigned int ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixBuffer), nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);

    return ubo;
}

// glm::vec3 GetRandomVec3(float min, float max)
// {
//     float x = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max - min)));
//     float y = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max - min)));
//     float z = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max - min)));
//     return {x, y, z};
// }
//
// unsigned int BindInstancedBuffer(int amount)
// {
//     std::vector<glm::mat4> modelMatrices;
//     for (int i = 0; i < amount; i++)
//     {
//         glm::mat4 tmp(1.0f);
//         tmp = glm::translate(tmp, GetRandomVec3(-5, 5));
//         tmp = glm::scale(tmp, glm::vec3(0.1f, 0.1f, 0.1f));
//         modelMatrices.push_back(tmp);
//     }
//
//     unsigned int buffer;
//     glGenBuffers(1, &buffer);
//     glBindBuffer(GL_ARRAY_BUFFER, buffer);
//     glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), modelMatrices.data(), GL_STATIC_DRAW);
//
//     return buffer;
// }

int main()
{
    opengl::system::init();
    if (!opengl::system::createWindow("model", screenWidth, screenHeight))
    {
        return -1;
    }

    camera *camera = opengl::system::getCamera();
    // glm::mat4 projection =
    //         glm::perspective(glm::radians(45.0f), (float)opengl::window::getWidth() / (float)opengl::window::getHeight(), 0.1f, 100.0f);
    // camera->setProspectiveMatrix(projection);

    // stbi_set_flip_vertically_on_load(true);
    Shader *blinnPhongShader = new Shader("../src/LearnOpenGL/Advanced/cube.vert", "../src/LearnOpenGL/Advanced/cube.frag");
    Shader *blinnPhongGShader = new Shader("../src/LearnOpenGL/Advanced/cube.vert", "../src/LearnOpenGL/Advanced/cube.frag", "../src/LearnOpenGL/Advanced/explode.geom");
    Shader *depthShader = new Shader("../src/LearnOpenGL/Advanced/cube.vert", "../src/LearnOpenGL/Advanced/depth.frag");
    Shader *outlineShader = new Shader("../src/LearnOpenGL/Advanced/cube.vert", "../src/LearnOpenGL/Advanced/outline.frag");
    Shader *imageShader = new Shader("../src/LearnOpenGL/Advanced/cube.vert", "../src/LearnOpenGL/Advanced/image.frag");
    Shader *screenShader = new Shader("../src/LearnOpenGL/Advanced/screen.vert", "../src/LearnOpenGL/Advanced/screen.frag");
    Shader *reverseColorFullScreenShader = new Shader("../src/LearnOpenGL/Advanced/screen.vert", "../src/LearnOpenGL/Advanced/reverse_color_fullscreen.frag");
    Shader *grayColorFullScreenShader = new Shader("../src/LearnOpenGL/Advanced/screen.vert", "../src/LearnOpenGL/Advanced/gray_color_fullscreen.frag");
    Shader *sharpenFullScreenShader = new Shader("../src/LearnOpenGL/Advanced/screen.vert", "../src/LearnOpenGL/Advanced/sharpen_fullscreen.frag");
    Shader *blurFullScreenShader = new Shader("../src/LearnOpenGL/Advanced/screen.vert", "../src/LearnOpenGL/Advanced/blur_fullscreen.frag");
    Shader *edgeFullScreenShader = new Shader("../src/LearnOpenGL/Advanced/screen.vert", "../src/LearnOpenGL/Advanced/edge_fullscreen.frag");
    // Shader *skyboxShader = new Shader("../src/LearnOpenGL/Advanced/Skybox/skybox.vert", "../src/LearnOpenGL/Advanced/Skybox/skybox.frag");
    Shader *glassShader = new Shader("../src/LearnOpenGL/Advanced/Glass/glass.vert", "../src/LearnOpenGL/Advanced/Glass/glass.frag");
    Shader *displayNormalShader = new Shader("../src/LearnOpenGL/Advanced/DisplayNormal/display_normal.vert", "../src/LearnOpenGL/Advanced/DisplayNormal/display_normal.frag",
        "../src/LearnOpenGL/Advanced/DisplayNormal/display_normal.geom");

    Model *model1 = new Model(FileSystem::getPath("Assets/Models/MarbleBlock/大理石块.obj").data());
    Model *model2 = new Model(FileSystem::getPath("Assets/Models/MarbleBlock/大理石块.obj").data());
    Model *model3 = new Model(FileSystem::getPath("Assets/Models/backpack/backpack.obj").data());
    Model *model4 = new Model(FileSystem::getPath("Assets/Models/warman/nanosuit.obj").data());
    unsigned int matrixUbo = BindUniformBlock(0);

    // PrepareGrass();
    // PrepareQuad();
    // unsigned int skyboxTex = LoadSkyBox();
    // unsigned int skyboxVAO = PrepareSkybox();
    // auto shader = blinnPhongGShader;
    auto shader = blinnPhongShader;
    shader->use();

    shader->setFloat("material.shininess", 16.0f);
    shader->setVec3("dirLight.direction", {-1, -1, -1});
    shader->setFloat("spotLight.cutOff",   glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight.outerCutOff",   glm::cos(glm::radians(17.5f)));

    // BindFrameBuffer();

    // std::vector<glm::vec3> objectPositions;
    // for (int i = 0; i < 5000; i++)
    //     objectPositions.push_back(GetRandomVec3(-5, 5));

    unsigned int instanceVBO = Utility::GenModelMatBuffer(1000, glm::vec3(0.1f));
    model1->SetAttributeMat4(instanceVBO, 3, 1);
    model2->SetAttributeMat4(instanceVBO, 3, 1);
    model3->SetAttributeMat4(instanceVBO, 3, 1);
    model4->SetAttributeMat4(instanceVBO, 3, 1); // 从顶点属性位置3开始，1表示每个实例更新一次;

    // unsigned int instanceVBO = BindInstancedBuffer();
    // for(auto & mesh : model1->meshes) // 一般是访问不到meshes和VAO的，这里是为了演示实例化
    // {
    //     unsigned int VAO = mesh.VAO;
    //     glBindVertexArray(VAO);
    //     // 顶点属性
    //     GLsizei vec4Size = sizeof(glm::vec4);
    //     glEnableVertexAttribArray(3);
    //     glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)nullptr);
    //     glEnableVertexAttribArray(4);
    //     glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
    //     glEnableVertexAttribArray(5);
    //     glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
    //     glEnableVertexAttribArray(6);
    //     glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
    //
    //     glVertexAttribDivisor(3, 1);
    //     glVertexAttribDivisor(4, 1);
    //     glVertexAttribDivisor(5, 1);
    //     glVertexAttribDivisor(6, 1);
    //
    //     glBindVertexArray(0);
    // }

    while (!opengl::system::systemShouldEnd())
    {
        // glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        opengl::system::update([model1, model2, model3, model4, shader, camera, outlineShader, imageShader, screenShader,
            reverseColorFullScreenShader, grayColorFullScreenShader, sharpenFullScreenShader, blurFullScreenShader, edgeFullScreenShader,
            // skyboxShader, skyboxVAO, skyboxTex,
            matrixUbo, displayNormalShader,
            glassShader]() {
                glBindBuffer(GL_UNIFORM_BUFFER, matrixUbo);
                MatrixBuffer matrices = {
                    .projection = camera->getProspectiveMatrix(),
                    .view = camera->getViewMatrix(),
                };
                glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MatrixBuffer), &matrices);

                // 先绘制天空盒会在之后再对每一个像素进行深度测试，开销较大
                // DrawSkyBox(skyboxShader, skyboxVAO, skyboxTex);

                shader->use();
                shader->setVec3("viewPos", camera->Position);
                // shader->setMat4("model", glm::mat4(1.0f));
                // shader->setMat4("view", camera->getViewMatrix());
                // shader->setMat4("projection", camera->getProspectiveMatrix());

                shader->setVec3("dirLight.ambient", 0.2f * lightColor[0]);
                shader->setVec3("dirLight.diffuse", 0.5f * lightColor[0]);
                shader->setVec3("dirLight.specular", 1.0f * lightColor[0]);

                shader->setVec3("spotLight.ambient", 0.2f * lightColor[0]);
                shader->setVec3("spotLight.diffuse", 0.5f * lightColor[0]);
                shader->setVec3("spotLight.specular", 1.0f * lightColor[0]);
                shader->setVec3("spotLight.position", camera->Position);
                shader->setVec3("spotLight.direction", camera->Front);
                shader->setFloat("time", (float)glfwGetTime());

                auto model_transform1 = glm::mat4(1.0f), model_transform2 = glm::mat4(1.0f), model_transform3 = glm::mat4(1.0f);
                // model_transform1 = glm::scale(model_transform1, glm::vec3(0.5f, 0.5f, 0.5f));
                // model_transform2 = glm::translate(model_transform2, glm::vec3(0, -1.5f, 0));
                // shader->setMat4("model", model_transform1);
                // model1->Draw(*shader);
                // shader->setMat4("model", model_transform2);
                // model2->Draw(*shader);

                // 玻璃材质（反射/折射）
                // glassShader->use();
                // model_transform3 = glm::rotate(model_transform3, glm::radians((float)glfwGetTime()*20), glm::vec3(0, 1, 0));
                // glassShader->setMat4("model", model_transform3);
                // glassShader->setVec3("viewPos", camera->Position);
                // glassShader->setMat4("view", camera->getViewMatrix());
                // glassShader->setMat4("projection", camera->getProspectiveMatrix());
                // glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
                // model3->Draw(*glassShader);

                // glActiveTexture(GL_TEXTURE5);
                // glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
                // camera->bindSkybox(5);

                // for (int i = 0; i < 1000; i++)
                // {
                //     // model_transform3 = glm::translate(model_transform3, glm::vec3(0, -1.0f, 0));
                //     model_transform3 = glm::translate(glm::mat4(1.0f), objectPositions[i]);
                //     model_transform3 = glm::scale(model_transform3, glm::vec3(0.1f, 0.1f, 0.1f));
                //     // model_transform3 = glm::rotate(model_transform3, glm::radians((float)glfwGetTime()*20), glm::vec3(0, 1, 0));
                //     shader->setInt("skybox", 5);
                //     shader->setMat4("model", model_transform3);
                //     model4->Draw(*shader);
                // }

                // model_transform3 = glm::translate(model_transform3, glm::vec3(0, -1.0f, 0));
                // model_transform3 = glm::scale(model_transform3, glm::vec3(0.1f, 0.1f, 0.1f));
                // // model_transform3 = glm::rotate(model_transform3, glm::radians((float)glfwGetTime()*20), glm::vec3(0, 1, 0));
                // shader->setInt("skybox", 5);
                // shader->setMat4("model", model_transform3);
                // model4->Draw(*shader);

                // 实例化，使用实例化数组的话需要在着色器中注释model才好，要用layout来匹配
                {
                    model3->DrawInstanced(*shader, 100);
                    model4->DrawInstanced(*shader, 100);


                    // for(auto & meshe : model1->meshes)
                   // {
                   //     glBindVertexArray(meshe.VAO);
                   //     glDrawElementsInstanced(
                   //         GL_TRIANGLES, meshe.indices.size(), GL_UNSIGNED_INT, 0, 5000
                   //     );
                   // }
                   // for (int i = 0; i < 5000; i++)
                   // {
                   //     // model_transform3 = glm::translate(model_transform3, glm::vec3(0, -1.0f, 0));
                   //     model_transform1 = glm::translate(glm::mat4(1.0f), objectPositions[i]);
                   //     model_transform1 = glm::scale(model_transform1, glm::vec3(0.1f, 0.1f, 0.1f));
                   //     // model_transform3 = glm::rotate(model_transform3, glm::radians((float)glfwGetTime()*20), glm::vec3(0, 1, 0));
                   //     shader->setMat4("model", model_transform1);
                   //     model1->Draw(*shader);
                   // }
                }

                // 几何着色器绘制法线
                // displayNormalShader->use();
                // displayNormalShader->setMat4("model", model_transform3);
                // model4->Draw(*displayNormalShader);

                // DrawGrass(imageShader, camera);

                // DrawSkyBox(skyboxShader, skyboxVAO, skyboxTex);

                // DrawFullScreen(screenShader, quadVAO);
                // DrawFullScreen(reverseColorFullScreenShader, quadVAO);
                // DrawFullScreen(grayColorFullScreenShader, quadVAO);
                // DrawFullScreen(sharpenFullScreenShader, quadVAO);
                // DrawFullScreen(blurFullScreenShader, quadVAO);
                // DrawFullScreen(edgeFullScreenShader, quadVAO);

                // 模板测试描边示例
                {
                    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // 都通过测试时替换为参考值
                    // glStencilFunc(GL_ALWAYS, 1, 0xFF); // 对物体1进行模板写入
                    // glStencilMask(0xFF);
                    // shader->setMat4("model", model_transform1);
                    // model1->Draw(*shader);
                    //
                    // glStencilMask(0x00);// 对另一个不写入模板缓冲
                    // shader->setMat4("model", model_transform2);
                    // model2->Draw(*shader);
                    //
                    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                    // glStencilMask(0x00);
                    // glDisable(GL_DEPTH_TEST); // 取消深度测试，让物体绘制再
                    // outlineShader->use();
                    // auto outline_transform = glm::scale(model_transform1, glm::vec3(1.1f)); // 描边放大一些
                    // outlineShader->setVec3("viewPos", camera->Position);
                    // outlineShader->setMat4("model", outline_transform);
                    // outlineShader->setMat4("view", camera->getViewMatrix());
                    // outlineShader->setMat4("projection", camera->getProspectiveMatrix());
                    // model1->Draw(*outlineShader);
                    // glEnable(GL_DEPTH_TEST);
                    // glStencilMask(0xFF);
                }
            });
    }

    opengl::system::clear();
    return 0;
}