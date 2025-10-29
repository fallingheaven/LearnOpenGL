#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void frameBuffer_size_callback(GLFWwindow* _window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // 1.初始化
    // GLFW库->版本号->使用核心模式
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2.窗口
    // 创建窗口->将窗口绑定到上下文
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "创建窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 3.加载OpenGL函数指针
    // OpenGL 的函数并不是直接可用的，需要在运行时确定，这里借助glad加载函数地址绑定到程序中
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "初始化GLAD失败" << std::endl;
        return -1;
    }

    // 4.OpenGL的一些窗口配置
    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, frameBuffer_size_callback);

    // 主循环
    while(!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        // glClearColor用来来设置清空屏幕所用的颜色，而非绘制颜色
        glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
        // glClear在清空时绘制了清空缓存的颜色(0.3f, 0.4f, 0.1f, 1.0f)
        glClear(GL_COLOR_BUFFER_BIT);

        // 交换颜色缓冲
        glfwSwapBuffers(window);
        // 检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态，
        // 并调用对应的回调函数（可以通过回调方法手动设置）
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
