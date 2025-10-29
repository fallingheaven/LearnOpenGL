#include <system.h>
#include <window.h>

namespace opengl
{
    class camera;

    glm::vec2 window::lastMousePos = {400, 400};
    GLFWwindow* window::instance = nullptr;
    int window::width = 800;
    int window::height = 800;

    GLFWwindow *window::getInstance()
    {
        return instance;
    }

    void window::setInstance(GLFWwindow *window)
    {
        instance = window;
    }


    void window::init(int width, int height)
    {
        window::width = width;
        window::height = height;
        setLastMousePos(width/2, height/2);
        glfwMakeContextCurrent(getInstance());
    }

    void window::preUpdate()
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void window::postUpdate()
    {
        glfwSwapBuffers(getInstance());
    }

    void window::close()
    {
        glfwSetWindowShouldClose(getInstance(), GLFW_TRUE);
    }


    glm::vec2 window::getLastMousePos()
    {
        return lastMousePos;
    }

    void window::setLastMousePos(glm::vec2 pos)
    {
        lastMousePos = pos;
    }

    void window::setLastMousePos(float x, float y)
    {
        lastMousePos = {x, y};
    }

    int window::getWidth()
    {
        return window::width;
    }

    int window::getHeight()
    {
        return window::height;
    }


    void window::mouse_callback(GLFWwindow *w, double xPos, double yPos)
    {
        if (glfwGetKey(w, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        {
            // 如果按下，则不处理鼠标移动
            return;
        }

        double dx = xPos - lastMousePos.x, dy = lastMousePos.y - yPos;
        setLastMousePos(xPos, yPos);
        // lastMousePos = {xPos, yPos};
        system::getCamera()->processMouseMovement(static_cast<float>(dx), static_cast<float>(dy));
    }

    void window::scroll_callback(GLFWwindow *, double, double yOffset)
    {
        system::getCamera()->processMouseScroll(static_cast<float>(yOffset));
    }

    void window::window_size_callback(GLFWwindow* window, int width, int height)
    {
        window::width = width;
        window::height = height;
        // 设置OpenGL的视口以匹配新的窗口尺寸
        glViewport(0, 0, width, height);

        glm::mat4 projection =
            glm::perspective(glm::radians(45.0f), (float)getWidth() / (float)getHeight(), 0.1f, 100.0f);
        system::getCamera()->setProspectiveMatrix(projection);
    }

}