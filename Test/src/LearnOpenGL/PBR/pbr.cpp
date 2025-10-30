#include <common.h>

using namespace opengl;

struct Character {
    GLuint     TextureID;  // 字形纹理的ID
    glm::ivec2 Size;       // 字形大小
    glm::ivec2 Bearing;    // 从基准线到字形左部/顶部的偏移值
    glm::ivec2 Advance;    // 原点距下一个字形原点的二维距离
};

std::map<wchar_t, Character> Characters;

std::map<std::string, Model*> models;
std::map<std::string, Shader*> shaders;

void loadModels()
{
    auto sphere = new Model(FileSystem::getPath("Assets/Models/PBRMetal/pbr_metal_detailed.glb").data());
    models["sphere"] = sphere;
}

void loadShaders()
{
    auto surfaceShader = new Shader(FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/Surface/surface.vert"),
                               FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/Surface/surface.frag"));
    shaders["surface"] = surfaceShader;

    auto equirectangularToCubeMapShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/EquirectangularToCubeMap/equirectangularToCubeMap.vert"),
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/EquirectangularToCubeMap/equirectangularToCubeMap.frag")
    );
    shaders["equirectangularToCubeMap"] = equirectangularToCubeMapShader;

    auto irradianceConvolutionShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/IrradianceConvolution/irradianceConvolution.vert"),
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/IrradianceConvolution/irradianceConvolution.frag")
    );
    shaders["irradianceConvolution"] = irradianceConvolutionShader;

    auto prefilterHDRMap = new Shader(
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/IrradianceConvolution/irradianceConvolution.vert"),
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/PrefilterHDRMap/prefilterHDRMap.frag")
    );
    shaders["prefilterHDRMap"] = prefilterHDRMap;

    auto brdfShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/BRDFMap/brdfMap.vert"),
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/BRDFMap/brdfMap.frag")
    );
    shaders["brdfShader"] = brdfShader;

    auto textShader = new Shader(
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/Text/text.vert"),
        FileSystem::getPath("src/LearnOpenGL/PBR/Shaders/Text/text.frag")
    );
    shaders["text"] = textShader;
}

void setLightInfo(Shader* shader)
{
    shader->use();
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };
    const float quadratic = 1.0f;
    for (unsigned int i = 0; i < 4; ++i)
    {
        shader->setVec3("pointLights[" + std::to_string(i) + "].position", lightPositions[i]);
        shader->setVec3("pointLights[" + std::to_string(i) + "].color", lightColors[i]);
        shader->setFloat("pointLights[" + std::to_string(i) + "].quadratic", quadratic);
    }
}

int rows = 5, cols = 5;

void setModelMatrices()
{
    std::vector<glm::vec3> positions;
    float spacing = 2.5;
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            glm::vec3 position;
            position.x = (col - cols / 2.0f) * spacing;
            position.y = (row - rows / 2.0f) * spacing;
            position.z = 0.0f;
            positions.push_back(position);
        }
    }
    unsigned int sphereVBO = Utility::GenModelMatBuffer(positions, glm::vec3(1.0f));
    models["sphere"]->SetAttributeMat4(sphereVBO, 4, 1);
}

unsigned int bindCubeMapVAO()
{
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // 三维UV
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);

    return cubeVAO;
}

// 设置捕获投影和视图矩阵
glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 25.0f);
glm::mat4 captureViews[] =
{
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
 };

