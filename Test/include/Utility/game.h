#pragma once
#include <shader.hpp>
#include <system.h>
#include <window.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <map>
#include <scene.h>

namespace opengl
{
    class Object;
    class Scene;

    class Game
    {
    public:
        ~Game();

        static Game* getInstance() { if (!instance) instance = new Game(); return instance;}

        void init();
        void update(const std::function<void()>& renderFunc);
        void close();
        void clear();

        bool createWindow(const char* windowName, int width = 800, int height = 800, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
        class window* getWindow();
        class camera* getCamera();
        Scene* getScene(const std::string& name = "default");

        float getDeltaTime();
        bool systemShouldEnd();

    private:
        Game() { init(); };
        static Game* instance;
        std::map<std::string, Scene*> scenes;

        class window* window = nullptr;
        class camera* camera = nullptr;

        float lastFrameTime = 0;
        double lastFPSTime = -1;
        int frameCount = 0;
        float fps = 0.0f;

        void calculateFPS();

        void setWindow(class window* target);
        void setCamera(class camera* camera);
    };


}
