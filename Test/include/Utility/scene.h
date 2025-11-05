#pragma once
#include <list>
#include <spriteRenderer.h>
#include <game.h>
#include <types.h>

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

            return model;
        }
    };

    class Object
    {
    public:
        Object();
        Object(Scene *scene);
        Object(Transform transform, glm::vec4 color);
        virtual ~Object();

        void setRenderer(Renderer *renderer);
        Renderer* getRenderer() { return renderer; }

        void setPosition(const glm::vec3& position) { transform.position = position; }
        glm::vec3 getPosition() const { return transform.position; }
        void setRotation(const glm::vec3& rotation) { transform.rotation = rotation; }
        glm::vec3 getRotation() const { return transform.rotation; }
        void setScale(const glm::vec3& scale) { transform.scale = scale; }
        glm::vec3 getScale() const { return transform.scale; }

        void setScene(Scene *scene) { this->scene = scene; }

        void Destroy() { scene->removeObject(this); isDestroyed = true; }

        bool isDestroyed = false;
        bool isSolid = false;
        glm::vec4 color;
        Transform transform;
    private:
        Scene *scene;
        Renderer *renderer;
    };

    class BallObject : public Object
    {
    public:
        bool active = false;
        float radius = 12.5f;

        BallObject() : Object(), active(false), radius(12.5f) {}
        BallObject(Scene *scene) : Object(scene), active(false), radius(12.5f) {}
        BallObject(Transform transform, glm::vec4 color) : Object(transform, color), active(false), radius(12.5f) {}

        glm::vec2 velocity = glm::vec2(0.0f, 0.0f);

        void move(float dt);
        void reset(glm::vec2 position, glm::vec2 velocity);
    };

    class ParticleObject : public Object
    {
    public:
        float life = 1.0f; // 当前生命周期
        float initialLife = 1.0f; // 初始生命周期
        glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
        ParticleObject() : Object(), life(0.0f), initialLife(1.0f) {}
        ParticleObject(Scene *scene) : Object(scene), life(0.0f), initialLife(1.0f) {}
        ParticleObject(Transform transform, glm::vec4 color) : Object(transform, color), life(0.0f), initialLife(1.0f) {}
    };

    class PropsObject : public Object
    {
    public:
        PropsType type = None;
        glm::vec2 velocity = glm::vec2(0.0f, 150.0f);

        PropsObject() : Object() {}
        PropsObject(Scene *scene) : Object(scene) {}
        PropsObject(Transform transform, glm::vec4 color) : Object(transform, color) {}
    };
}
