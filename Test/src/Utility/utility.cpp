#include <utility.h>

float Utility::angle2Radian(float angle)
{
    return angle / 180.0 * PI;
}
float Utility::radian2Angle(float radian)
{
    return radian / PI * 180.0;
}

glm::vec3 Utility::GetRandomVec3(float min, float max)
{
    float x = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max - min)));
    float y = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max - min)));
    float z = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max - min)));
    return {x, y, z};
}

unsigned int Utility::GenModelMatBuffer(int amount, glm::vec3 scale, glm::vec2 range)
{
    std::vector<glm::mat4> modelMatrices;
    for (int i = 0; i < amount; i++)
    {
        glm::mat4 tmp(1.0f);
        tmp = glm::translate(tmp, GetRandomVec3(range.x, range.y));
        tmp = glm::scale(tmp, scale);
        modelMatrices.push_back(tmp);
    }

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), modelMatrices.data(), GL_STATIC_DRAW);

    return buffer;
}

unsigned int Utility::GenModelMatBuffer(glm::vec3 position, glm::vec3 scale)
{
    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &modelMatrix, GL_STATIC_DRAW);

    return buffer;
}

unsigned int Utility::GenModelMatBuffer(std::vector<glm::vec3> &position, glm::vec3 scale)
{
    std::vector<glm::mat4> modelMatrices;
    for (const auto& pos : position)
    {
        glm::mat4 tmp(1.0f);
        tmp = glm::translate(tmp, pos);
        tmp = glm::scale(tmp, scale);
        modelMatrices.push_back(tmp);
    }

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data(), GL_STATIC_DRAW);

    return buffer;
}

std::string Utility::getFileExtension(const std::string& filePath)
{
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        return filePath.substr(dotPos + 1);
    }
    return "";
}

std::vector<glm::vec3> Utility::genSSAOKernel(int kernelSize)
{
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < kernelSize; ++i)
    {
        glm::vec3 sample(
            randomFloats(generator) * 2 - 1, // -1.0 to 1.0
            randomFloats(generator) * 2 - 1, // -1.0 to 1.0
            randomFloats(generator)    //  0.0 to 1.0
        );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);

        // 距离球心由近到远采样，将核心样本更多地分布在原点附近
        float scale = float(i) / float(kernelSize);
        scale = 0.1f + scale * scale * (1.0f - 0.1f);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    return ssaoKernel;
}

unsigned int Utility::genSSAONoiseTexture(int texSize)
{
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec2> ssaoNoise;
    for (unsigned int i = 0; i < texSize * texSize; i++)
    {
        glm::vec2 noise(
            randomFloats(generator) * 2 - 1, // -1.0 to 1.0
            randomFloats(generator) * 2 - 1); // -1.0 to 1.0
        ssaoNoise.push_back(noise);
    }

    unsigned int noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, texSize, texSize, 0, GL_RG, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return noiseTexture;
}



unsigned int Utility::TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    if (getFileExtension(path) == "png") {
        stbi_set_flip_vertically_on_load(true);
    } else {
        stbi_set_flip_vertically_on_load(false);
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat = 0, dataFormat = 0;
        if (nrComponents == 1)
        {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            // 如果启用了 gamma 校正，使用 SRGB 内部格式
            internalFormat = gamma ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            // 如果启用了 gamma 校正，使用 SRGB_ALPHA 内部格式
            internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        if (internalFormat != 0 && dataFormat != 0)
        {
            glBindTexture(GL_TEXTURE_2D, textureID);
            // 使用正确的 internalFormat 和 dataFormat
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            std::cout << "Texture loaded successfully: " << filename << std::endl;
        }
        else
        {
            std::cerr << "Texture failed to load (unsupported nrComponents): " << nrComponents << " at path: " << filename << std::endl;
        }

        stbi_image_free(data);
        std::cout << "Texture loaded successfully: " << filename << std::endl;
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);

        // 创建一个默认的白色纹理
        unsigned char whitePixel[3] = {255, 255, 255};
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return textureID;
}

unsigned int Utility::TextureFromMemory(const aiTexture* texture, bool gamma)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;

    // mHeight == 0 表示纹理是压缩格式（如png, jpg）
    // 否则是未压缩的 ARGB8888 格式
    unsigned char* data = nullptr;
    if (texture->mHeight == 0)
    {
        data = stbi_load_from_memory(
            reinterpret_cast<unsigned char*>(texture->pcData),
            texture->mWidth, // 压缩数据的大小
            &width, &height, &nrComponents, 0);
    }
    else
    {
        // Assimp 加载的未压缩数据是 ARGB 格式，需要手动处理或转换
        // 这里我们假设 stb_image 可以处理它，但可能需要更复杂的转换
        // 简单的处理方式是直接使用，但颜色通道可能错乱
        data = reinterpret_cast<unsigned char*>(texture->pcData);
        width = texture->mWidth;
        height = texture->mHeight;
        nrComponents = 4; // 假设是 ARGB
    }

    if (data)
    {
        GLenum internalFormat = 0, dataFormat = 0;
        if (nrComponents == 1)
        {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            // 如果启用了 gamma 校正，使用 SRGB 内部格式
            internalFormat = gamma ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            // 如果启用了 gamma 校正，使用 SRGB_ALPHA 内部格式
            internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        if (internalFormat != 0 && dataFormat != 0)
        {
            glBindTexture(GL_TEXTURE_2D, textureID);
            // 使用正确的 internalFormat 和 dataFormat
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            std::cerr << "Texture failed to load (unsupported nrComponents): " << nrComponents << std::endl;
        }

        // 如果是嵌入式纹理且是从内存加载的，释放 stb_image 的内存
        if (texture->mHeight == 0) {
            stbi_image_free(data);
        }
    }
    else
    {
        std::cout << "Texture failed to load from memory" << std::endl;
    }

    return textureID;
}

void Utility::glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam)
{
    // 忽略一些不重要的错误/警告代码
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
        default: std::cout << "Source: Unknown"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
        default: std::cout << "Type: Unknown"; break;
    } std::cout << std::endl;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
        default: std::cout << "Severity: Unknown"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}