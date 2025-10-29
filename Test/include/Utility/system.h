#pragma once
#include <camera.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>

namespace opengl
{
    class window;
    class camera;

    // 当前system=window+camera+opengl控制
    class system
    {
    public:
        // 生命周期函数
        static void init();
        static void update(const std::function<void()>& renderFunc);
        static void close();
        static void clear();

        // 场景系统对象
        // TODO 日后可能集成到Scene中
        static bool createWindow(const char* windowName, int width = 800, int height = 800, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
        static window* getWindow();
        static camera* getCamera();

        // 辅助函数
        static float getDeltaTime();
        static bool systemShouldEnd();

        static bool useBlinnPhong;
        static bool useHDR;
        static bool deferredLighting;
        static bool useSSAO;
        static bool useIBL;
        static bool useSpecularIBL;

    private:
        static window* window;
        static camera* camera;

        static double lastFPSTime;
        static int frameCount;
        static float fps;

        static bool drawSkyBox;

        static void calculateFPS();

        void static setWindow(class window* target);
        void static setCamera(class camera* camera);

        static float lastFrameTime;
    };


}