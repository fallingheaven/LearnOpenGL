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

        glm::mat4 getModelMatrix() const
        {
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, scale);
            return model;
        }
    };

    class Object
    {
    public:
        Object(Scene *scene);
        ~Object();

        void setRenderer(Renderer *renderer);
        Renderer* getRenderer() { return renderer; }

        void setPosition(const glm::vec3& position) { transform.position = position; }
        glm::vec3 getPosition() const { return transform.position; }
        void setRotation(const glm::vec3& rotation) { transform.rotation = rotation; }
        glm::vec3 getRotation() const { return transform.rotation; }
        void setScale(const glm::vec3& scale) { transform.scale = scale; }
        glm::vec3 getScale() const { return transform.scale; }

        void Destroy() { scene->removeObject(this); }

        Transform transform;
    private:
        Scene *scene;
        Renderer *renderer;
    };
}
