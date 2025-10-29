#include <common.h>
#include <map>
using namespace opengl;

std::map<std::string, Model*> models;
std::map<std::string, Shader*> shaders;

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

float near_plane = 1.0f, far_plane = 25.0f;
glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
                              glm::vec3( 0.0f, 0.0f,  0.0f),
                              glm::vec3( 0.0f, 1.0f,  0.0f));
glm::mat4 lightSpaceMatrix = lightProjection * lightView;

GLfloat aspect = (GLfloat)SHADOW_WIDTH/(GLfloat)SHADOW_HEIGHT;
glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);

glm::vec3 lightPos(0.0f, 4.0f, 0.0f);

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
    Shader* blinnPhongShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/BlinnPhong/blinn_phong.vert"),
                                         FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/BlinnPhong/blinn_phong.frag"));
    shaders["blinnPhongShader"] = blinnPhongShader;

    Shader* blinnPhongWithOmniShadowMapShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/BlinnPhongWithOmniShadowMap/blinn_phong_with_omni_shadow_map.vert"),
                                         FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/BlinnPhongWithOmniShadowMap/blinn_phong_with_omni_shadow_map.frag"));
    shaders["blinnPhongWithOmniShadowMapShader"] = blinnPhongWithOmniShadowMapShader;

    Shader* shadowMapShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/ShadowMap/shadow_map.vert"),
                                        FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/ShadowMap/shadow_map.frag"));
    shaders["shadowMapShader"] = shadowMapShader;

    Shader* omniShadowMapShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/OmnidirectionalShadowMap/omnidirectional_shadow_map.vert"),
                                        FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/OmnidirectionalShadowMap/omnidirectional_shadow_map.frag"),
                                        FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/OmnidirectionalShadowMap/omnidirectional_shadow_map.geom"));
    shaders["omniShadowMapShader"] = omniShadowMapShader;

    Shader* litShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/Lit/lit.vert"),
                                        FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/Lit/lit.frag"));
    shaders["lit"] = litShader;

    // Shader* gBufferShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/GBuffer/gbuffer.vert"),
    //                                     FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/GBuffer/gbuffer.frag"));
    // shaders["gBufferShader"] = gBufferShader;
    Shader* deferredLightingShader = new Shader(FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/DeferredLighting/deferred_lighting.vert"),
                                        FileSystem::getPath("src/LearnOpenGL/AdvancedLighting/Shaders/DeferredLighting/deferred_lighting.frag"));
    shaders["deferredLightingShader"] = deferredLightingShader;
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
    // shader->setVec3("dirLight.direction", {-0.2f, -1.0f, -0.3f});
    // shader->setVec3("dirLight.ambient", {0.05f, 0.05f, 0.05f});
    // shader->setVec3("dirLight.diffuse", {0.8f, 0.8f, 0.8f});
    // shader->setVec3("dirLight.specular", {0.6f, 0.6f, 0.6f});

    // 点光源
    shader->setVec3("pointLights[0].position", lightPos);

    // shader->setVec3("pointLights[0].ambient", {0.5f, 0.5f, 0.5f});
    shader->setVec3("pointLights[0].ambient", {5.0f, 5.0f, 5.0f});
    // shader->setVec3("pointLights[0].diffuse", {10.0f, 10.0f, 10.0f});
    shader->setVec3("pointLights[0].diffuse", {1.0f, 1.0f, 1.0f});
    shader->setVec3("pointLights[0].specular", {1.0f, 1.0f, 1.0f});

    shader->setFloat("pointLights[0].constant", 1.0f);
    shader->setFloat("pointLights[0].linear", 0.09f);
    shader->setFloat("pointLights[0].quadratic", 0.032f);

    // 聚光灯
    // shader->setVec3("spotLight.position", {0.0, 1.0, -4.0});
    // shader->setVec3("spotLight.direction", {0.0, -1.0, 2.0});
    //
    // shader->setVec3("spotLight.ambient", {0.0f, 0.0f, 0.0f});
    // shader->setVec3("spotLight.diffuse", {1.0f, 1.0f, 1.0f});
    // shader->setVec3("spotLight.specular", {1.0f, 1.0f, 1.0f});
    //
    // shader->setFloat("spotLight.cutOff", glm::cos(glm::radians(7.5f)));
    // shader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

