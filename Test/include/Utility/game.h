#pragma once
#include <shader.hpp>
#include <system.h>
#include <window.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <map>
#include <scene.h>
#include <cmath>

namespace opengl
{
    class GameLevel;
    class Object;
    class BallObject;
    class ParticleObject;
    class Scene;
    class Texture;

    class Game
    {
    public:
        ~Game() = default;

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

        void loadLevel(GameLevel* level);
        void loadLevel(int level);
        void loadLevel();

        enum GameState
        {
            GAME_ACTIVE,
            GAME_MENU,
            GAME_WIN
        };
        GameState State = GAME_ACTIVE;

        Texture* blockTex;
        Texture* solidBlockTex;
        Texture* backgroundTex;
        Texture* ballTex;
        Texture* particleTex;
    private:
        Game() { init(); };
        static Game* instance;
        std::map<std::string, Scene*> scenes;
        std::vector<GameLevel*> levels;
        int currentLevelIndex = 0;
        std::vector<ParticleObject*> particles;
        float particleSpawnTimer = 0.0f;
        const float spawnInterval = 0.05f;

        Object* player = nullptr;
        // 初始化挡板的大小
        const glm::vec2 PLAYER_SIZE = glm::vec2(100, 20);
        // 初始化挡板的速率
        const GLfloat PLAYER_VELOCITY = 500.0f;

        BallObject* ball = nullptr;
        const GLfloat BALL_RADIUS = 12.5f;
        const glm::vec2 INITIAL_BALL_VELOCITY = glm::vec2(100.0f, -350.0f);
        // const glm::vec2 INITIAL_BALL_VELOCITY = glm::vec2(150.0f, -150.0f);

        class window* window = nullptr;
        class camera* camera = nullptr;

        float lastFrameTime = 0;
        double lastFPSTime = -1;
        int frameCount = 0;
        float fps = 0.0f;

        void calculateFPS();

        void setWindow(class window* target);
        void setCamera(class camera* camera);

        void processInput(float dt);

        void processCollisions();

        void resetPlayer();

        ParticleObject* getFirstUnusedParticle();
        void updateParticles(float dt);
        void drawScene(const char* sceneName);
    };

    class GameLevel
    {
    public:
        std::vector<Object*> objects;

        GameLevel() = default;
        ~GameLevel() = default;

        void load(std::string levelFile, GLuint levelWidth, GLuint levelHeight);
        bool isCompleted();
    private:
        void init(std::vector<std::vector<GLuint>> tileData);
        GLuint levelWidth{};
        GLuint levelHeight{};
    };
}
