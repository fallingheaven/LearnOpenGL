#include <common.h>
using namespace opengl;

std::map<std::string, Model*> models;
std::map<std::string, Shader*> shaders;

void loadModel()
{
    Model* woodFloor = new Model(FileSystem::getPath("Assets/Models/WoodFloor/woodfloor.obj").data());
    models["woodFloor"] = woodFloor;
    Model* marbleBlock = new Model(FileSystem::getPath("Assets/Models/MarbleBlock/大理石块.obj").data());
    models["marbleBlock"] = marbleBlock;
    Model* brickWall = new Model(FileSystem::getPath("Assets/Models/BrickWall/brickwall.obj").data());
    models["brickWall"] = brickWall;
    Model* redBrickWall = new Model(FileSystem::getPath("Assets/Models/RedBrickWall/red_brick_wall.obj").data());
    models["redBrickWall"] = redBrickWall;
    Model* nanosuit = new Model(FileSystem::getPath("Assets/Models/warman/nanosuit.obj").data());
    models["nanosuit"] = nanosuit;
}

void loadShader()
{
    Shader* shadowMapShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Others/CSM/Shaders/ShadowMap/shadow.vert"),
                                        FileSystem::getPath("src/LearnOpenGL/Others/CSM/Shaders/ShadowMap/shadow.frag"),
                                        FileSystem::getPath("src/LearnOpenGL/Others/CSM/Shaders/ShadowMap/shadow.geom"));
    shaders["shadowMapShader"] = shadowMapShader;

    Shader* litShader = new Shader(FileSystem::getPath("src/LearnOpenGL/Others/CSM/Shaders/Lit/lit.vert"),
                           FileSystem::getPath("src/LearnOpenGL/Others/CSM/Shaders/Lit/lit.frag"));
    shaders["lit"] = litShader;
}

std::vector<float> planeSplits = {0.0f, 0.05f, 0.15f, 0.45f, 1.0f};
std::vector<glm::mat4> lightVP;
GLuint lightVPUBO;
glm::vec3 lightDir = {-0.2f, -1.0f, -0.3f};

GLuint shadowFBO;
GLuint shadowMapArray;
const GLuint SHADOW_MAP_RESOLUTION = 2048;

void bindShadowFBO()
{
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    glGenTextures(1, &shadowMapArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, planeSplits.size() - 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMapArray, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
        throw std::exception();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenBuffers(1, &lightVPUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightVPUBO);
    glBufferData(GL_UNIFORM_BUFFER, (planeSplits.size() - 1) * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightVPUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

glm::mat4 createLightViewProjectionMat(std::vector<glm::vec4> &corners, glm::vec3 lightDir)
{
    glm::vec3 center(0.0f);
    for (const auto& corner : corners)
    {
        center += glm::vec3(corner);
    }
    center /= corners.size();

    // 让光源离中心稍远一点（避免在中心附近只有1单位）
    float lightDistance = 50.0f; // 根据场景尺度调整
    glm::vec3 lightPos = center - glm::normalize(lightDir) * lightDistance;

    glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0.0f, 1.0f, 0.0f));
    std::vector<glm::vec4> cornersInLightSpace;
    cornersInLightSpace.reserve(corners.size());
    for (auto &c : corners)
    {
        cornersInLightSpace.push_back(lightView * c);
    }

    glm::mat4 lightProjection = Utility::createOrthoProjectionMatrixFromFrustumCorners(cornersInLightSpace);
    return lightProjection * lightView;
}

void createLightVP()
{
    lightVP.clear();

    float n = system::getCamera()->getNearPlane();
    float f  = system::getCamera()->getFarPlane();
    float fov = glm::radians(system::getCamera()->Zoom);
    float aspectRatio = (float)opengl::window::getWidth() / (float)opengl::window::getHeight();
    for (int i = 0; i < planeSplits.size() - 1; i++)
    {
        float nearPlane = n + (f - n) * planeSplits[i];
        float farPlane  = n + (f - n) * planeSplits[i + 1];

        glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
        glm::mat4 view = system::getCamera()->getViewMatrix();
        std::vector<glm::vec4> frustumCorners = Utility::getFrustumCornersWorldSpace(projection, view);

        glm::mat4 lightVPMat = createLightViewProjectionMat(frustumCorners, normalize(lightDir));
        lightVP.push_back(lightVPMat);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, lightVPUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, lightVP.size() * sizeof(glm::mat4), lightVP.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void drawScene(Shader *shader, int baseUnit)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw Scene");
    {
        models["woodFloor"]->DrawInstanced(*shader, 49, baseUnit);
        models["marbleBlock"]->DrawInstanced(*shader, 10, baseUnit);
        models["brickWall"]->DrawInstanced(*shader, 1, baseUnit);
        models["redBrickWall"]->DrawInstanced(*shader, 1, baseUnit);
        models["nanosuit"]->DrawInstanced(*shader, 1, baseUnit);
    }
    glPopDebugGroup();
}

void drawShadowMap()
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw Cascaded Shadow Map");
    {
        shaders["shadowMapShader"]->use();
        glViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightVPUBO);
        drawScene(shaders["shadowMapShader"], 1);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, opengl::window::getWidth(), opengl::window::getHeight());
    }
    glPopDebugGroup();
}

