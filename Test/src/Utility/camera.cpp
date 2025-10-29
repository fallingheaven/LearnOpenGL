#include "camera.h"

using namespace opengl;

camera::camera(glm::vec3 pos, glm::vec3 up, float yaw, float pitch)
    : Front(0, 0, -1), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f), prospectiveMatrix(1)
{
    Position = pos;
    WorldUp  = up;
    Yaw      = yaw;
    Pitch    = pitch;
    updateCameraVectors();
}

void camera::setSkyboxPath(const std::string& path)
{
    skyboxPath = path;
}

void camera::loadSkyBox()
{
    glGenTextures(1, &skyboxTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

    std::string faces[6] = {
        FileSystem::getPath(skyboxPath + "right.jpg"),
        FileSystem::getPath(skyboxPath + "left.jpg"),
        FileSystem::getPath(skyboxPath + "top.jpg"),
        FileSystem::getPath(skyboxPath + "bottom.jpg"),
        FileSystem::getPath(skyboxPath + "front.jpg"),
        FileSystem::getPath(skyboxPath + "back.jpg")
    };

    int width, height, nrChannels;
    for (int i = 0; i < 6; i++)
    {
        if (unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0))
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void camera::prepareSkybox()
{
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);

    glGenBuffers(1, &skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // 三维UV
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
}

void camera::bindSkybox(int index)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
}


void camera::drawSkybox()
{
    glDepthMask(GL_FALSE);
    glBindVertexArray(skyboxVAO);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

    skyboxShader->use();
    auto view = glm::mat4(glm::mat3(getViewMatrix())); // 去掉平移部分
    skyboxShader->setVec3("viewPos", Position);
    skyboxShader->setMat4("view", view);
    skyboxShader->setMat4("projection", getProspectiveMatrix());
    skyboxShader->setInt("skybox", 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
}

void camera::prepareFullScreen()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &screenVAO);
    // unsigned int quadVBO;
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));

    glBindVertexArray(0);
}

