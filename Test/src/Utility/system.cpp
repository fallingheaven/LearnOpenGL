#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <system.h>
#include <window.h>

namespace opengl
{
    window* system::window = nullptr;
    camera* system::camera = nullptr;
    float system::lastFrameTime = -1;

    double system::lastFPSTime = 0.0;
    int system::frameCount = 0;
    float system::fps = 0.0f;

    bool system::drawSkyBox = true;
    bool system::useBlinnPhong = true;
    bool system::useHDR = false;
    bool system::deferredLighting = true;
    bool system::useSSAO = true;
    bool system::useIBL = true;
    bool system::useSpecularIBL = true;

    void system::init()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        camera = new class camera();
        system::setCamera(camera);
    }

    bool system::createWindow(const char* windowName, int width, int height, GLFWmonitor* monitor, GLFWwindow* share)
    {
        if (window == nullptr)
        {
            window = new opengl::window();
        }

        GLFWwindow* tmp = glfwCreateWindow(width, height, windowName, monitor, share);
        if (tmp == nullptr)
        {
            std::cerr << "创建窗口失败" << std::endl;
            glfwTerminate();

            throw;
            return false;
        }

        window->setInstance(tmp);
        window->init(width, height);


        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cerr << "GLAD 加载失败" << std::endl;
            glfwDestroyWindow(window->getInstance());
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window->getInstance());
        // 设置回调函数
        glfwSetCursorPosCallback(window->getInstance(), window::mouse_callback);
        glfwSetScrollCallback(window->getInstance(), window::scroll_callback);
        glfwSetWindowSizeCallback(window->getInstance(), window::window_size_callback);

        // 初始化 ImGui
        {
            // 初始化ImGui上下文
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui::StyleColorsDark();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            // 为 GLFW 和 OpenGL 设置 ImGui 后端
            ImGui_ImplGlfw_InitForOpenGL(window->getInstance(), true);
            ImGui_ImplOpenGL3_Init("#version 440");
        }

        // 设置相机的投影矩阵
        glm::mat4 projection =
                glm::perspective(glm::radians(45.0f), (float)opengl::window::getWidth() / (float)opengl::window::getHeight(), camera->getNearPlane(), camera->getFarPlane());
        camera->setProspectiveMatrix(projection);
        // 设置天空盒
        {
            camera->setSkyboxPath("Assets/Materials/skybox/"); // 设置默认天空盒路径
            Shader *skyboxShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Advanced/Skybox/skybox.vert"),
                                            FileSystem::getPath("src/LearnOpenGL/Advanced/Skybox/skybox.frag"));
            camera->setSkyboxShader(skyboxShader); // 设置天空盒着色器
            camera->loadSkyBox(); // 加载天空盒纹理
            camera->prepareSkybox(); // 准备天空盒
        }
        // 设置全屏四边形
        {
            Shader *fullScreenShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Advanced/screen.vert"),
                                                FileSystem::getPath("src/LearnOpenGL/Advanced/screen.frag"));
            camera->setFullScreenShader(fullScreenShader); // 设置全屏四边形着色器
            camera->prepareFullScreen(); // 准备全屏四边形
            camera->genScreenFrameBuffer(); // 创建帧缓冲，包括颜色、深度、模板缓冲

            camera->genDisplayFrameBuffer(); // 创建显示用帧缓冲

            Shader* blurShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/GaussianBlur/gaussian_blur.vert"),
                                      FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/GaussianBlur/gaussian_blur.frag"));
            camera->setBlurShader(blurShader); // 设置高斯模糊着色器
            camera->genPingPongFrameBuffer(); // 创建ping-pong帧缓冲，用于后处理

            Shader* gBufferShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/GBuffer/gbuffer.vert"),
                                        FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/GBuffer/gbuffer.frag"));
            camera->setGBufferShader(gBufferShader); // 设置GBuffer着色器
            camera->genGBufferFrameBuffer(); // 创建GBuffer帧缓冲

            Shader* ssaoShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/SSAO/ssao.vert"),
                                        FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/SSAO/ssao.frag"));
            camera->setSSAOShader(ssaoShader); // 设置SSAO着色器
            camera->genSSAOFBO(); // 创建SSAO帧缓冲
        }
        // 绑定视图投影矩阵的统一接口块缓冲区
        camera->bindUniformBlock(0, sizeof(camera::MatrixBuffer));

        // OpenGL 全局状态设置
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_STENCIL_TEST);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glEnable(GL_CULL_FACE);
            // glCullFace(GL_FRONT);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);

            // 这里我们手动伽马校正
            // glEnable(GL_FRAMEBUFFER_SRGB);

            // 让立方体贴图采样更加平滑
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }

        return true;
    }

    void system::update(const std::function<void()>& func)
    {
        if (glfwGetKey(window->getInstance(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            close();
            return;
        }

        window->preUpdate(); // 清空缓冲区，并处理事件
        camera->update(getDeltaTime()); // 更新相机位置
        camera->updateViewProjectionMatrix(); // 更新相机的视图和投影矩阵
        calculateFPS(); // 计算FPS

        camera->bindScreenFrameBuffer(); // 绑定相机的帧缓冲进行渲染

        func();

        if (drawSkyBox)
            camera->drawSkybox();
        // camera->drawSkybox();
        camera->drawPingPongFrameBuffer(); // 绘制高斯模糊后的图像到相机帧缓冲
        camera->drawFullScreen(true); // 绘制全屏四边形，将相机帧缓冲内容显示到屏幕上


        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 解绑帧缓冲，回到默认帧缓冲
        // 渲染 ImGui 界面
        {
            // 开始新的 ImGui 帧
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 创建主停靠空间
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            ImGui::Begin("DockSpace Demo", nullptr, window_flags);
            ImGui::PopStyleVar(2);

            // 创建停靠空间
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
            {
                // 创建一个简单的 ImGui 窗口来显示信息
                ImGui::Begin("Info");
                ImGui::Text("FPS: %.1f", fps);
                ImGui::Checkbox("Draw Skybox", &drawSkyBox);
                ImGui::Checkbox("Use BlinnPhong", &useBlinnPhong);
                ImGui::Checkbox("Use HDR", &useHDR);
                ImGui::Checkbox("Deferred Lighting", &deferredLighting);
                if (deferredLighting)
                    ImGui::Checkbox("Use SSAO", &useSSAO);
                ImGui::Checkbox("Use IBL", &useIBL);
                ImGui::Checkbox("Use Specular IBL", &useSpecularIBL);
                ImGui::End();

                // 创建分屏显示窗口
                ImGui::Begin("Render View");

                // 获取窗口内容区域大小
                ImVec2 contentRegion = ImGui::GetContentRegionAvail();

                // 计算图像显示大小，保持宽高比
                float aspectRatio = (float)window::getWidth() / (float)window::getHeight();
                ImVec2 imageSize;
                if (contentRegion.x / contentRegion.y > aspectRatio) {
                    imageSize.y = contentRegion.y;
                    imageSize.x = imageSize.y * aspectRatio;
                } else {
                    imageSize.x = contentRegion.x;
                    imageSize.y = imageSize.x / aspectRatio;
                }

                // 显示渲染结果纹理
                // 注意：ImGui::Image需要将纹理ID转换为void*，并且Y轴需要翻转
                ImGui::Image(
                    camera->getDisplayColorBufferTex(),
                    imageSize,
                    ImVec2(0, 1), // UV坐标起点 (左下角)
                    ImVec2(1, 0)  // UV坐标终点 (右上角) - 这样可以翻转Y轴
                );

                ImGui::End();
            }

            // 结束主停靠空间窗口
            ImGui::End();
            
            // 渲染 ImGui 绘制数据
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        window->postUpdate(); // 交换缓冲区
    }

    window *system::getWindow()
    {
        return window;
    }

    void system::setWindow(class window* target)
    {
        window = target;
    }

    camera* system::getCamera()
    {
        return camera;
    }

    void system::setCamera(class camera *camera)
    {
        system::camera = camera;
    }


    void system::close()
    {
        window->close();
    }

    void system::clear()
    {
        // 在程序结束时清理 ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window->getInstance());
        glfwTerminate();
    }

    float system::getDeltaTime()
    {
        if (lastFrameTime < 0)
        {
            lastFrameTime = static_cast<float>(glfwGetTime());
            return 0;
        }

        auto currentTime = static_cast<float>(glfwGetTime());
        float deltaTime  = currentTime - lastFrameTime;
        lastFrameTime    = currentTime;

        return deltaTime;
    }

    bool system::systemShouldEnd()
    {
        return glfwWindowShouldClose(window->getInstance());
    }

    void system::calculateFPS()
    {
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - lastFPSTime >= 1.0)
        {
            fps = static_cast<float>(frameCount);
            frameCount = 0;
            lastFPSTime = currentTime;
        }
    }


}
