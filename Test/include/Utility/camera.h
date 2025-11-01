#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <shader.hpp>
#include <stb_image.h>
#include <system.h>
#include <window.h>
#include <GLFW/glfw3.h>
#include <filesystem.hpp>
#include <functional>
#include <utility.h>
#include <game.h>


enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN};

namespace opengl
{
    class camera
    {
    public:
        glm::vec3 Position{}, Front, Up{}, Right{}, WorldUp{};
        float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;

        GLuint quadVAO{}, quadVBO{};
        void prepareQuadVAO();

        explicit camera(glm::vec3 pos = glm::vec3(0, 0, 3), glm::vec3 up = glm::vec3(0, 1, 0), float yaw = -90,
                        float pitch   = 0);

        void setSkyboxPath(const std::string& path);
        void setSkyboxShader(Shader* shader) { skyboxShader = shader; }
        void loadSkyBox();
        void prepareSkybox();
        void bindSkybox(int index); // 绑定天空盒纹理到指定纹理单元，用于着色器采样
        void drawSkybox();


        void setFullScreenShader(Shader* shader) { fullScreenShader = shader; }
        void prepareFullScreen();
        void genScreenFrameBuffer();
        void bindScreenFrameBuffer();
        void genDisplayFrameBuffer();
        unsigned int getDisplayColorBufferTex() const { return displayColorBufferTex; }
        void drawFullScreen(bool renderToDisplayFBO = false);
        void drawFullScreen(unsigned int tex);// 供外部调用，绘制指定帧缓冲的颜色纹理到相机fbo
        void drawFullScreen(Shader* shader, unsigned int tex = -1); // 使用指定着色器绘制全屏四边形
        void setBlurShader(Shader* shader) { blurShader = shader; }
        void genPingPongFrameBuffer(); // 生成ping-pong帧缓冲，用于高斯模糊
        void drawPingPongFrameBuffer();

        void drawFullScreenTex(unsigned int fbo, Shader* shader);

        void setGBufferShader(Shader* shader) { gBufferShader = shader; }
        // Shader* getGBufferShader() const { return gBufferShader; }
        void genGBufferFrameBuffer();
        void drawGBufferFrameBuffer(const std::function<void(Shader*)>& renderScene);
        unsigned int getGPositionTex() const { return gPosition; }
        unsigned int getGNormalTex() const { return gNormal; }
        unsigned int getGAlbedoSpec() const { return gAlbedoSpec; }
        void copyDepthGBufferToCameraFrameBuffer() const;

        void genSSAOFBO();
        void drawSSAOBuffer();
        void setSSAOShader(Shader* shader) { ssaoShader = shader; }
        unsigned int getSSAOTex() const { return ssaoColorBuffer; }

        void bindUniformBlock(unsigned int bindingPoint, GLsizei blockSize);


        void updateViewProjectionMatrix();
        glm::mat4 getViewMatrix() const;
        void setProspectiveMatrix(glm::mat4);
        glm::mat4 getProspectiveMatrix() const;
        float getNearPlane() const { return near_plane; }
        float getFarPlane() const { return far_plane; }

        void update(float deltaTime);


        void processKeyboard(Camera_Movement, float deltaTime);
        void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
        void processMouseScroll(float yoffset);

        struct MatrixBuffer
        {
            glm::mat4 projection;
            glm::mat4 view;
        };

    private:
        glm::mat4 prospectiveMatrix;
        float near_plane = 0.1f, far_plane = 25.0f;

        std::string skyboxPath;
        unsigned int skyboxVAO{}, skyboxVBO{}, skyboxTex{};
        Shader* skyboxShader;

        unsigned int screenVAO{}, screenVBO{};
        Shader* fullScreenShader;

        // 设置多渲染目标
        unsigned int screenFBO{}, colorBufferTex[5]{}, depthStencilRBO{};
        // 显示用FBO
        unsigned int displayFBO{}, displayColorBufferTex{};

        // 用于bloom的ping-pong帧缓冲，循环高斯模糊
        unsigned int pingPongFBO[2]{}, pingPongColorBuffers[2]{};
        Shader* blurShader;

        unsigned int gBufferFBO{}, gPosition{}, gNormal{}, gAlbedoSpec{}, gDepth{};
        Shader* gBufferShader;

        unsigned int ssaoFBO{}, ssaoColorBuffer{}, ssaoNoiseTex{};
        std::vector<glm::vec3> ssaoKernel{};
        Shader* ssaoShader;

        unsigned int ubo[5]{}; // 预设5个uniform缓冲区对象binding从0到4，其中自由使用后四个，第一个管理投影和视图矩阵

        void updateCameraVectors();
    };
}

