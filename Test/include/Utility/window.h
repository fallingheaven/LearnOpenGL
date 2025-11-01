#pragma once
#include <GLFW/glfw3.h>
#include <game.h>

namespace opengl
{
    class window
    {
    public:
        GLFWwindow* getInstance();
        void setInstance(GLFWwindow* window);
        void init(int width, int height);
        void close();
        void preUpdate();
        void postUpdate();

        static glm::vec2 getLastMousePos();
        static void setLastMousePos(glm::vec2);
        static void setLastMousePos(float x, float y);
        static int getWidth();
        static int getHeight();

        static void mouse_callback(GLFWwindow *w, double xPos, double yPos);
        static void scroll_callback(GLFWwindow *, double, double yOffset);
        static void window_size_callback(GLFWwindow* window, int width, int height);

    private:
        static glm::vec2 lastMousePos;
        static GLFWwindow* instance;
        static int width;
        static int height;
    };
}