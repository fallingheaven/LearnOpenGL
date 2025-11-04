#include <scene.h>

namespace opengl
{
    // Scene 类实现
    Scene::Scene() = default;
    Scene::~Scene() = default;

    void Scene::addObject(Object *obj) {
        obj->setScene(this);
        obj->isDestroyed = false;
        objects.push_back(obj);
    }

    void Scene::removeObject(Object *obj) {
        objects.remove_if([&obj](const Object *o) { return o == obj; });
    }

    void Scene::clearObjects()
    {
        objects.clear();
    }

    std::list<Object*>& Scene::getObjects() {
        return objects;
    }

    // Object 类实现
    Object::Object()
    {
        this->scene = nullptr;
        this->transform = Transform();

        this->renderer = new SpriteRenderer(nullptr, nullptr);
        this->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    Object::Object(Scene* scene)
    {
        this->scene = scene;
        this->scene->addObject(this);

        this->transform = Transform();

        this->renderer = new SpriteRenderer(nullptr, nullptr);
        this->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    Object::Object(Transform transform, glm::vec4 color)
    {
        this->scene = nullptr;
        this->transform = transform;

        this->renderer = new SpriteRenderer(nullptr, nullptr);
        this->color = color;
    }

    Object::~Object()
    {
        scene->removeObject(this);
    }

    void Object::setRenderer(Renderer* renderer)
    {
        this->renderer = renderer;
        this->renderer->owner = this;
    }

    void BallObject::move(float dt)
    {
        if (!active) return;

        auto offset = glm::vec3(velocity.x * dt, velocity.y * dt, 0.0f);
        transform.position += offset;

        if (transform.position.x <= 0.0f)
        {
            velocity.x = -velocity.x;
            transform.position.x = 0.0f;
        }
        else if (transform.position.x + radius * 2 >= Game::getInstance()->getWindow()->getWidth())
        {
            velocity.x = -velocity.x;
            transform.position.x = Game::getInstance()->getWindow()->getWidth() - radius * 2;
        }

        if (transform.position.y <= 0.0f)
        {
            velocity.y = -velocity.y;
            transform.position.y = 0.0f;
        }
    }

    void BallObject::reset(glm::vec2 position, glm::vec2 velocity)
    {
        transform.position = glm::vec3(position, transform.position.z);
        this->velocity = velocity;
        this->active = false;
    }


}
