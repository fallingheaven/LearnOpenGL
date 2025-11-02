#pragma once
#include <list>
#include <spriteRenderer.h>
#include <game.h>

namespace opengl
{
    class Object;
    class Renderer;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void addObject(Object *obj);
        void removeObject(Object *obj);
        void clearObjects();
        std::list<Object*>& getObjects();
    private:
        std::list<Object*> objects;
    };

    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        Transform()
            : position(0.0f, 0.0f, 0.0f),
              rotation(0.0f, 0.0f, 0.0f),
              scale(1.0f, 1.0f, 1.0f)
        {}

        Transform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
            : position(pos), rotation(rot), scale(scl)
        {}

        Transform(const glm::vec2& pos, const glm::vec2& rot, const glm::vec2& scl)
            : position(glm::vec3(pos, 0)), rotation(glm::vec3(rot, 0)), scale(glm::vec3(scl, 1))
        {}

        glm::mat4 getModelMatrix() const
        {
            // std::cout << position.x << ", " << position.y << ", " << position.z << std::endl;
            // std::cout << rotation.x << ", " << rotation.y << ", " << rotation.z << std::endl;
            // std::cout << scale.x << ", " << scale.y << ", " << scale.z << std::endl;
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, scale);

            // std::cout << "Model Matrix: " << std::endl;
            // for (int i = 0; i < 4; ++i)
            // {
            //     for (int j = 0; j < 4; ++j)
            //     {
            //         std::cout << model[j][i] << " ";
            //     }
            //     std::cout << std::endl;
            // }std::cout << std::endl;

            return model;
        }
    };

    class Object
    {
    public:
        Object();
        Object(Scene *scene);
        Object(Transform transform, glm::vec3 color);
        ~Object();

        void setRenderer(Renderer *renderer);
        Renderer* getRenderer() { return renderer; }

        void setPosition(const glm::vec3& position) { transform.position = position; }
        glm::vec3 getPosition() const { return transform.position; }
        void setRotation(const glm::vec3& rotation) { transform.rotation = rotation; }
        glm::vec3 getRotation() const { return transform.rotation; }
        void setScale(const glm::vec3& scale) { transform.scale = scale; }
        glm::vec3 getScale() const { return transform.scale; }

        void setScene(Scene *scene) { this->scene = scene; }

        void Destroy() { scene->removeObject(this); }

        bool isDestroyed = true;
        glm::vec3 color;
        Transform transform;
    private:
        Scene *scene;
        Renderer *renderer;
    };
}
