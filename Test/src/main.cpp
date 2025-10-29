#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.hpp>
#include <Camera.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

// 回调和全局
void framebuffer_size_callback(GLFWwindow *w, int w_, int h)
{
    glViewport(0, 0, w_, h);
}

camera camera;
double lastX     = 800.0f / 2, lastY = 600.0f / 2;
bool firstMouse = true;

void mouse_callback(GLFWwindow *w, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX      = xPos;
        lastY      = yPos;
        firstMouse = false;
    }
    double dx = xPos - lastX, dy = lastY - yPos;
    lastX    = xPos;
    lastY    = yPos;
    camera.processMouseMovement(static_cast<float>(dx), static_cast<float>(dy));
}

void scroll_callback(GLFWwindow *, double, double yOffset) { camera.processMouseScroll(static_cast<float>(yOffset)); }

int main()
{
    // 初始化 GLFW + GLAD
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(800, 600, "MyGLDemo", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    // 回调注册
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ImGui 初始化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 加载着色器
    Shader shader("shaders/phong.vert", "shaders/phong.frag");

    // 准备 VAO/VBO: 完整立方体顶点数据
    float vertices[] = {
        // 后面
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

        // 前面
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,

        // 左面
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f,

        // 右面
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

        // 底面
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f,

        // 顶面
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
    };
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,GL_STATIC_DRAW);
    // 位置属性 (location = 0)：
    // 3: 每个顶点的 3 个分量 (x, y, z)；
    // GL_FLOAT: 数据类型为 float
    // GL_FALSE: 不需要归一化
    // 6 * sizeof(float): 每个顶点的步长 (stride)
    // nullptr: 偏移量为 0，表示从数组的起始位置读取
    glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE, 6 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);
    // 法线属性 (location = 1)：
    // 3: 每个法线的 3 个分量 (x, y, z)；
    // GL_FLOAT: 数据类型为 float
    // GL_FALSE: 不需要归一化
    // 6 * sizeof(float): 每个顶点的步长 (stride)
    // reinterpret_cast<void *>(3 * sizeof(float)): 偏移量为 3 个 float，表示从顶点数据的第 4 个 float 开始读取法线
    glVertexAttribPointer(1, 3,GL_FLOAT,GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    bool showImGui  = true;
    float lastFrame = 0.0f;

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    float lightColor[3] = {lightPos.x, lightPos.y, lightPos.z};

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        auto currentFrame = (float) glfwGetTime();
        float dt          = currentFrame - lastFrame;
        lastFrame         = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
        {
            showImGui = !showImGui;
            glfwSetInputMode(window, GLFW_CURSOR, showImGui ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }

        if (!showImGui)
        {
            // 输入
            if (glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard(FORWARD, dt);
            if (glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard(BACKWARD, dt);
            if (glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard(LEFT, dt);
            if (glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard(RIGHT, dt);
        }

        // 清屏
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 渲染
        shader.use();

        // 更新视图和投影矩阵
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", proj);

        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("light.position", lightPos);
        glm::vec3 lightColorVec = glm::vec3(lightColor[0], lightColor[1], lightColor[2]);
        shader.setVec3("light.ambient", lightColorVec);
        shader.setVec3("light.diffuse", glm::vec3(0.5f, 0.5f, 0.5f)); // 漫反射
        shader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f)); // 高光

        // 更新模型矩阵
        auto model = glm::mat4(1.0f);
        shader.setMat4("model", model);

        // 绘制
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        if (showImGui)
        {
            // ImGui 新帧
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // UI 控件
            ImGui::Begin("Settings");
            ImGui::ColorEdit3("Light Color", lightColor);
            lightPos = glm::vec3(lightColor[0], lightColor[1], lightColor[2]);
            ImGui::End();

            // 渲染 UI
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
