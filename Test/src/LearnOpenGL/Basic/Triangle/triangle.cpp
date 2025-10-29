#include <complex>
#include <system.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <window.h>
#include <shader.hpp>
#include <utility.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
// 通过定义STB_IMAGE_IMPLEMENTATION，预处理器会修改头文件，让其只包含相关的函数定义源码，
// 等于是将这个头文件变为一个 .cpp 文件了。现在只需要在程序中包含stb_image.h并编译就可以了
// 但是这边因为克隆了整个仓库，应该不用
#include <stb_image.h>


// 顶点着色器源码
const char* vertexShaderSource = R"(
    #version 440 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    layout (location = 2) in vec2 aTexCoord;

    out vec3 inputColor;
    out vec2 TexCoord;

    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        inputColor = aColor;
        TexCoord = aTexCoord;
    }
)";

// 片段着色器源码
const char* fragmentShaderSource = R"(
    #version 440 core
    out vec4 FragColor;

    in vec3 inputColor;
    in vec2 TexCoord;

    // uniform vec4 inputColor;

    // uniform sampler2D ourTexture;

    uniform sampler2D texture1;
    uniform sampler2D texture2;

    void main()
    {
        // FragColor = vec4(inputColor.xyz, 1.0f);
        // FragColor = texture(ourTexture, TexCoord);
        // FragColor = texture(ourTexture, TexCoord) * vec4(inputColor, 1.0);
        FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
    }
)";

unsigned int VBO, VAO, EBO;
unsigned int shaderProgram;
Shader* shader;

int screenWidth = 1000, screenHeight = 1000;

