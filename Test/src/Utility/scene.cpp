#include <scene.h>

namespace opengl
{
    // Scene 类实现
    Scene::Scene() = default;
    Scene::~Scene() = default;

    void Scene::addObject(Object *obj) {
        objects.push_back(obj);
    }

    void Scene::removeObject(Object *obj) {
        objects.remove_if([&obj](const Object *o) { return &o == &obj; });
        delete obj;
    }

    std::list<Object*>& Scene::getObjects() {
        return objects;
    }

    // Object 类实现
    Object::Object(Scene* scene)
    {
        this->scene = scene;
        this->scene->addObject(this);

        this->transform = Transform();

        this->renderer = new SpriteRenderer(nullptr, nullptr);
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
