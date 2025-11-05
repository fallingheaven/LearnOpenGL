#include <game.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <game.h>
#include <window.h>
#include <GL/gl.h>

namespace opengl
{
    Game* Game::instance = nullptr;

    void Game::init()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

        camera = new class camera();

        scenes = {
            {"default", new Scene()},
            {"particles", new Scene()},
            {"props", new Scene()}
        };

        levels = {};

        blockTex = nullptr;
        solidBlockTex = nullptr;
        backgroundTex = nullptr;
        ballTex = nullptr;
    }

    bool Game::createWindow(const char* windowName, int width, int height, GLFWmonitor* monitor, GLFWwindow* share)
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
        setWindow(window);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cerr << "GLAD 加载失败" << std::endl;
            glfwDestroyWindow(window->getInstance());
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window->getInstance());
        // 设置回调函数
        // glfwSetCursorPosCallback(window->getInstance(), window::mouse_callback);
        // glfwSetScrollCallback(window->getInstance(), window::scroll_callback);
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
        // glm::mat4 projection =
        //         glm::perspective(glm::radians(45.0f), (float)opengl::window::getWidth() / (float)opengl::window::getHeight(), camera->getNearPlane(), camera->getFarPlane());
        // camera->setProspectiveMatrix(projection);
        // 设置全屏四边形
        {
            Shader *fullScreenShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Advanced/screen.vert"),
                                                FileSystem::getPath("src/LearnOpenGL/Advanced/screen.frag"));
            camera->setFullScreenShader(fullScreenShader); // 设置全屏四边形着色器
            camera->prepareFullScreen(); // 准备全屏四边形
            camera->genScreenFrameBuffer(); // 创建帧缓冲，包括颜色、深度、模板缓冲

            camera->genPostprocessingFrameBuffer(); // 创建后处理用帧缓冲

            camera->genDisplayFrameBuffer(); // 创建显示用帧缓冲
        }
        // 准备sprite的quad顶点数据
        camera->prepareQuadVAO();
        Shader *spriteShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Practice/Shaders/sprite.vert"),
                                   FileSystem::getPath("src/LearnOpenGL/Practice/Shaders/sprite.frag"));
        Shader *particleShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Practice/Shaders/Particle/particle.vert"),
                                      FileSystem::getPath("src/LearnOpenGL/Practice/Shaders/Particle/particle.frag"));
        postprocessingShader = new Shader (FileSystem::getPath("src/LearnOpenGL/Practice/Shaders/Postprocess/postprocess.vert"),
                                          FileSystem::getPath("src/LearnOpenGL/Practice/Shaders/Postprocess/postprocess.frag"));
        postprocessingShader->use();
        float xOff = 1.0f / static_cast<float>(getWindow()->getWidth());
        float yOff = 1.0f / static_cast<float>(getWindow()->getHeight());
        std::vector<glm::vec2> offsets = {
            { -xOff,  yOff  },  // top-left
            {0.0f,  yOff  },  // top-center
            {  xOff,  yOff  },  // top-right
            { -xOff, 0.0f  },  // center-left
            {0.0f, 0.0f  },  // center-center
            {  xOff, 0.0f  },  // center - right
            { -xOff, -yOff  },  // bottom-left
            {0.0f, -yOff  },  // bottom-center
            {  xOff, -yOff  }   // bottom-right
        };
        postprocessingShader->setVec2("offsets", offsets[0], 9);
        GLint edge_kernel[9] = {
            -1, -1, -1,
            -1,  8, -1,
            -1, -1, -1
        };
        postprocessingShader->setInt("edge_kernel", edge_kernel, 9);
        GLfloat blur_kernel[9] = {
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16
        };
        postprocessingShader->setFloat("blur_kernel", blur_kernel, 9);
        postprocessingShader->setInt("scene", 0);

        camera->setSpriteShader(spriteShader);

        // // 绑定视图投影矩阵的统一接口块缓冲区
        // camera->bindUniformBlock(0, sizeof(camera::MatrixBuffer));

        // 加载纹理和关卡
        {
            blockTex = new Texture();       blockTex->init(FileSystem::getPath("Assets/Materials/block.png"), GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
            solidBlockTex = new Texture();  solidBlockTex->init(FileSystem::getPath("Assets/Materials/block_solid.png"), GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
            backgroundTex = new Texture();  backgroundTex->init(FileSystem::getPath("Assets/Materials/background.jpg"), GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
            ballTex = new Texture();        ballTex->init(FileSystem::getPath("Assets/Materials/awesomeface.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
            particleTex = new Texture();    particleTex->init(FileSystem::getPath("Assets/Materials/particle.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);

            GLuint levelWidth = getWindow()->getWidth();
            GLuint levelHeight = getWindow()->getHeight() * 0.5;
            auto one = new GameLevel();
            one->load(FileSystem::getPath("Assets/Levels/level1.txt"), levelWidth, levelHeight);
            auto two = new GameLevel();     two->load(FileSystem::getPath("Assets/Levels/level2.txt"), levelWidth, levelHeight);
            auto three = new GameLevel();   three->load(FileSystem::getPath("Assets/Levels/level3.txt"), levelWidth, levelHeight);
            auto four = new GameLevel();    four->load(FileSystem::getPath("Assets/Levels/level4.txt"), levelWidth, levelHeight);

            this->levels.push_back(one);
            this->levels.push_back(two);
            this->levels.push_back(three);
            this->levels.push_back(four);
            this->currentLevelIndex = 0;

            // 初始化玩家挡板和球
            auto playerTex = new Texture(); playerTex->init(FileSystem::getPath("Assets/Materials/paddle.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
            player = new Object();
            player->setScale(glm::vec3(PLAYER_SIZE, 1));
            player->setPosition(glm::vec3(getWindow()->getWidth() / 2 - player->getScale().x / 2, getWindow()->getHeight() - player->getScale().y - 10.0f, 0.0f));
            auto playerRenderer = new SpriteRenderer(spriteShader, playerTex);
            player->setRenderer(playerRenderer);

            ball = new BallObject();
            ball->setScale(glm::vec3(BALL_RADIUS * 2, BALL_RADIUS * 2, 1));
            ball->setPosition(glm::vec3(getWindow()->getWidth() / 2 - BALL_RADIUS, player->getPosition().y - BALL_RADIUS * 2, 0.0f));
            ball->velocity = INITIAL_BALL_VELOCITY;
            ball->radius = BALL_RADIUS;
            auto ballRenderer = new SpriteRenderer(spriteShader, ballTex);
            ball->setRenderer(ballRenderer);

            // 预创建粒子对象池
            for (int i = 0; i < 500; i++)
            {
                auto p = new ParticleObject();
                auto particleRenderer = new ParticleRenderer(particleShader, particleTex);
                p->setRenderer(particleRenderer);
                particles.push_back(p);
            }

            // 创建道具纹理
            for (PropsType type = None; type <= CHAOS; type = static_cast<PropsType>(static_cast<int>(type) + 1))
            {
                activeBuffs[type] = 0.0f;
                propsTextures[type] = new Texture();
                switch (type)
                {
                    case None:
                        break;
                    case SPEED_UP:
                        propsTextures[type]->init(FileSystem::getPath("Assets/Materials/powerup_speed.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
                        break;
                    case STICKY:
                        propsTextures[type]->init(FileSystem::getPath("Assets/Materials/powerup_sticky.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
                        break;
                    case PASS_THROUGH:
                        propsTextures[type]->init(FileSystem::getPath("Assets/Materials/powerup_passthrough.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
                        break;
                    case PAD_SIZE_INCREASE:
                        propsTextures[type]->init(FileSystem::getPath("Assets/Materials/powerup_increase.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
                        break;
                    case CONFUSE:
                        propsTextures[type]->init(FileSystem::getPath("Assets/Materials/powerup_confuse.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
                        break;
                    case CHAOS:
                        propsTextures[type]->init(FileSystem::getPath("Assets/Materials/powerup_chaos.png"), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
                        break;
                }
            }
        }

        // OpenGL 全局状态设置
        {
            // glEnable(GL_DEPTH_TEST);
            glDisable(GL_DEPTH_TEST);
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

    void Game::update(const std::function<void()>& func)
    {
        if (State != GAME_ACTIVE)
            return;

        if (glfwGetKey(window->getInstance(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            close();
            return;
        }

        window->preUpdate(); // 清空缓冲区，并处理事件

        if (ball->getPosition().y > getWindow()->getHeight())
        {
            loadLevel();
            return;
        }

        processCollisions(); // 处理碰撞检测

        float dt = getDeltaTime();
        processInput(dt); // 处理输入
        if (!ball->active)
            ball->transform.position.x = player->getPosition().x + player->getScale().x / 2 - ball->getScale().x / 2;
        else
            ball->move(dt); // 移动小球
        updateParticles(dt);
        updateProps(dt);
        updateActiveBuffs(dt);

        if (shakeTime > 0.0f)
        {
            shakeTime -= dt;
            postprocessingShader->use();
            postprocessingShader->setFloat("time", glfwGetTime());
            if (shakeTime <= 0.0f)
            {
                postprocessingShader->setBool("shake", false);
            }
        }
        if (activeBuffs[PropsType::CHAOS] > 0.0f)
        {
            postprocessingShader->use();
            postprocessingShader->setFloat("time", glfwGetTime());
        }

        // camera->update(getDeltaTime()); // 更新相机位置
        // camera->updateViewProjectionMatrix(); // 更新相机的视图和投影矩阵
        calculateFPS(); // 计算FPS

        camera->bindScreenFrameBuffer(); // 绑定相机的帧缓冲进行渲染
        camera->drawFullScreen(backgroundTex->ID); // 绘制背景图像到相机帧缓冲

        glDisable(GL_CULL_FACE);

        drawScene("particles");
        drawScene("default");
        drawScene("props");
        glEnable(GL_CULL_FACE);

        func();

        camera->drawPostprocessing(postprocessingShader); // 使用后处理着色器绘制相机帧缓冲内容
        // camera->drawPingPongFrameBuffer(); // 绘制高斯模糊后的图像到相机帧缓冲
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

    window *Game::getWindow()
    {
        return window;
    }

    void Game::setWindow(class window* target)
    {
        this->window = target;
    }

    camera* Game::getCamera()
    {
        return camera;
    }

    void Game::setCamera(class camera *camera)
    {
        this->camera = camera;
    }

    Scene* Game::getScene(const std::string& name)
    {
        if (scenes.find(name) == scenes.end())
        {
            scenes[name] = new Scene();
        }
        return scenes[name];
    }

    void Game::close()
    {
        window->close();
    }

    void Game::clear()
    {
        // 在程序结束时清理 ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window->getInstance());
        glfwTerminate();
    }

    float Game::getDeltaTime()
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

    bool Game::systemShouldEnd()
    {
        return glfwWindowShouldClose(window->getInstance());
    }

    void Game::calculateFPS()
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

    void Game::loadLevel(GameLevel *level)
    {
        Scene* scene = getScene("default");
        scene->clearObjects();
        for (auto& object : level->objects)
        {
            scene->addObject(object);
        }
        resetPlayer();
        scene->addObject(player);
        scene->addObject(ball);
    }

    void Game::loadLevel(int level)
    {
        Scene* scene = getScene("default");
        scene->clearObjects();
        if (level < 0 || level >= static_cast<int>(levels.size())) return;

        for (auto& object : levels[level]->objects)
        {
            scene->addObject(object);
        }
        resetPlayer();
        scene->addObject(player);
        scene->addObject(ball);
    }

    void Game::loadLevel()
    {
        Scene* scene = getScene("default");
        int level = currentLevelIndex;
        scene->clearObjects();
        if (level < 0 || level >= static_cast<int>(levels.size())) return;

        std::cout << "Loading level " << level << " with " << levels[level]->objects.size() << " objects." << std::endl;
        for (auto& object : levels[level]->objects)
        {
            scene->addObject(object);
        }
        resetPlayer();
        scene->addObject(player);
        scene->addObject(ball);
    }

    void Game::processInput(GLfloat dt)
    {
        if (this->State == GAME_ACTIVE)
        {
            GLfloat velocity = PLAYER_VELOCITY * dt;
            // 移动挡板
            if (glfwGetKey(getWindow()->getInstance(),GLFW_KEY_A) == GLFW_PRESS)
            {
                if (player->transform.position.x >= 0)
                    player->transform.position.x -= velocity;
            }
            if (glfwGetKey(getWindow()->getInstance(),GLFW_KEY_D) == GLFW_PRESS)
            {
                if (player->transform.position.x <= getWindow()->getWidth() - player->transform.scale.x)
                    player->transform.position.x += velocity;
            }
            if (glfwGetKey(getWindow()->getInstance(),GLFW_KEY_SPACE) == GLFW_PRESS)
            {
                ball->active = true;
            }
        }
    }

    void Game::processCollisions()
    {
        std::vector<Object*> toDestroy; // 创建一个列表来存储待销毁的对象

        for (auto& object : getScene("default")->getObjects())
        {
            // 处理碰撞检测和响应
            if (object == ball)
                continue;

            if (object == player)
            {
                auto collision = Utility::checkCollisionAABB(ball->getPosition(), ball->radius,
                                                            player->getPosition(), player->getScale());
                if (collision.isCollided)
                {
                    // 计算碰撞点的相对位置
                    GLfloat centerBoard = player->getPosition().x + player->getScale().x / 2;
                    GLfloat distance = (ball->getPosition().x + ball->radius) - centerBoard;
                    GLfloat percentage = distance / (player->getScale().x / 2);
                    // 根据碰撞点调整小球的速度
                    GLfloat strength = 2.0f;
                    glm::vec2 oldVelocity = ball->velocity;
                    ball->velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
                    ball->velocity.y = -1 * abs(ball->velocity.y);
                    ball->velocity = glm::normalize(ball->velocity) * glm::length(oldVelocity);
                }
                continue;
            }

            auto collision = Utility::checkCollisionAABB(ball->getPosition(), ball->radius,
                                                        object->getPosition(), object->getScale());
            if (collision.isCollided)
            {
                if (!passThrough)
                {
                    if (collision.collisionDir & (Utility::UP | Utility::DOWN))
                    {
                        ball->velocity.y = -ball->velocity.y; // 反转Y轴速度
                        // 根据碰撞方向调整位置，防止粘连
                        GLfloat penetration = ball->radius - abs(collision.difference.y);
                        if (collision.collisionDir == Utility::UP)
                            ball->transform.position.y += penetration;
                        else
                            ball->transform.position.y -= penetration;
                    }
                    else
                    {
                        ball->velocity.x = -ball->velocity.x; // 反转X轴速度
                        // 根据碰撞方向调整位置，防止粘连
                        GLfloat penetration = ball->radius - abs(collision.difference.x);
                        if (collision.collisionDir == Utility::RIGHT)
                            ball->transform.position.x += penetration;
                        else
                            ball->transform.position.x -= penetration;
                    }
                }


                if (!object->isSolid)
                {
                    postprocessingShader->use();
                    postprocessingShader->setBool("shake", true);
                    shakeTime = 0.05f;

                    // 不直接销毁，而是添加到待销毁列表
                    toDestroy.push_back(object);
                    // object->Destroy();
                    // delete object; // 这里delete的话，后面重新加载的时候也不会显示了

                    if (rand() % 5 == 0) // 20%的概率生成道具
                    {
                        auto pType = static_cast<PropsType>((rand() % (CHAOS)) + 1);
                        auto props = new PropsObject();
                        props->type = pType;
                        props->setScale(glm::vec3(PROP_SIZE, 1.0f));
                        props->setPosition(object->getPosition() + glm::vec3(object->getScale().x / 2 - props->getScale().x / 2,
                                                                             object->getScale().y / 2 - props->getScale().y / 2,
                                                                             0.0f));
                        props->velocity = PROP_VELOCITY;
                        auto propsRenderer = new SpriteRenderer(camera->getSpriteShader(), propsTextures[pType]);
                        props->setRenderer(propsRenderer);
                        getScene("props")->addObject(props);
                    }
                }

            }
        }

        for (auto& object : getScene("props")->getObjects())
        {
            auto collision = Utility::checkCollisionAABB(player->getPosition(), player->getScale(),
                                                        object->getPosition(), object->getScale());
            if (collision.isCollided)
            {
                if (object->isDestroyed) continue;

                auto propsObj = dynamic_cast<PropsObject*>(object);
                if (propsObj)
                {
                    std::cout << "collide props: " << propsObj->type << std::endl;
                    // 处理道具碰撞效果
                    switch (propsObj->type)
                    {
                        case None:
                            std::cerr << "Error: PropsType is None." << std::endl;
                        break;
                        case SPEED_UP:
                            ball->velocity *= 1.2f; // 提升20%速度
                        activeBuffs[SPEED_UP] = 5.0f; // 持续5秒
                        break;
                        case STICKY:
                            ball->setPosition(glm::vec3(player->getPosition().x + player->getScale().x / 2 - ball->getScale().x / 2,
                                                        player->getPosition().y - ball->getScale().y,
                                                        0.0f));
                            ball->active = false;
                            activeBuffs[STICKY] = 999.9f; // 永久
                        break;
                        case PASS_THROUGH:
                            passThrough = true;
                            activeBuffs[PASS_THROUGH] = 5.0f; // 持续5秒
                        break;
                        case PAD_SIZE_INCREASE:
                            player->setScale(glm::vec3(player->getScale().x * 1.5f, player->getScale().y, player->getScale().z));
                            activeBuffs[PAD_SIZE_INCREASE] = 10.0f; // 持续10秒
                        break;
                        case CONFUSE:
                            confuse = true;
                            postprocessingShader->use();
                            postprocessingShader->setBool("confuse", true);
                            activeBuffs[CONFUSE] = 3.0f; // 持续5秒
                        break;
                        case CHAOS:
                            chaos = true;
                            postprocessingShader->use();
                            postprocessingShader->setBool("chaos", true);
                            activeBuffs[CHAOS] = 3.0f; // 持续5秒
                        break;
                    }

                    toDestroy.push_back(object);
                }
            }
        }

        // 现在统一销毁对象
        for (auto& obj : toDestroy)
        {
            obj->Destroy();
        }
    }

    ParticleObject *Game::getFirstUnusedParticle()
    {
        for (auto &p : particles)
        {
            if (p->life <= 0.0f)
            {
                return p;
            }
        }
        return nullptr;
    }


    void Game::updateParticles(float dt)
    {
        particleSpawnTimer += dt;
        if (particleSpawnTimer > spawnInterval)
        {
            particleSpawnTimer = 0.0f;
            for (int i = 0; i < 2; ++i)
            {
                ParticleObject* particle = getFirstUnusedParticle();
                if (particle)
                {
                    particle->isDestroyed = false;
                    particle->life = 1.0f;
                    float random = ((rand() % 100) - 50) / 10.0f;
                    float rColor = 0.5f + ((rand() % 100) / 100.0f);
                    particle->transform.position =
                        ball->getPosition() + glm::vec3(random, random, 0.0f) + glm::vec3(ball->radius, ball->radius, 0.0f);
                    particle->velocity = ball->velocity * 0.05f;
                    particle->color = glm::vec4(rColor, rColor, rColor, 1.0f);
                    getScene("particles")->addObject(particle);
                }
            }
        }

        for (auto &particle : particles)
        {
            particle->life -= dt;
            if (particle->life > 0.0f)
            {
                particle->transform.position += glm::vec3(particle->velocity * dt, 0.0f);
                particle->color.a = std::max(particle->color.a - dt * 2.5f, 0.0f);
            }
            else
            {
                // particle->Destroy();
                getScene("particles")->removeObject(particle);
            }
        }
    }

    void Game::updateProps(float dt)
    {
        std::vector<Object*> toDestroy; // 创建一个列表来存储待销毁的对象

        for (auto &propsObj : scenes["props"]->getObjects())
        {
            auto props = dynamic_cast<PropsObject*>(propsObj);
            if (!props) continue;

            props->transform.position += glm::vec3(props->velocity * dt, 0.0f);
            if (props->getPosition().y >= getWindow()->getHeight())
            {
                toDestroy.push_back(props);
            }
        }

        for (auto& obj : toDestroy)
        {
            obj->Destroy();
        }
    }

    void Game::updateActiveBuffs(float dt)
    {
        for (auto& buff : activeBuffs)
        {
            if (buff.second <= 0.0f) continue;

            buff.second -= dt;
            if (buff.second <= 0.0f)
            {
                // Buff过期，执行相应的清除操作
                switch (buff.first)
                {
                    case None:
                        break;
                    case SPEED_UP:
                        ball->velocity /= 1.2f; // 恢复原始速度
                        break;
                    case STICKY:
                        std::cerr << "Sticky buff expired." << std::endl;
                        break;
                    case PASS_THROUGH:
                        passThrough = false;
                        break;
                    case PAD_SIZE_INCREASE:
                        player->setScale(glm::vec3(PLAYER_SIZE, 1.0f));
                        break;
                    case CONFUSE:
                        confuse = false;
                        postprocessingShader->use();
                        postprocessingShader->setBool("confuse", false);
                        break;
                    case CHAOS:
                        chaos = false;
                        postprocessingShader->use();
                        postprocessingShader->setBool("chaos", false);
                        break;
                }
            }
        }
    }



    void Game::drawScene(const char* sceneName)
    {
        for (auto& object : scenes[sceneName]->getObjects())
        {
            if (object->isDestroyed) continue;
            if (object->getRenderer())
                object->getRenderer()->Draw();
        }
    }


    void Game::resetPlayer()
    {
        player->setPosition(glm::vec3(getWindow()->getWidth() / 2 - player->getScale().x / 2, getWindow()->getHeight() - player->getScale().y - 10.0f, 0.0f));
        ball->reset(glm::vec2(getWindow()->getWidth() / 2 - BALL_RADIUS, player->getPosition().y - BALL_RADIUS * 2), INITIAL_BALL_VELOCITY);
    }



    void GameLevel::load(std::string levelFile, GLuint levelWidth, GLuint levelHeight)
    {
        this->objects.clear();
        this->levelWidth = levelWidth;
        this->levelHeight = levelHeight;
        // 从文件中加载
        GLuint tileCode;
        GameLevel level;
        std::string line;
        std::ifstream fstream(levelFile);
        std::vector<std::vector<GLuint>> tileData;
        if (fstream)
        {
            while (std::getline(fstream, line)) // 读取关卡文件的每一行
            {
                std::istringstream sstream(line);
                std::vector<GLuint> row;
                while (sstream >> tileCode) // 读取被空格分隔的每个数字
                    row.push_back(tileCode);
                tileData.push_back(row);
            }
            if (!tileData.empty())
                this->init(tileData);
        }
    }

    void GameLevel::init(std::vector<std::vector<GLuint> > tileData)
    {
        GLuint height = tileData.size();
        GLuint width = tileData[0].size();
        GLfloat unit_width = (float)levelWidth / static_cast<GLfloat>(width);
        GLfloat unit_height = (float)levelHeight / height;
        Shader* spriteShader = Game::getInstance()->getCamera()->getSpriteShader();
        // 基于tileDataC初始化关卡
        std::cout << std::endl;
        for (GLuint y = 0; y < height; ++y)
        {
            std::cout << std::endl;
            for (GLuint x = 0; x < width; ++x)
            {
                std::cout << tileData[y][x] << " ";

                // 检查砖块类型
                if (tileData[y][x] == 1)
                {
                    glm::vec2 pos(unit_width * x, unit_height * y);
                    glm::vec2 size(unit_width, unit_height);

                    auto solidBlockTex = Game::getInstance()->solidBlockTex;
                    Transform transform = Transform(glm::vec3(pos, 0.0f), glm::vec3(0.0f), glm::vec3(size, 1.0f));
                    Object* obj = new Object(transform, glm::vec4(0.8f, 0.8f, 0.7f, 1.0f));
                    obj->setRenderer(new SpriteRenderer(spriteShader, solidBlockTex));
                    obj->isSolid = GL_TRUE;

                    this->objects.push_back(obj);
                }
                else if (tileData[y][x] > 1)
                {
                    glm::vec4 color = glm::vec4(1.0f); // 默认为白色
                    if (tileData[y][x] == 2)
                        color = glm::vec4(0.2f, 0.6f, 1.0f, 1.0f);
                    else if (tileData[y][x] == 3)
                        color = glm::vec4(0.0f, 0.7f, 0.0f, 1.0f);
                    else if (tileData[y][x] == 4)
                        color = glm::vec4(0.8f, 0.8f, 0.4f, 1.0f);
                    else if (tileData[y][x] == 5)
                        color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);

                    glm::vec2 pos(unit_width * x, unit_height * y);
                    glm::vec2 size(unit_width, unit_height);

                    auto blockTex = Game::getInstance()->blockTex;
                    Transform transform = Transform(glm::vec3(pos, 0.0f), glm::vec3(0.0f), glm::vec3(size, 1.0f));
                    Object* obj = new Object(transform, color);
                    obj->setRenderer(new SpriteRenderer(spriteShader, blockTex));

                    this->objects.push_back(obj);
                }
            }
        }
    }

}
