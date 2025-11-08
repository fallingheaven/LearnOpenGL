#include <common.h>
using namespace opengl;

std::map<std::string, Model*> models;
std::map<std::string, Shader*> shaders;


void loadModels()
{
    Model* RemyModel = new Model(
        FileSystem::getPath("Assets/Models/Shooter/Remy.fbx").c_str()
    );
    models["Remy"] = RemyModel;

    Model* XiLianModel = new Model(
        FileSystem::getPath("Assets/Models/XiLian/星穹铁道—昔涟5.pmx").c_str()
    );
    models["XiLian"] = XiLianModel;
}

void loadShaders()
{
    Shader* animationShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/Others/Animation/Shaders/animation.vert"),
        FileSystem::getPath("src/LearnOpenGL/Others/Animation/Shaders/animation.frag")
    );
    shaders["animation"] = animationShader;

    Shader* simpleShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/Others/Animation/Shaders/simple.vert"),
        FileSystem::getPath("src/LearnOpenGL/Others/Animation/Shaders/simple.frag")
    );
    shaders["simple"] = simpleShader;
}
void setLightUniforms(Shader* shader)
{
    shader->use();
    // 光照属性
    // 方向光
    shader->setVec3("dirLight.direction", {-0.2f, -1.0f, -0.3f});
    shader->setVec3("dirLight.ambient", {0.05f, 0.05f, 0.05f});
    shader->setVec3("dirLight.diffuse", {0.8f, 0.8f, 0.8f});
    shader->setVec3("dirLight.specular", {0.6f, 0.6f, 0.6f});
}


int main()
{
    system::init();
    if (!system::createWindow("Animation"))
    {
        return -1;
    }

    auto camera = system::getCamera();

    loadModels();
    loadShaders();
    setLightUniforms(shaders["animation"]);
    setLightUniforms(shaders["simple"]);

    std::vector<std::pair<std::string, Animation>> animations;

    std::string modelPath = FileSystem::getPath("Assets/Models/Shooter/Remy.fbx");
    std::filesystem::path modelDir = std::filesystem::path(modelPath).parent_path();
    std::string modelFilename = std::filesystem::path(modelPath).filename().string();

    for (const auto& entry : std::filesystem::directory_iterator(modelDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".fbx")
        {
            std::string animFilename = entry.path().filename().string();
            // 排除模型文件本身
            if (animFilename != modelFilename)
            {
                std::string animName = entry.path().stem().string(); // 使用文件名（不含扩展名）作为动画名
                animations.emplace_back(animName, Animation(entry.path().string(), models["Remy"]));
                std::cout << "Loaded animation: " << animName << std::endl;
            }
        }
    }

    int currentAnimationIndex = 0;
    Animator animator(&animations[currentAnimationIndex].second); // 使用第一个动画初始化Animator

    float deltaTime = 0.0f;
    float lastFrame = glfwGetTime();
    while (!system::systemShouldEnd())
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        animator.updateAnimation(deltaTime);

        system::update([deltaTime, &animator](){
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Render Animation Model");
            {
                auto shader = shaders["animation"];
                shader->use();

                auto transforms = animator.getFinalBoneMatrices();
                for (int i = 0; i < transforms.size(); ++i)
                {
                    shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
                }

                auto model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
                model = glm::scale(model, glm::vec3(0.01f));
                shader->setMat4("model", model);
                shader->setVec3("viewPos", system::getCamera()->Position);
                models["Remy"]->Draw(*shader);

                // 试试绘制pmx模型
                shader = shaders["simple"];
                shader->use();
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(2.0f, -1.5f, 0.0f));
                model = glm::scale(model, glm::vec3(0.1f));
                shader->setMat4("model", model);
                models["XiLian"]->Draw(*shader);
            }
            glPopDebugGroup();
        }, [&animations, &currentAnimationIndex, &animator]() {
            ImGui::Begin("Animation Control");
            if (ImGui::BeginCombo("Animations", animations[currentAnimationIndex].first.c_str()))
            {
                for (int n = 0; n < animations.size(); n++)
                {
                    bool isSelected = (currentAnimationIndex == n);
                    if (ImGui::Selectable(animations[n].first.c_str(), isSelected))
                    {
                        currentAnimationIndex = n;
                        animator.playAnimation(&animations[n].second);
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::End();
        });
    }

    system::close();
    return 0;
}