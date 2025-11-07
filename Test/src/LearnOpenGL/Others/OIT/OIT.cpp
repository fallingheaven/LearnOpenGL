#include <common.h>
using namespace opengl;

std::map<std::string, Model*> models;
std::map<std::string, Shader*> shaders;


GLuint opaqueFBO, transparentFBO;
GLuint quadVAO, quadVBO;
GLuint opaqueTexture, depthTexture;
GLuint revealTexture, accumTexture;

glm::vec4 colors[4] = {
    glm::vec4(1.0f, 0.0f, 0.0f, 0.2f),
    glm::vec4(0.0f, 1.0f, 0.0f, 0.2f),
    glm::vec4(0.0f, 0.0f, 1.0f, 0.2f),
    glm::vec4(1.0f, 1.0f, 0.0f, 0.2f)
};

glm::vec3 quadPositions[4] = {
    glm::vec3( 0.0f,  0.0f, -0.5f),
    glm::vec3( 0.5f, -0.5f, 0.0f),
    glm::vec3( 0.5f,  0.5f, 0.0f),
    glm::vec3(-0.5f,  0.5f, -1.0f)
};

void loadModels()
{
    float quadVertices[] = {
        // positions        // uv
        -1.0f, -1.0f, 0.0f,	0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,

         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f
    };

    // quad VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void loadShaders()
{
    Shader* opaqueShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/Others/OIT/Shaders/Opaque/opaque.vert"),
        FileSystem::getPath("src/LearnOpenGL/Others/OIT/Shaders/Opaque/opaque.frag")
    );
    shaders["opaque"] = opaqueShader;

    Shader* transparentShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/Others/OIT/Shaders/Transparent/transparent.vert"),
        FileSystem::getPath("src/LearnOpenGL/Others/OIT/Shaders/Transparent/transparent.frag")
    );
    shaders["transparent"] = transparentShader;

    Shader* blendShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/Others/OIT/Shaders/Blend/blend.vert"),
        FileSystem::getPath("src/LearnOpenGL/Others/OIT/Shaders/Blend/blend.frag")
    );
    shaders["blend"] = blendShader;
}

void bindFBO()
{
    glGenFramebuffers(1, &opaqueFBO);
    glGenFramebuffers(1, &transparentFBO);

    GLuint width = window::getWidth();
    GLuint height = window::getHeight();

    glGenTextures(1, &opaqueTexture);
    glBindTexture(GL_TEXTURE_2D, opaqueTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);


    glGenTextures(1, &accumTexture);
    glBindTexture(GL_TEXTURE_2D, accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &revealTexture);
    glBindTexture(GL_TEXTURE_2D, revealTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 绑定
    glBindFramebuffer(GL_FRAMEBUFFER, opaqueFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, opaqueTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, transparentFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, revealTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    const GLenum transparentDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, transparentDrawBuffers);
}

void drawQuad()
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Opaque Pass");
    {
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glBindFramebuffer(GL_FRAMEBUFFER, opaqueFBO);
        glClearColor(0,0,0,0);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glBindVertexArray(quadVAO);

        auto shader = shaders["opaque"];
        shader->use();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), quadPositions[0]);
        shader->setVec4("color", colors[0]);
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glPopDebugGroup();

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Transparent Accumulation");
    {
        // 绘制半透明
        glBindFramebuffer(GL_FRAMEBUFFER, transparentFBO);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunci(0, GL_ONE, GL_ONE); // accumulation blend target
        glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); // revealge blend target
        glBlendEquation(GL_FUNC_ADD);

        glm::vec4 zeroFillerVec(0.0f, 0.0f, 0.0f, 0.0f);
        glm::vec4 oneFillerVec(1.0f, 1.0f, 1.0f, 1.0f);
        // use a four component float array or a glm::vec4(0.0)
        glClearBufferfv(GL_COLOR, 0, &zeroFillerVec[0]);
        // use a four component float array or a glm::vec4(1.0)
        glClearBufferfv(GL_COLOR, 1, &oneFillerVec[0]);

        auto shader = shaders["transparent"];
        shader->use();

        auto model = glm::translate(glm::mat4(1.0f), quadPositions[1]);
        shader->setVec4("color", colors[1]);
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        model = glm::translate(glm::mat4(1.0f), quadPositions[2]);
        shader->setVec4("color", colors[2]);
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        model = glm::translate(glm::mat4(1.0f), quadPositions[3]);
        shader->setVec4("color", colors[3]);
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glPopDebugGroup();

    // 绘制混合结果
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Blend Pass");
    {
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindFramebuffer(GL_FRAMEBUFFER, opaqueFBO);
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, revealTexture);
        auto shader = shaders["blend"];
        shader->use();
        // 使用binding后就不需要设置了
        // shader->setInt("accumTex", 0);
        // shader->setInt("revealTex", 1);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glPopDebugGroup();

    // glBindFramebuffer(GL_READ_FRAMEBUFFER, opaqueFBO);
    // glReadBuffer(GL_COLOR_ATTACHMENT0);
    // glBindTexture(GL_TEXTURE_2D, );
    system::getCamera()->drawFullScreen(opaqueTexture);

    glBindVertexArray(0);
}

int main()
{
    system::init();
    if (!system::createWindow("OIT - Order Independent Transparency"))
    {
        return -1;
    }

    auto camera = system::getCamera();

    loadModels();
    loadShaders();
    bindFBO();

    while (!system::systemShouldEnd())
    {

        system::update([](){
            drawQuad();
        });
    }

    system::close();
    return 0;
}