unsigned int genDepthMapFBO()
{
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);

    return fbo;
}

unsigned int genDepthMapTex(unsigned int depthMapFBO)
{
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 边界视为“无限远”（不在阴影中）
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return depthMap;
}

unsigned int genOmniDepthMapTex(unsigned int depthMapFBO)
{
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return depthMap;
}

void renderShadowMap(unsigned int depthMapFBO, const std::function<void(Shader*)>& renderScene)
{
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    auto shader = shaders["shadowMapShader"];
    shader->use();
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    // glCullFace(GL_FRONT); // 避免阴影悬浮，但似乎作用不大
    renderScene(shader);
    // glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, opengl::window::getWidth(), opengl::window::getHeight());
}

void renderOmnidirectionalShadowMaps(unsigned int depthMapFBO, glm::vec3 lightPos, const std::function<void(Shader*)>& renderScene)
{
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClearDepth(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    auto shader = shaders["omniShadowMapShader"];
    shader->use();
    shader->setVec3("lightPos", lightPos);
    shader->setFloat("far_plane", far_plane);

    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj *
                     glm::lookAt(lightPos, lightPos + glm::vec3(1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
    shadowTransforms.push_back(shadowProj *
                     glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
    shadowTransforms.push_back(shadowProj *
                     glm::lookAt(lightPos, lightPos + glm::vec3(0.0,1.0,0.0), glm::vec3(0.0,0.0,1.0)));
    shadowTransforms.push_back(shadowProj *
                     glm::lookAt(lightPos, lightPos + glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0)));
    shadowTransforms.push_back(shadowProj *
                     glm::lookAt(lightPos, lightPos + glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,-1.0,0.0)));
    shadowTransforms.push_back(shadowProj *
                     glm::lookAt(lightPos, lightPos + glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,-1.0,0.0)));

    for (unsigned int i = 0; i < 6; ++i)
    {
        shader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    }

    renderScene(shader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, opengl::window::getWidth(), opengl::window::getHeight());
}

void updateLightPos(float time)
{
    // lightPos.x = 5.0f * std::sin(time);
    lightPos.x = 0.0f;
    // lightPos.z = 1.0f * std::cos(time);
    lightPos.z = -0.0f;
    lightPos.y = 3.0f;
}

int main()
{
    system::init();
    if (!system::createWindow("advancedLighting"))
    {
        return -1;
    }

    loadModel();
    loadShader();
    // setLightUniforms(shaders["blinnPhongShader"]);
    // setLightUniforms(shaders["blinnPhongWithOmniShadowMapShader"]);

    unsigned int depthMapFBO = genDepthMapFBO();
    // unsigned int depthMapTex = genDepthMapTex(depthMapFBO);
    unsigned int depthMapTex = genOmniDepthMapTex(depthMapFBO);

    setModelTransform();

    // 使用万向深度贴图作为天空盒立方体贴图调试输出
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, depthMapTex);

    while (!system::systemShouldEnd())
    {
        updateLightPos((float)glfwGetTime());

        // setLightUniforms(shaders["blinnPhongWithOmniShadowMapShader"]);
        // setLightUniforms(shaders["lit"]);

        // 单向阴影贴图
        // renderShadowMap(depthMapFBO, [](Shader* shader){
        //         models["woodFloor"]->DrawInstanced(*shader, 49);
        //         models["marbleBlock"]->DrawInstanced(*shader, 10);
        //     });// 要放update外，因为update中相机fbo不是默认的0，框架问题

        // renderOmnidirectionalShadowMaps(depthMapFBO, lightPos, [](Shader* shader){
        //         models["woodFloor"]->DrawInstanced(*shader, 49);
        //         models["marbleBlock"]->DrawInstanced(*shader, 10);
        //         models["brickWall"]->DrawInstanced(*shader, 1);
        //         models["redBrickWall"]->DrawInstanced(*shader, 1);
        //         models["nanosuit"]->DrawInstanced(*shader, 1);
        //     });
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        system::getCamera()->drawGBufferFrameBuffer([](Shader* shader){
                models["woodFloor"]->DrawInstanced(*shader, 49);
                models["marbleBlock"]->DrawInstanced(*shader, 10);
                models["brickWall"]->DrawInstanced(*shader, 1);
                models["redBrickWall"]->DrawInstanced(*shader, 1);
                models["nanosuit"]->DrawInstanced(*shader, 1);
            });
        system::getCamera()->drawSSAOBuffer();
        // drawGBufferFrameBuffer([](Shader* shader){
        //     models["woodFloor"]->DrawInstanced(*shader, 49);
        //     models["marbleBlock"]->DrawInstanced(*shader, 10);
        //     models["brickWall"]->DrawInstanced(*shader, 1);
        //     models["redBrickWall"]->DrawInstanced(*shader, 1);
        // });

        system::update([depthMapTex](){
            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D, depthMapTex);
            //
            // system::getCamera()->drawFullScreen(depthMapTex);
                
            // system::getCamera()->drawFullScreen(system::getCamera()->getSSAOTex());
            // return;

            // auto shader = shaders["blinnPhongShader"];
            // auto shader = shaders["blinnPhongWithOmniShadowMapShader"];
            if (!system::deferredLighting)
            // 传统前向渲染
            {
                auto shader = shaders["lit"];

                shader->use();
                setLightUniforms(shader);
                shader->setVec3("viewPos", system::getCamera()->Position);
                shader->setFloat("material.shininess", 8.0f);
                shader->setBool("material.useBlinnPhong", system::useBlinnPhong);
                shader->setBool("useSSAO", system::useSSAO);

                shader->setInt("skybox", 4);
                shader->setInt("shadowMap", 5);

                // 单向阴影贴图
                // shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
                // 用于点光源的全向阴影贴图
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_CUBE_MAP, depthMapTex);

                glActiveTexture(GL_TEXTURE6);
                glBindTexture(GL_TEXTURE_2D, system::getCamera()->getSSAOTex());

                shader->setFloat("farPlane", far_plane);


                // 单向阴影贴图
                // glActiveTexture(GL_TEXTURE5);
                // glBindTexture(GL_TEXTURE_2D, depthMapTex);
                // 用于天空盒反射
                // system::getCamera()->bindSkybox(4);

                models["woodFloor"]->DrawInstanced(*shader, 49);
                models["marbleBlock"]->DrawInstanced(*shader, 10);
                models["brickWall"]->DrawInstanced(*shader, 1);
                models["redBrickWall"]->DrawInstanced(*shader, 1);
                models["nanosuit"]->DrawInstanced(*shader, 1);
            }
            else
            // 延迟渲染
            {
                auto shader = shaders["deferredLightingShader"];
                shader->use();
                setLightUniforms(shader);
                shader->setVec3("viewPos", system::getCamera()->Position);
                shader->setFloat("material.shininess", 8.0f);
                shader->setInt("gPosition", 0);
                shader->setInt("gNormal", 1);
                shader->setInt("gAlbedoSpec", 2);
                shader->setInt("ssao", 3);
                shader->setBool("useSSAO", system::useSSAO);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, system::getCamera()->getGPositionTex());
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, system::getCamera()->getGNormalTex());
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, system::getCamera()->getGAlbedoSpec());
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, system::getCamera()->getSSAOTex());

                system::getCamera()->copyDepthGBufferToCameraFrameBuffer();

                system::getCamera()->drawFullScreen(shader);

                // // 延迟渲染+前向渲染，开启记得注释掉GBUFFER渲染的部分
                // shader = shaders["lit"];
                //
                // shader->use();
                // setLightUniforms(shader);
                // shader->setVec3("viewPos", system::getCamera()->Position);
                // shader->setFloat("material.shininess", 8.0f);
                // shader->setBool("material.useBlinnPhong", system::useBlinnPhong);
                //
                // shader->setInt("skybox", 4);
                // shader->setInt("shadowMap", 5);
                //
                // // 用于点光源的全向阴影贴图
                // glActiveTexture(GL_TEXTURE5);
                // glBindTexture(GL_TEXTURE_CUBE_MAP, depthMapTex);
                //
                // shader->setFloat("farPlane", far_plane);
                //
                //
                // models["woodFloor"]->DrawInstanced(*shader, 49);
            }
        });


    }

    return 0;
}