void setModelTransform()
{
    std::vector<glm::vec3> positions;
    for (int i = -3; i <= 3; i++)
    {
        for (int j = -3; j <= 3; j++)
        {
            positions.emplace_back(i*2, -1.0f, j*2);
        }
    }
    unsigned int floorModelVBO = Utility::GenModelMatBuffer(positions, glm::vec3(1.0f));
    models["woodFloor"]->SetAttributeMat4(floorModelVBO, 4, 1);

    // std::vector<glm::vec3> blockPositions;
    // blockPositions.emplace_back(-2.0f, -0.0f, -2.0f);
    unsigned int blockModelVBO = Utility::GenModelMatBuffer(10, glm::vec3(0.2f), {-2, 2});
    // unsigned int blockModelVBO = Utility::GenModelMatBuffer(blockPositions, glm::vec3(1.0f));
    models["marbleBlock"]->SetAttributeMat4(blockModelVBO, 4, 1);

    std::vector<glm::vec3> brickWallPositions;
    brickWallPositions.emplace_back(0.0f, 1.0f, 0.0f);
    unsigned int brickWallModelVBO = Utility::GenModelMatBuffer(brickWallPositions, glm::vec3(1.0f));
    models["brickWall"]->SetAttributeMat4(brickWallModelVBO, 4, 1);

    std::vector<glm::vec3> redBrickWallPositions;
    redBrickWallPositions.emplace_back(2.0f, 1.0f, 0.0f);
    unsigned int redBrickWallModelVBO = Utility::GenModelMatBuffer(redBrickWallPositions, glm::vec3(1.0f));
    models["redBrickWall"]->SetAttributeMat4(redBrickWallModelVBO, 4, 1);

    std::vector<glm::vec3> nanosuitPositions;
    nanosuitPositions.emplace_back(-2.0f, -1.5f, -1.0f);
    unsigned int nanosuitModelVBO = Utility::GenModelMatBuffer(nanosuitPositions, glm::vec3(0.2f));
    models["nanosuit"]->SetAttributeMat4(nanosuitModelVBO, 4, 1);
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
    if (!system::createWindow("Cascaded Shadow Mapping")) return -1;

    loadModel();
    loadShader();
    setModelTransform();
    bindShadowFBO();
    setLightUniforms(shaders["lit"]);

    while (!system::systemShouldEnd())
    {
        createLightVP();

        drawShadowMap();

        system::update([]() {
            auto shader = shaders["lit"];
            shader->use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapArray);
            shader->setInt("shadowMap", 0);
            shader->setVec3("viewPos", system::getCamera()->Position);
            shader->setFloat("near", system::getCamera()->getNearPlane());
            shader->setFloat("far", system::getCamera()->getFarPlane());
            shader->setFloat("cascadeSplits", planeSplits.data(), planeSplits.size());

            drawScene(shader, 1);
        });
    }
}