unsigned int renderRadianceEquirectangularMapToCubeMap()
{
    // 加载 HDR 环境贴图
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    // float *data = stbi_loadf(FileSystem::getPath("Assets/Materials/hdr_map.hdr").c_str(),
    //     &width, &height, &nrComponents, 0);
    float *data = stbi_loadf(FileSystem::getPath("Assets/Materials/newport_loft.hdr").c_str(),
        &width, &height, &nrComponents, 0);
    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "HDR image loaded successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to load HDR image." << stbi_failure_reason() << std::endl;
    }

    const unsigned int captureSize = 512;

    // 创建帧缓冲
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureSize, captureSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // 创建纹理
    unsigned int envCubeMap;
    glGenTextures(1, &envCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                     captureSize, captureSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 用于基于PDF和粗糙度采样，可以去除反射IBL中的光斑
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    unsigned int cubeVAO = bindCubeMapVAO();
    glBindVertexArray(cubeVAO);

    // 渲染到立方体贴图
    auto shader = shaders["equirectangularToCubeMap"];
    // convert HDR equirectangular environment map to cubemap equivalent
    shader->use();
    shader->setInt("equirectangularMap", 0);
    shader->setMat4("cameraProjection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, captureSize, captureSize); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        shader->setMat4("cameraView", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window::getWidth(), window::getHeight());

    return envCubeMap;
}

unsigned int renderEnvMapToIrradianceMap(unsigned int envCubeMap)
{
    const unsigned int captureSize = 64;

    // 创建帧缓冲
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureSize, captureSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, captureSize, captureSize, 0,
                     GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto shader = shaders["irradianceConvolution"];
    shader->use();
    shader->setInt("cubeMap", 0);
    shader->setMat4("cameraProjection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

    unsigned int cubeVAO = bindCubeMapVAO();
    glBindVertexArray(cubeVAO);

    glViewport(0, 0, captureSize, captureSize); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        shader->setMat4("cameraView", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window::getWidth(), window::getHeight());

    return irradianceMap;
}

unsigned int renderEnvMapToSpecularMap(unsigned int envCubeMap)
{
    const unsigned int captureSize = 128;

    // 创建帧缓冲
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureSize, captureSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, captureSize, captureSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    auto shader = shaders["prefilterHDRMap"];
    shader->use();
    shader->setInt("cubeMap", 0);
    shader->setMat4("cameraProjection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

    unsigned int cubeVAO = bindCubeMapVAO();
    glBindVertexArray(cubeVAO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; mip++)
    {
        unsigned int mipWidth  = captureSize * std::pow(0.5, mip);
        unsigned int mipHeight = captureSize * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        shader->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            shader->setMat4("cameraView", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window::getWidth(), window::getHeight());

    return prefilterMap;
}

unsigned int renderBRDFMap()
{
    const unsigned int captureSize = 512;

    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, captureSize, captureSize, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 创建帧缓冲
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureSize, captureSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, captureSize, captureSize);
    auto shader = shaders["brdfShader"];
    shader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    system::getCamera()->drawFullScreenTex(captureFBO, shader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window::getWidth(), window::getHeight());

    return brdfLUTTexture;
}

void PrintExtensions()
{
    GLint nExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);

    for (GLint i = 0; i < nExtensions; ++i)
    {
        const GLubyte* extension = glGetStringi(GL_EXTENSIONS, i);
        std::cout << extension << std::endl;
    }
}

void loadFont(std::string& fontPath)
{
    std::setlocale(LC_ALL, "en_US.UTF-8"); // 或者 "zh_CN.UTF-8"

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    FT_Set_Pixel_Sizes(face, 0, 48);

    FT_Matrix matrix;
    FT_Vector pen;
    double angle = glm::radians(90.0); // 旋转角度，单位为弧度
    matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
    matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
    matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
    matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
    pen.x = 0;
    pen.y = 0;
    FT_Set_Transform(face, &matrix, &pen);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //禁用字节对齐限制
    for (GLubyte c = 0; c < 128; c++)
    {
        // 加载字符的字形
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // 生成纹理
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // 设置纹理选项
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 储存字符供之后使用
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            glm::ivec2(face->glyph->advance.x, face->glyph->advance.y)
        };
        Characters.insert(std::pair<wchar_t, Character>((wchar_t)c, character));
    }

    std::wstring chinese_chars = L"测试字体渲染功能";
    for (wchar_t c : chinese_chars) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // ... (与上面完全相同的纹理生成和存储逻辑)
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width, face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            glm::ivec2(face->glyph->advance.x, face->glyph->advance.y)
        };
        // std::cout << c << ' ' << character.TextureID << std::endl;
        Characters.insert(std::pair<wchar_t, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

unsigned int textVAO, textVBO;

void bindTextVAO()
{
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderText(Shader *s, std::wstring &text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    glm::mat4 projection = glm::ortho(0.0f, (float)window::getWidth(), 0.0f, (float)window::getHeight());

    s->use();
    s->setVec3("textColor", color);
    s->setMat4("projection", projection);
    s->setInt("text", 0);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(textVAO);
    glActiveTexture(GL_TEXTURE0);
    // 遍历文本中所有的字符
    for (wchar_t c : text)
    {
        Character ch = Characters[c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // 对每个字符更新VBO
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // 在四边形上绘制字形纹理
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // 更新VBO内存的内容
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // 绘制四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // 更新位置到下一个字形的原点，注意单位是1/64像素
        auto advance = glm::mat4(1.0f);
        advance = glm::rotate(advance, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::ivec2 offset = (glm::mat2)advance * ch.Advance;
        x += (offset.x >> 6) * scale; // 位偏移6个单位来获取单位为像素的值 (2^6 = 64)
        y += (offset.y >> 6) * scale; // 同时更新y坐标
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_DEPTH_TEST);
}

int main()
{
    system::init();
    if (!system::createWindow("pbr"))
    {
        return -1;
    }

    loadModels();
    loadShaders();

    setModelMatrices();

    std::vector<glm::vec2> metallicRoughness;
    for (int i = 0; i < rows; i++)
    {
        float metallic = (float)(i+1) / (rows);
        for (int j = 0; j < cols; j++)
        {
            float roughness = (float)(j+1) / (cols);
            metallicRoughness.emplace_back(metallic, roughness);
        }
    }
    shaders["surface"]->use();
    shaders["surface"]->setVec2("metallicRoughnessScale", metallicRoughness[0], rows*cols);
    setLightInfo(shaders["surface"]);

    unsigned int envCubeMap = renderRadianceEquirectangularMapToCubeMap();
    unsigned int irradianceMap = renderEnvMapToIrradianceMap(envCubeMap);
    unsigned int specularMap = renderEnvMapToSpecularMap(envCubeMap);
    unsigned int brdfLUT = renderBRDFMap();

    // PrintExtensions();

    GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(Utility::glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    std::string fontPath = FileSystem::getPath("Assets/Fonts/AaFuLuBangShu/AaFuLuBangShu-2.ttf");
    loadFont(fontPath);
    bindTextVAO();

    while (!system::systemShouldEnd())
    {
        system::update([envCubeMap, irradianceMap, specularMap, brdfLUT](){
            auto shader = shaders["surface"];
            shader->use();

            shader->setVec3("viewPos", system::getCamera()->Position);
            shader->setInt("irradianceMap", 0);
            shader->setInt("specularMap", 1);
            shader->setInt("brdfLUT", 2);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, specularMap);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, brdfLUT);
            shader->setBool("useIBL", system::useIBL);
            shader->setBool("useSpecularIBL", system::useSpecularIBL);

            // models["sphere"]->DrawInstanced(*shader, rows*cols, 3);

            // debug输出brdfLUT，即BRDF积分贴图
            // system::getCamera()->drawFullScreen(brdfLUT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, specularMap);
            if (system::drawSkyBox)
                system::getCamera()->drawSkybox(); // 渲染字体需要先渲染天空盒

            std::wstring wtext = L"测试字体渲染功能";
            renderText(shaders["text"], wtext, 50.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        });

        // GLuint errorMsg = glGetError();
        // if (errorMsg != GL_NO_ERROR)
        // {
        //     std::cout << errorMsg << std::endl;
        // }
    }

    return 0;
}