void prepare_square()
{
    // float vertices[] = {
    //     -0.5f, -0.5f,  0.0f,   0.0f, 0.0f,
    //     -0.5f,  0.5f,  0.0f,   0.0f, 1.0f,
    //      0.5f, -0.5f,  0.0f,   1.0f, 0.0f,
    //      0.5f,  0.5f,  0.0f,   1.0f, 1.0f,
    // };
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    // 索引要求是uint
    unsigned int indices[] ={
        0, 1, 2,
        1, 2, 3
    };

    // 纹理
    {
        unsigned int texture1, texture2;
        glGenTextures(1, &texture1);
        glGenTextures(1, &texture2);

        int width, height, nrChannels;
        unsigned char *data = nullptr;

        // 纹理1
        glBindTexture(GL_TEXTURE_2D, texture1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // 纹理2
        glBindTexture(GL_TEXTURE_2D, texture2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        stbi_set_flip_vertically_on_load(true);
        data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // 绑定应用
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        stbi_image_free(data);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 使用glGetAttribLocation或直接在着色器中指定index都是可以的
    int vertexPosLocation = glGetAttribLocation(shaderProgram, "aPos");
    glVertexAttribPointer(vertexPosLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(vertexPosLocation);

    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    // // index为着色器的属性的location，如上面aPos的location=0
    // glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);
}

void prepare_triangle()
{
    // 定义三角形的顶点数据
    // float vertices[] = {
    //     -0.5f, -0.5f, 0.0f,
    //      0.5f, -0.5f, 0.0f,
    //      0.0f,  0.5f, 0.0f
    // };

    // float vertices[] = {
    //     //      位置                  颜色            纹理坐标
    //     -0.5f, -0.5f,  0.0f,   1.0f, 0.0f, 0.0f,   0.45f, 0.45f,
    //      0.5f, -0.5f,  0.0f,   0.0f, 1.0f, 0.0f,   0.55f, 0.45f,
    //      0.0f,  0.5f,  0.0f,   0.0f, 0.0f, 1.0f,   0.5f, 0.55f
    // };

    float vertices[] = {
        //      位置                  颜色            纹理坐标
        -0.5f, -0.5f,  0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,   0.0f, 0.0f, 1.0f,   0.5f, 1.0f
    };

    // 生成纹理
    {
        // 环绕方式和过滤方式
        {
            // 纹理环绕方式
            // GL_REPEAT	      对纹理的默认行为。重复纹理图像。
            // GL_MIRRORED_REPEAT 和GL_REPEAT一样，但每次重复图片是镜像放置的。
            // GL_CLAMP_TO_EDGE   纹理坐标会被约束在0到1之间，超出的部分会重复纹理坐标的边缘，产生一种边缘被拉伸的效果。
            // GL_CLAMP_TO_BORDER 超出的坐标为用户指定的边缘颜色。
            // 当使用GL_CLAMP_TO_BORDER，需要设置：
            //      float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
            //      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            // 纹理过滤
            // GL_NEAREST（也叫邻近过滤，Nearest Neighbor Filtering）是OpenGL默认的纹理过滤方式。
            // 当设置为GL_NEAREST的时候，OpenGL会选择中心点最接近纹理坐标的那个像素
            // GL_LINEAR（也叫线性过滤，(Bi)linear Filtering）
            // 它会基于纹理坐标附近的纹理像素，计算出一个插值，近似出这些纹理像素之间的颜色
            // 当纹理的大小和渲染屏幕的大小不一致时会出现两种情况：
            // - 第一种情况：纹理小于渲染屏幕，将会有一部分像素无法映射到屏幕上，对应于GL_TEXTURE_MIN_FILTER。
            // - 第二种情况：纹理大于渲染屏幕，没有足够的像素映射到屏幕上，GL_TEXTURE_MAG_FILTER。
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // 多级渐远纹理
            // GL_NEAREST_MIPMAP_NEAREST	使用最邻近的多级渐远纹理来匹配像素大小，并使用邻近插值进行纹理采样
            // GL_LINEAR_MIPMAP_NEAREST     使用最邻近的多级渐远纹理级别，并使用线性插值进行采样
            // GL_NEAREST_MIPMAP_LINEAR     在两个最匹配像素大小的多级渐远纹理之间进行线性插值，使用邻近插值进行采样
            // GL_LINEAR_MIPMAP_LINEAR      在两个邻近的多级渐远纹理之间使用线性插值，并使用线性插值进行采样
            // 如果放大过滤的选项设置为多级渐远纹理过滤选项之一，没有任何效果，因为多级渐远纹理主要是使用在纹理被缩小的情况下的：纹理放大不会使用多级渐远纹理
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }

        // 加载、绑定、生成纹理
        {
            // 创建纹理对象，并绑定（GL_TEXTURE_2D指向的对象就会是这个texture对象）
            unsigned int texture1;
            glGenTextures(1, &texture1);
            // glBindTexture(GL_TEXTURE_2D, texture1);
            unsigned int texture2;
            glGenTextures(1, &texture2);

            int width, height, nrChannels;
            unsigned char *data = nullptr;
            // 第一个纹理
            {
                glBindTexture(GL_TEXTURE_2D, texture1);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);

                if (!data)
                {
                    std::cerr << "加载纹理出错" << std::endl;
                    return;
                }

                // 为纹理对象附上纹理图像
                // 第三个参数告诉OpenGL我们希望把纹理储存为何种格式
                // 倒数第二第三个参数定义了源图的格式和数据类型
                // 如果要使用多级渐远纹理，我们必须手动设置所有不同的图像（不断递增第二个参数），或者,
                // 直接在生成纹理之后调用glGenerateMipmap。这会为当前绑定的纹理自动生成所有需要的多级渐远纹理
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            // 第二个纹理
            {
                glBindTexture(GL_TEXTURE_2D, texture2);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

                stbi_set_flip_vertically_on_load(true);
                data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
                if (!data)
                {
                    std::cerr << "加载纹理出错" << std::endl;
                    return;
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);

            stbi_image_free(data);
        }
    }

    // 创建顶点缓冲对象和顶点数组对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 绑定顶点数组对象
    glBindVertexArray(VAO);

    // 绑定顶点缓冲对象并传递数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 仅顶点
    {
        // 设置顶点属性指针
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        // glEnableVertexAttribArray(0);
    }

    // 顶点和颜色
    {
        // // 注意加入了颜色的数组的步长变成6*sizeof(float)
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        // glEnableVertexAttribArray(0);
        //
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
        // glEnableVertexAttribArray(1);
    }

    // 顶点、颜色和纹理
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        // 应用纹理
        {
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
            glEnableVertexAttribArray(2);
        }
    }
}

void draw_triangle()
{
    // 绘制三角形
    // glBindVertexArray(VAO);

    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    // glm::mat4 trans(1.0f);
    // trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
    // trans = glm::translate(trans, glm::vec3(0.0f, 1.0f, 0.0f));
    // trans = glm::scale(trans, glm::vec3(0.2f, 0.5f, 1.0f));
    // shader->setMat4("transform", trans);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // trans = glm::mat4(1.0f);
    // trans = glm::translate(trans, glm::vec3(-0.5f, 0.5f, 0.0f));
    // trans = glm::scale(trans, glm::vec3(1) * glm::max(0.0f, glm::sin((float)glfwGetTime())));
    // shader->setMat4("transform", trans);
    //
    // glDrawArrays(GL_TRIANGLES, 0, 3);

    // // 解绑
    // glBindVertexArray(0);
    // glUseProgram(0);

    // // 删除缓冲对象
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
}

void draw_square()
{
    // glm::mat4 model(1.0f);
    // model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

    // glm::mat4 view(1.0f);
    // view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    // float radius = 10.0f;
    // float camX = sin(glfwGetTime()) * radius;
    // float camZ = cos(glfwGetTime()) * radius;
    // glm::mat4 view;
    // view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    camera* camera = opengl::system::getCamera();
    glm::mat4 view = camera->getViewMatrix();

    glm::mat4 projection(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    // shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glDrawArrays(GL_TRIANGLES, 0, 36);

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
      };

    for(unsigned int i = 0; i < 10; i++)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, cubePositions[i]);
        float angle = 20.0f * (float)i;
        if (i % 3 == 0)
            model = glm::rotate(model, glm::radians(angle) + (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
        else
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        shader->setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

unsigned int apply_shader()
{
    int success;
    char infoLog[512];
    unsigned int vertexShader, fragmentShader;

    // 编译顶点着色器
    {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        // 检查编译错误
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // 编译片段着色器
    {
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        // 检查编译错误
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // 链接着色器程序
    {
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // 检查链接错误
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        // 删除着色器
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // 使用着色器程序
        // 没有更改着色器脚本不需要重复调用
        glUseProgram(shaderProgram);

        return shaderProgram;
    }
}

using namespace opengl;

int main()
{
    // 初始化系统
    system::init();
    system::createWindow("Triangle Example", screenWidth, screenHeight);

    if (system::getWindow() == nullptr || system::getWindow()->getInstance() == nullptr)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    shader = new Shader("../src/LearnOpenGL/Basic/Triangle/basic.vert", "../src/LearnOpenGL/Basic/Triangle/basic.frag");
    shader->use();
    shaderProgram = shader->ID;
    // shaderProgram = apply_shader();

    // prepare_triangle();
    prepare_square();

    // 一个纹理的默认纹理单元是0，它是默认的激活纹理单元，所以只有一个纹理时不用分配一个位置值
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);


    // float blueColor = 0.0f;
    // 主循环
    while (!glfwWindowShouldClose(system::getWindow()->getInstance()))
    {
        // blueColor      = static_cast<float>(std::sin(glfwGetTime()));
        // int inputColorLocation = glGetUniformLocation(shaderProgram, "inputColor"); // 注意uniform变量地址获取和普通变量的获取函数不同
        // glUniform4f(inputColorLocation, 0.0f, 0.2f, blueColor, 1.0f);

        // system::update(draw_triangle);

        system::update(draw_square);
    }

    system::clear();
    return 0;
}
