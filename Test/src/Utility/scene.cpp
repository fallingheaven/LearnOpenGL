#include <scene.h>

namespace opengl
{
    // Scene 类实现
    Scene::Scene() = default;
    Scene::~Scene() = default;

    void Scene::addObject(Object *obj) {
        obj->setScene(this);
        objects.push_back(obj);
    }

    void Scene::removeObject(Object *obj) {
        objects.remove_if([&obj](const Object *o) { return &o == &obj; });
        delete obj;
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
        this->color = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    Object::Object(Scene* scene)
    {
        this->scene = scene;
        this->scene->addObject(this);

        this->transform = Transform();

        this->renderer = new SpriteRenderer(nullptr, nullptr);
        this->color = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    Object::Object(Transform transform, glm::vec3 color)
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
}