void camera::drawFullScreen(bool renderToDisplayFBO)
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, renderToDisplayFBO? displayFBO : 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    fullScreenShader->use();
    fullScreenShader->setBool("hdr", system::useHDR);
    fullScreenShader->setInt("screenTexture", 0);
    fullScreenShader->setInt("bloomBlur", 1);
    glBindVertexArray(screenVAO);
    // 绑定主场景纹理到单元 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[0]);

    // 绑定模糊后的高光纹理到单元 1
    glActiveTexture(GL_TEXTURE1);
    // 经过10次迭代，最终的模糊结果在 pingPongColorBuffers[0] 中
    glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[0]);

    // 下面用于调试多渲染目标
    // for (int i = 0; i < 2; i++)
    // {
    //     glActiveTexture(GL_TEXTURE0 + i);
    //     glBindTexture(GL_TEXTURE_2D, colorBufferTex[i]);
    // }
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void camera::drawFullScreen(unsigned int tex)
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

    fullScreenShader->use();
    fullScreenShader->setBool("hdr", system::useHDR);
    fullScreenShader->setInt("screenTexture", 0);
    glBindVertexArray(screenVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void camera::drawFullScreen(Shader* shader, unsigned int tex)
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

    shader->use();

    glBindVertexArray(screenVAO);
    if (tex != -1)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void camera::genScreenFrameBuffer()
{
    // unsigned int fbo;
    glGenFramebuffers(1, &screenFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

    // 颜色纹理附件
    // 可以创建多个颜色附件，然后用glDrawBuffers指定绘制到哪些颜色附件上
    // unsigned int colorBuffer;
    int colorBufferCount = 2;
    glGenTextures(colorBufferCount, colorBufferTex);
    for (int i = 0; i < colorBufferCount; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[i]);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGBA, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBufferTex[i], 0);
    }

    // 设置绘制的目标有哪些（只是设置附件不会让OpenGL绘制，必须指定）
    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    // 深度&模板纹理附件
    unsigned int depthStencilBuffer;
    // glGenTextures(1, &depthStencilBuffer);
    // glBindTexture(GL_TEXTURE_2D, depthStencilBuffer);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilBuffer, 0);

    // unsigned int rbo;
    glGenRenderbuffers(1, &depthStencilRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, opengl::window::getWidth(), opengl::window::getHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilRBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void camera::genDisplayFrameBuffer()
{
    glGenFramebuffers(1, &displayFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, displayFBO);

    // 颜色纹理附件
    glGenTextures(1, &displayColorBufferTex);
    glBindTexture(GL_TEXTURE_2D, displayColorBufferTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayColorBufferTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Display Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void camera::bindScreenFrameBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
    // glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void camera::genPingPongFrameBuffer()
{
    glGenFramebuffers(2, pingPongFBO);
    glGenTextures(2, pingPongColorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[i]);

        glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongColorBuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "ERROR::FRAMEBUFFER:: PingPong Framebuffer is not complete!" << std::endl;
    }
    // glGenTextures(1, pingPongColorBuffers);
    // glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[0]);
    // // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGBA, GL_FLOAT, nullptr);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void camera::drawPingPongFrameBuffer()
{
    bool begin = true, horizontal = true;
    blurShader->use();
    blurShader->setInt("image", 0);
    for (int i = 0; i < 10; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[horizontal]);
        blurShader->setBool("horizontal", horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, begin ? colorBufferTex[1] : pingPongColorBuffers[!horizontal]);

        glBindVertexArray(screenVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        horizontal = !horizontal;
        begin = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 设置计算着色器
    // Shader* blurComputeShader = new Shader(
    //     FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/GaussianBlur/gaussian_blur.comp")
    // );
    // blurComputeShader->use();
    //
    // int width = window::getWidth();
    // int height = window::getHeight();
    //
    // glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    //
    // // 绑定图像单元
    // glBindImageTexture(0, colorBufferTex[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    // glBindImageTexture(1, pingPongColorBuffers[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    //
    // // 单次调度完成所有模糊工作！
    // glDispatchCompute(
    //     (width + 15) / 16,  // 工作组X数量
    //     (height + 15) / 16, // 工作组Y数量
    //     1                   // 工作组Z数量
    // );
    //
    // // 等待计算完成
    // glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //
    // glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    // glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
}

void camera::drawFullScreenTex(unsigned int fbo, Shader *shader)
{
    glBindVertexArray(screenVAO);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}


void camera::genGBufferFrameBuffer()
{
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

    // 位置颜色附件
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // 法线颜色附件
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // 颜色+镜面反射颜色附件
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // 添加深度缓冲附件
    // glGenTextures(1, &gDepth);
    // glBindTexture(GL_TEXTURE_2D, gDepth);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

    glGenRenderbuffers(1, &gDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, opengl::window::getWidth(), opengl::window::getHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gDepth);


    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // 检查帧缓冲完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::FRAMEBUFFER:: GBuffer framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void camera::drawGBufferFrameBuffer(const std::function<void(Shader *)> &renderScene)
{
    glViewport(0, 0, opengl::window::getWidth(), opengl::window::getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glClearColor(0, 0, 0, far_plane+1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_BLEND);

    gBufferShader->use();
    gBufferShader->setFloat("material.shininess", 8.0f);

    renderScene(gBufferShader);

    glEnable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, opengl::window::getWidth(), opengl::window::getHeight());
}

void camera::copyDepthGBufferToCameraFrameBuffer() const
{
    int width = opengl::window::getWidth(), height = opengl::window::getHeight();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFBO); // 写入到相机帧缓冲
    glBlitFramebuffer(
      0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
}


void camera::genSSAOFBO()
{
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

    ssaoKernel = Utility::genSSAOKernel(64);
    ssaoNoiseTex = Utility::genSSAONoiseTexture(16);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void camera::drawSSAOBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    ssaoShader->use();
    ssaoShader->setVec3("sampleKernel", ssaoKernel[0], 64);
    ssaoShader->setMat4("projection", prospectiveMatrix);
    ssaoShader->setMat4("view", getViewMatrix());
    ssaoShader->setInt("gPositoin", 0);
    ssaoShader->setInt("gNormal", 1);
    ssaoShader->setInt("ssaoNoiseTex", 2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex);

    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void camera::bindUniformBlock(unsigned int bindingPoint, GLsizei blockSize)
{
    glGenBuffers(1, &ubo[bindingPoint]);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo[bindingPoint]);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo[bindingPoint]);
}

void camera::updateViewProjectionMatrix()
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo[0]);
    MatrixBuffer matrices = {
        .projection = prospectiveMatrix,
        .view = getViewMatrix(),
    };
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MatrixBuffer), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0); // 解绑
}

glm::mat4 camera::getViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

void camera::setProspectiveMatrix(glm::mat4 mat)
{
    prospectiveMatrix = mat;
}

glm::mat4 camera::getProspectiveMatrix() const
{
    return prospectiveMatrix;
}

void camera::processKeyboard(Camera_Movement dir, float dt)
{
    glm::vec3 moveDir(0);
    switch (dir)
    {
        case FORWARD:
            moveDir = Front;
        break;
        case BACKWARD:
            moveDir = -Front;
        break;
        case LEFT:
            moveDir = -Right;
        break;
        case RIGHT:
            moveDir = Right;
        break;
        case UP:
            moveDir = WorldUp;
        break;
        case DOWN:
            moveDir = -WorldUp;
        break;
    }
    // moveDir.y = 0;
    moveDir = glm::normalize(moveDir) * MovementSpeed * dt;
    Position += moveDir;
}

void camera::processMouseMovement(float xOff, float yOff, bool c)
{
    xOff *= MouseSensitivity;
    yOff *= MouseSensitivity;
    Yaw += xOff;
    Pitch += yOff;
    if (c)
    {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }
    updateCameraVectors();
}

void camera::updateCameraVectors()
{
    glm::vec3 f;
    f.x   = static_cast<float>(cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)));
    f.y   = static_cast<float>(sin(glm::radians(Pitch)));
    f.z   = static_cast<float>(sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)));
    Front = glm::normalize(f);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}

void camera::update(float dt)
{
    if (glfwGetKey(opengl::system::getWindow()->getInstance(),GLFW_KEY_W) == GLFW_PRESS) processKeyboard(FORWARD, dt);
    if (glfwGetKey(opengl::system::getWindow()->getInstance(),GLFW_KEY_S) == GLFW_PRESS) processKeyboard(BACKWARD, dt);
    if (glfwGetKey(opengl::system::getWindow()->getInstance(),GLFW_KEY_A) == GLFW_PRESS) processKeyboard(LEFT, dt);
    if (glfwGetKey(opengl::system::getWindow()->getInstance(),GLFW_KEY_D) == GLFW_PRESS) processKeyboard(RIGHT, dt);
    if (glfwGetKey(opengl::system::getWindow()->getInstance(),GLFW_KEY_Q) == GLFW_PRESS) processKeyboard(UP, dt);
    if (glfwGetKey(opengl::system::getWindow()->getInstance(),GLFW_KEY_E) == GLFW_PRESS) processKeyboard(DOWN, dt);
}

void camera::processMouseScroll(float yOff)
{
    Zoom -= yOff;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 45.0f) Zoom = 45.0f;
}