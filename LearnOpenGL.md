# LearnOpenGL

## 概述

OpenGL的运行机制一般被称为状态机，实际上执行的操作更像是设置一个配置文件，绑定当前的VAO、Shader、纹理、是否深度测试、是否混合等等。绘制命令会读取这些变量来影响最终的渲染效果

## 初始化

这里使用glfw+glad，前者可以替我们取创建上下文、管理资源、同步调度，后者查找、绑定函数指针。具体来讲，

- `glfwInit();`后，先准备资源
- `glfwCreateWindow()`创建窗口后，才真正创建上下文，上下文包括：
  - 创建 GPU 状态机；
  - 初始化 OpenGL 命令队列；
  - 建立 GPU 资源表；
  - 注册与窗口的交换链（swap chain）；
  - 绑定渲染线程。
- 调用`glfwMakeContextCurrent(window);`后，将线程绑定到该上下文，即这个窗口上
- 此外，调用`gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);`，glfw提供指针，glad查找、保存指针。要这么做是因为OpenGL是一个规范而非实现，不同显卡实现不同，不同操作系统提供了入口，需要通过这个入口动态获取函数地址

## 基础资源绑定

### 着色器

#### 创建和编译shader对象

```c++
const char* vertexShaderSource; // 着色器文本
unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); // 着色器对象
glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr); // 绑定顶点着色器
glCompileShader(vertexShader); // 编译顶点着色器
// 会将着色器源码传给驱动程序，编译为GPU机器语言，通常存放在GPU
```

#### 链接program

```c++
unsigned int shaderProgram = glCreateProgram(); // program对象
glAttachShader(shaderProgram, vertexShader); // 链接顶点着色器
glAttachShader(shaderProgram, fragmentShader); // 链接片元着色器
glLinkProgram(shaderProgram); // 状态机绑定program对象
// 两个attach相当于set函数，link用来匹配输入输出、确定变量位置、生成管线对象，之后shaderProgram就对应这一条管线了
```

#### 使用program

```c++
glUseProgram(shaderProgram);
```

#### 使用管线对象

```c++
GLuint vs = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vs_src);
GLuint fs = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fs_src);
GLuint pipeline;
glGenProgramPipelines(1, &pipeline);
glBindProgramPipeline(pipeline);
glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vs);
glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fs);
// OpengGL 4.1+新引入的，除了program，可以用管线对象来组合各阶段着色器，可以动态组合，热切换更快
```

### 顶点数据

```c++
float vertices[] = {
    -0.5f, -0.5f,  0.0f,   0.0f, 0.0f,
    -0.5f,  0.5f,  0.0f,   0.0f, 1.0f,
     0.5f, -0.5f,  0.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.0f,   1.0f, 1.0f,
};

unsigned int indices[] ={
        0, 1, 2,
        1, 2, 3
    };
```

#### 创建对象

```c++
unsigned int VAO, VBO, EBO;
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glGenBuffers(1, &EBO);
// 分配ID，但还没有分配显存
// EBO规定了绘制时按什么样的顺序取点
// 那个 1 应该是创建的数量
```

#### 绑定

```c++
glBindVertexArray(VAO);
// 设置当前的VAO对象（ID）
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
// 设置当前VAO的VBO对象（ID），在GPU中存储数据
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
// 设置当前VAO的EBO对象（这里是数据），在GPU中存储数据

// 这里当绑定EBO时，会直接设置VAO存储的EBO ID
// 当绑定一个VAO时，会将内部的EBO ID自动绑定覆盖当前的状态机EBO
```

#### 顶点属性指针

```c++
auto stride = 5 * sizeof(float);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
// 以此为例，0是着色器变量的location，3是这个属性的大小，GL_FLOAT是属性数据的类型，然后是是否归一化，stride是步长，即读取顶点数据的循环的步进i+=stride，最后是相对于每一次步进后的指针的偏移
// 简单来说，就是规定了取vertices中的哪些列
glEnableVertexAttribArray(0);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
glEnableVertexAttribArray(1);
// 设置attribute后，才会设置当前VAO的VBO ID为当前状态机绑定的VBO
// 着色器变量location如：layout (location = 0) in vec3 aPos; 
// 也可以通过这个方式获取location：int vertexPosLocation = glGetAttribLocation(shaderProgram, "aPos");
```

### 纹理绑定

#### 纹理绑定

```c++
unsigned int texture1;
glGenTextures(1, &texture1);
glBindTexture(GL_TEXTURE_2D, texture1);
```

#### 纹理参数

```c++
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// x方向越界时处理规则
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// y方向越界时处理规则
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                GL_LINEAR_MIPMAP_LINEAR);
// 屏幕像素>纹理像素时的采样方式，这里使用MIPMAP线性插值
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 屏幕像素<纹理像素时的采样方式

// 纹理环绕方式
// GL_REPEAT	      对纹理的默认行为。重复纹理图像。
// GL_MIRRORED_REPEAT 和GL_REPEAT一样，但每次重复图片是镜像放置的。
// GL_CLAMP_TO_EDGE   纹理坐标会被约束在0到1之间，超出的部分会重复纹理坐标的边缘，产生一种边缘被拉伸的效果。
// GL_CLAMP_TO_BORDER 超出的坐标为用户指定的边缘颜色。
// 当使用GL_CLAMP_TO_BORDER，需要设置：
//      float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
//      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

// 纹理过滤
// GL_NEAREST（也叫邻近过滤，Nearest Neighbor Filtering）是OpenGL默认的纹理过滤方式。
// 当设置为GL_NEAREST的时候，OpenGL会选择中心点最接近纹理坐标的那个像素
// GL_LINEAR（也叫线性过滤，(Bi)linear Filtering）
// 它会基于纹理坐标附近的纹理像素，计算出一个插值，近似出这些纹理像素之间的颜色
// 当纹理的大小和渲染屏幕的大小不一致时会出现两种情况：
// - 第一种情况：纹理小于渲染屏幕，将会有一部分像素无法映射到屏幕上，对应于GL_TEXTURE_MIN_FILTER。
// - 第二种情况：纹理大于渲染屏幕，没有足够的像素映射到屏幕上，GL_TEXTURE_MAG_FILTER。
// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

// 多级渐远纹理
// GL_NEAREST_MIPMAP_NEAREST	使用最邻近的多级渐远纹理来匹配像素大小，并使用邻近插值进行纹理采样
// GL_LINEAR_MIPMAP_NEAREST     使用最邻近的多级渐远纹理级别，并使用线性插值进行采样
// GL_NEAREST_MIPMAP_LINEAR     在两个最匹配像素大小的多级渐远纹理之间进行线性插值，使用邻近插值进行采样
// GL_LINEAR_MIPMAP_LINEAR      在两个邻近的多级渐远纹理之间使用线性插值，并使用线性插值进行采样
// 如果放大过滤的选项设置为多级渐远纹理过滤选项之一，没有任何效果，因为多级渐远纹理主要是使用在纹理被缩小的情况下的：纹理放大不会使用多级渐远纹理
            
```

#### 纹理数据

```c++
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
             GL_RGB, GL_UNSIGNED_BYTE, data);
// 第三个参数规定储存的格式，倒数第二、第三定义源图的数据格式和数据类型
glGenerateMipmap(GL_TEXTURE_2D);
// 调用此会自动生成多级渐远纹理，也可以改变glTexImage2D的第二个参数手动设置不同level的纹理
```

#### 纹理单元

```c++
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, texture1);
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, texture2);
// OpenGL允许绑定多个纹理，将着色器中的uniform sampler2D设为对应的GL_TEXTUREX后的X后，就能采样对应的纹理了
```

## 进阶内容

### 深度测试

OpenGL的深度测试运行于片元着色器和模板测试之后，通过测试时，会更新深度缓冲

有提前深度测试（Early Depth Test）支持在片元着色器之前进行深度测试，但这时片元着色器不能再写入深度了

```c++
glEnable(GL_DEPTH_TEST);
// 启用深度测试
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 每次渲染前，清空深度缓冲，防止复用上一次的结果
glDepthMask(GL_FALSE);
// 使用掩码，不允许更新深度缓冲，深度缓冲只读
glDepthFunc(GL_LESS);
// 规定深度比较规则
// GL_ALWAYS	永远通过深度测试
// GL_NEVER		永远不通过深度测试
// GL_LESS		在片段深度值小于缓冲的深度值时通过测试
// GL_EQUAL		在片段深度值等于缓冲区的深度值时通过测试
// GL_LEQUAL	在片段深度值小于等于缓冲区的深度值时通过测试
// GL_GREATER	在片段深度值大于缓冲区的深度值时通过测试
// GL_NOTEQUAL	在片段深度值不等于缓冲区的深度值时通过测试
// GL_GEQUAL	在片段深度值大于等于缓冲区的深度值时通过测试
FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
// 在glsl中，内建gl_FragCoord向量的xy存储了屏幕空间坐标（左下角为00），z值则是存储了深度值
```

深度缓冲的值在0~1之间，并非z值，当然可以简单地使用线性映射将z映射为深度，但在工程中，我们会希望在离近平面较近时，深度的精度要大一些，即希望一段z能对应更长的一段深度，如$F_{depth} = \frac{1/z - 1/near}{1/far - 1/near}$，其非线性关系对应如下：

<img src="https://learnopengl-cn.github.io/img/04/01/depth_non_linear_graph.png" alt="img" style="zoom:80%;" />

深度冲突（Z-Fight）指两个面相邻很近，导致渲染时不断切换顺序，尤其在远距离精度降低时发生

### 模板测试

使用stencil buffer，也就是一个掩码来决定要渲染哪些片元

```c++
glEnable(GL_STENCIL_TEST);
// 启用模板测试
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
// 和深度缓冲区类似，每次渲染前清楚模板缓冲区
glStencilMask(0xFF); // 每一位写入模板缓冲时都保持原样
glStencilMask(0x00); // 每一位在写入模板缓冲时都会变成0（禁用写入，只读）
// 似乎设为只读后，glClear也不起作用了，所以记得最后要设为0xFF
glStencilFunc(GLenum func, GLint ref, GLuint mask)
// func是通过测试的规则，和glDepthFunc类似；
// ref是参考值，模板缓冲会与ref进行比较；
// 参考值和模板值在比较时会先与mask进行与运算
```

```c++
glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
// 设定测试后对模板缓冲所做的行为，默认都是GL_KEEP
// sfail：模板测试失败时采取的行为。
// dpfail：模板测试通过，但深度测试失败时采取的行为。
// dppass：模板测试和深度测试都通过时采取的行为。

// GL_KEEP		保持当前储存的模板值
// GL_ZERO		将模板值设置为0
// GL_REPLACE	将模板值设置为glStencilFunc函数设置的ref值
// GL_INCR		如果模板值小于最大值则将模板值加1
// GL_INCR_WRAP	与GL_INCR一样，但如果模板值超过了最大值则归零
// GL_DECR		如果模板值大于最小值则将模板值减1
// GL_DECR_WRAP	与GL_DECR一样，但如果模板值小于0则将其设置为最大值
// GL_INVERT	按位翻转当前的模板缓冲值
```

### 混合

```c++
glEnable(GL_BLEND);
// 启用混合
glBlendFunc(GLenum sfactor, GLenum dfactor)
// 设定混合的因数，其中去覆盖的为源颜色
// GL_ZERO						因子等于0
// GL_ONE						因子等于1
// GL_SRC_COLOR					因子等于源颜色向量C¯source
// GL_ONE_MINUS_SRC_COLOR	 	 因子等于1−C¯source
// GL_DST_COLOR					因子等于目标颜色向量C¯destination
// GL_ONE_MINUS_DST_COLOR	 	 因子等于1−C¯destination
// GL_SRC_ALPHA					因子等于C¯source的alpha分量
// GL_ONE_MINUS_SRC_ALPHA		 因子等于1− C¯source的alpha分量
// GL_DST_ALPHA					因子等于C¯destination的alpha分量
// GL_ONE_MINUS_DST_ALPHA		 因子等于1− C¯destination的alpha分量
// GL_CONSTANT_COLOR			因子等于常数颜色向量C¯constant
// GL_ONE_MINUS_CONSTANT_COLOR	 因子等于1−C¯constant
// GL_CONSTANT_ALPHA			因子等于C¯constant的alpha分量
// GL_ONE_MINUS_CONSTANT_ALPHA	 因子等于1− C¯constant的alpha分量
// 其中Cconstant可以通过glBlendColor进行设置
glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
// 可以分别对RGB和A设置因数，上面这个个可以做到混合RGB、但A不变
glBlendEquation(GLenum mode)
// 改变混合的计算公式
// GL_FUNC_ADD：默认选项，将两个分量相加：C¯result=Src+Dst
// GL_FUNC_SUBTRACT：将两个分量相减： C¯result=Src−Dst
// GL_FUNC_REVERSE_SUBTRACT：将两个分量相减，但顺序相反：C¯result=Dst−Src
// GL_MIN：取两个分量中的最小值：C¯result=min(Dst,Src)
// GL_MAX：取两个分量中的最大值：C¯result=max(Dst,Src)
```

混合有一个问题，就是深度测试不会考虑alpha，如果一个物体A深度比物体B的深度浅，又先渲染了物体A，那么渲染物体B时因为深度测试的原因就不会渲染被A物体遮盖的部分

基础有两个解决方法，一个是渲染透明物体时将深度缓冲设为只读，一个是先渲染不透明物体，再将**透明物体排序**后由远到近进行渲染；不过前者的做法有可能会有问题，因为它不考虑深度测试，那么后渲染的透明物体就会混合之前的透明物体，如果透明物体**有不透明的部分**，这些部分就会被后渲染的透明物体混合，如下图中的十字框架

<img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251009104449242.png" alt="image-20251009104449242" style="zoom:50%;" />

另外，可以通过**OIT**方法，即不只保存最前面的片元，而是保存这个像素上所有的透明片元，最后再逐层叠加；还有一个**加权的OIT**，是将颜色和透明度根据一个与深度相关的函数作为权值累积，最后输出颜色累积除以透明度累积，是一种近似方法

### 面剔除

OpenGL根据顶点的环绕顺序判断正向反向面

```c++
glEnable(GL_CULL_FACE);
// 启用背面剔除
glCullFace(GL_FRONT);
// 规定剔除规则
// GL_BACK：只剔除背向面。
// GL_FRONT：只剔除正向面。
// GL_FRONT_AND_BACK：剔除正向面和背向面。
glFrontFace(GL_CCW);
// 规定哪个是正向面
// GL_CCW：逆时针；
// GL_CW：顺时针
```

### 帧缓冲

帧缓冲就是包含多个缓冲的区域，一个fbo对象中，包含了多个颜色缓冲、一个深度缓冲、一个模板缓冲

先前的帧缓冲使用的都是默认的GLFW创建的帧缓冲

```c++
unsigned int fbo;
glGenFramebuffers(1, &fbo);
// 创建帧缓冲对象，默认fbo为0
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
// 绑定
glDeleteFramebuffers(1, &fbo);
// 删除帧缓冲
```

#### 使用纹理作为帧缓冲附件

```c++
unsigned int colorBuffer;
glGenTextures(1, &colorBuffer);
glBindTexture(GL_TEXTURE_2D, colorBuffer);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
// 前半部分和一般的纹理对象创建一致
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
// 这里是选择24位存深度、8位存模板，实际上就是一个纹理
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilBuffer, 0);
```

#### 使用渲染缓冲对象作为帧缓冲附件

```c++
unsigned int rbo;
glGenRenderbuffers(1, &rbo);
// 创建渲染缓冲对象
glBindRenderbuffer(GL_RENDERBUFFER, rbo);
// 绑定
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
// 开辟内存
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
// 绑定到fbo中
// 和纹理作为附件相比，rbo无法被采样（用于内部深度、模板测试），且更快一些；需要采样纹理时，才用纹理作为附件
```

### 立方体贴图cube map

```c++
unsigned int textureID;
glGenTextures(1, &textureID);
glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
    );
// 立方体贴图需要指定6个面的贴图
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
// 立方体贴图的纹理坐标是三维的
```

在使用立方体贴图时，仍然需要投影矩阵和视图矩阵，但要去掉视图矩阵的平移

天空盒可以在一开始绘制，也可以最后绘制，但提前绘制后，由于天空盒不应该影响深度，所以之后还需要每个像素再过一遍，有一定的开销；最后绘制要将深度在顶点着色器中设为1，即`gl_Position = pos.xyww;`，并将深度通过规则设为小于等于GL_LEQUAL

```glsl
#version 440 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos; // 立方体采样uv直接一个向量（可以不归一化，但建议归一化），这里表示为TexCoords=aPos-vec2(0,0)
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_Position = gl_Position.xyww;
    // 将深度设为1
}
```

### 其他操作缓冲的方式

```c++
glBufferSubData(GL_ARRAY_BUFFER, 24, sizeof(data), &data);
// 该函数与glBufferData类似，不过可以设置第二个偏移量作为起始位置

void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
memcpy(ptr, data, sizeof(data));
glUnmapBuffer(GL_ARRAY_BUFFER);
// 指定一个指针，先指向当前的数组缓冲，再利用这个指针去填充数据
// 先前的顶点数据的存放方式是坐标、法线、uv紧密相邻，我们可以对每一个分别存储
glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), &positions);
glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), &normals);
glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(tex), &tex);
// 在使用时，初始位置设为整个块的起始位置即可
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);  
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(positions)));  
glVertexAttribPointer(
  2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(positions) + sizeof(normals)));
// 但是似乎先前的方法更好些，可能是因为如果数据太大，加载一个顶点需要offset较多？或者有可能由于空间局部性，先前交错的方法效率更好？

void glCopyBufferSubData(GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size);
// 复制缓冲
glBindBuffer(GL_COPY_READ_BUFFER, vbo1);
glBindBuffer(GL_COPY_WRITE_BUFFER, vbo2);
glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertexData));
// 对于同一类型缓冲，可以借助类似临时指针的方式进行指定
glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertexData));
// 这样也可以
```

### 高级GLSL

#### 顶点着色器

```glsl
// --- 输出变量 ---
gl_Position
// 这是裁剪空间的位置，需要在顶点着色器中进行赋值
gl_PointSize
// 需要glEnable(GL_PROGRAM_POINT_SIZE);
// 如果使用GL_POINTS进行DRAW，可以通过这个变量改变绘制的点的大小
    
// --- 输入变量 ---
gl_VertexID
// 存储当前顶点的ID，
// 使用glDrawElement时，存储正在绘制顶点的ID；
// 使用glDrawArray时，存储已经处理的顶点数量
```

#### 片元着色器

```glsl
// --- 输入变量 ---
gl_FragCoord
// xy是窗口的像素坐标，z是深度(0~1)
gl_FrontFacing
// 返回当前片段是正面还是反面，可以以此区分
    
// --- 输出变量 ---
gl_FragDepth
// 用来设置这个片元的深度值，如果没有写入值，就会自动取gl_FragCoord.z
// 如果用这个，会禁用Early Depth Testing
layout (depth_<condition>) out float gl_FragDepth;
// 在OpenGL4.2+，可以对深度条件进行声明，除了any之外的仍可以提前深度测试，<condition>可以为：
// any			默认值。提前深度测试是禁用的，你会损失很多性能
// greater		你只能让深度值比gl_FragCoord.z更大
// less			你只能让深度值比gl_FragCoord.z更小
// unchanged	如果你要写入gl_FragDepth，你将只能写入gl_FragCoord.z的值
```

#### 接口块

类似hlsl中的CBuffer

```glsl
out VS_OUT
{
    vec2 TexCoords;
} vs_out;

in VS_OUT // 块名要相同
{
    vec2 TexCoords;
} fs_in; // 实例名可以不同

```

#### Uniform缓冲对象

```glsl
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
// 一个可以被着色器访问的uniform缓冲区
// 可以在多个着色器中共享
// 对于普通的uniform变量，它是每个program之间独立的，一个program和vert和frag是共享uniform变量的，切换program后，uniform块会保留，下次激活program后可以继续用
// 对于uniform缓冲对象，则是GPU中一个类似于全局的对象，在c++端绑定后glsl就能找到对应的缓冲区了
unsigned int uboExampleBlock;
glGenBuffers(1, &uboExampleBlock);
glBindBuffer(GL_UNIFORM_BUFFER, uboExampleBlock);
glBufferData(GL_UNIFORM_BUFFER, 152, NULL, GL_STATIC_DRAW); 
// 分配152字节的内存，也可以直接sizeof对应的struct
glBindBuffer(GL_UNIFORM_BUFFER, 0);
// 这里的绑定是为了选择在c++端进行交互的目标uniform缓冲区
glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboExampleBlock); 
glBindBufferRange(GL_UNIFORM_BUFFER, 2, uboExampleBlock, 0, 152);
// uniform缓冲区通过这两个任一函数绑定index
```

```c++
layout(std140) uniform Lights { ... };
// 对于上面glsl中定义的缓冲区
unsigned int lights_index = glGetUniformBlockIndex(shaderA.ID, "Lights"); 
glUniformBlockBinding(shaderA.ID, lights_index, 2);
// 通过上述方法对某个shader的某个index（这里是2）的缓冲区设置对应

layout(std140, binding = 2) uniform Lights { ... };
// 但在4.2+版本，可以在glsl显式指定binding的index，在c++端设置这个index对应哪个uniform缓冲区就好了，不需要再用glUniformBlockBinding逐个shader绑定了
```

```c++
glBindBuffer(GL_UNIFORM_BUFFER, uboExampleBlock);
int b = true; // GLSL中的bool是4字节的，所以我们将它存为一个integer
glBufferSubData(GL_UNIFORM_BUFFER, 144, 4, &b); 
glBindBuffer(GL_UNIFORM_BUFFER, 0);
// 使用这种方法就可以只更新缓冲区中某个uniform变量了
```

#### 几何着色器

几何着色器是在顶点着色器后、片元着色器前的

```glsl
layout (points) in;
layout (line_strip, max_vertices = 2) out;
// 首先要规定输入和输出的图元类型
// --- 输入 ---
// points：绘制GL_POINTS图元时（1）。
// lines：绘制GL_LINES或GL_LINE_STRIP时（2）
// lines_adjacency：GL_LINES_ADJACENCY或GL_LINE_STRIP_ADJACENCY（4）
// triangles：GL_TRIANGLES、GL_TRIANGLE_STRIP或GL_TRIANGLE_FAN（3）
// triangles_adjacency：GL_TRIANGLES_ADJACENCY或GL_TRIANGLE_STRIP_ADJACENCY（6）
// --- 输出 ---
// points
// line_strip
// triangle_strip
in gl_Vertex
{
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
} gl_in[];
// glsl提供了类似于上述结构的内建变量gl_in
in VS_OUT {
    vec3 Normal;
    vec3 worldPos;
    vec2 TexCoords;
} vs_out[];
out vec3 Normal;
out vec3 worldPos;
out vec2 TexCoords;
// 接收来自顶点着色器的输出时，glsl规定如果输出是一个数据，必须定义接口块
void main() {
    for (int i = 0; i < 2; i++)
    {
        gl_Position = gl_in[i].gl_Position + vec4(explodeDir * 0.02, 0.0);

        Normal = vs_out[i].Normal; // 这里重复赋值是正确的，因为它赋值的不是同一个片元
        worldPos = vs_out[i].worldPos;
        TexCoords = vs_out[i].TexCoords;

        EmitVertex(); // 通过该函数来设置顶点的位置
    }

    EndPrimitive(); // 结束该图元的设置
}
// 这个示例通过传入point，输出了line
```

### 实例化

一个重要的性能瓶颈在于CPU到GPU总线的命令传输，如果一次CPU命令可以让GPU渲染多个物体，效率就会大大提高

```c++
gl_InstanceID
// glsl内建了一个变量用来存储实例ID，每渲染一个物体，ID会加一
glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
// 使用该函数可以批量绘制某个VAO对象，每次绘制的区别在于上面的实例ID
uniform vec2 offsets[100];
// 通过这个ID，可以使用数组设置每个实例的offset、贴图等
```

#### 实例化数组

但是上面的方法存在一个问题，在于uniform数据大小是有上限的

我们可以改变数据读取的规则，通过glVertexAttribDivisor函数规定每n个实例更新一次变量

```c++
layout (location = 1) in vec2 aOffset;

glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
glVertexAttribDivisor(1, 1);
// 该函数是指location为1的变量，在每一个实例都进行更新，所谓更新，就是指针读取数组数据
// 默认是glVertexAttribDivisor(1, 0);每个顶点更新一次变量，第二个变量就是每n个实例更新一次数据
// 注意这个pointer是VAO之间相互独立的，所以一个model中的两个mesh的是分不同批次的，即先draw所有model的mesh1，再draw所有model的mesh2

layout (location = 3) in mat4 model;
// OpenGL的顶点属性最大支持vec4，所以这个mat需要通过四个vec4来传递，如下：
GLsizei vec4Size = sizeof(glm::vec4);
glEnableVertexAttribArray(3); 
glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
glEnableVertexAttribArray(4); 
glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
glEnableVertexAttribArray(5); 
glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
glEnableVertexAttribArray(6); 
glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

glVertexAttribDivisor(3, 1);
glVertexAttribDivisor(4, 1);
glVertexAttribDivisor(5, 1);
glVertexAttribDivisor(6, 1);
```

### 抗锯齿（AA）

#### 超采样抗锯齿 SSAA

就是先用更高分辨率进行渲染，再下采样

#### 多重采样抗锯齿 MSAA

最初光栅化采样就是每个像素点进行一次采样，不考虑像素点覆盖的其他部分的颜色，只取采样点的颜色

MSAA就是一个像素进行多次有偏移的采样，最后插值采样结果

```c++
glfwWindowHint(GLFW_SAMPLES, 4);
// 采样点设为4，缓冲大小也变为4倍
glEnable(GL_MULTISAMPLE);
```

但如果我们使用帧缓冲，需要自己创建多重采样缓冲区

纹理附件：

```c++
glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
// 最后设为true，将会对每个纹素使用相同的样本位置以及相同数量的子采样点个数
glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
// 设为附件
```

渲染缓冲对象附件：

```c++
unsigned int rbo;
glGenRenderbuffers(1, &rbo);
glBindRenderbuffer(GL_RENDERBUFFER, rbo);
// 创建、绑定部分没啥变化
glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
// 开辟内存，这里和普通缓冲对象开辟内存函数不同
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
```

多重采样帧缓冲输出：

```c++
glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
// 使用glBindFrameBuffer将fbo绑定，是同时绑定了读取和绘制的帧缓冲目标；绘制到fbo对象后，再进行fullscreen绘制
// 我们可以通过GL_READ_FRAMEBUFFER与GL_DRAW_FRAMEBUFFER指定源帧缓冲和目标帧缓冲
glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
// glBlitFramebuffer会将源帧缓冲的一片区域，复制到目标帧缓冲的一片区域
// 这个过程中，如果目标是一个同样的多重采样帧缓冲对象，就会复制，如果是采样数不同的，会出错，如果是普通的帧缓冲对象，会自动进行平均操作再赋值
// 如果想对多重采样后的结果进行后处理之类的操作，需要先将fbo帧缓冲对象的内容通过glBlitFramebuffer输出到一个临时普通帧缓冲对象中进行读取
```

着色器访问多重缓冲纹理

```glsl
uniform sampler2DMS screenTextureMS;
// 别忘了rbo不能倍采样，这里是纹理作为附件才行
vec4 colorSample = texelFetch(screenTextureMS, TexCoords, 3);  // 第4个子样本
```

## 高级光照

### blinn-phong

phong模型会将夹角超过90度的光算为负数，取0，会有较为明显的分界线
blinn-phong模型使用半程向量，不会出现负数的情况

| phong效果：                                                  | blinn-phong效果：                                            |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| <img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251015173120173.png" alt="image-20251015173120173" style="zoom: 67%;" /> | <img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251015173145870.png" alt="image-20251015173145870" style="zoom:67%;" /> |

### gamma校正

首先，整体的过程大致是这样的：$L\ \underrightarrow{x^\gamma}\ sRGB\ \underrightarrow{x^{1/\gamma}}\ 线性空间\underrightarrow{x^\gamma}\ 电压值\ \underrightarrow{x^{1/\gamma}} 亮度\ \underrightarrow{x^{1/\gamma}}\ 感知亮度L'$，这里发现一开始的颜色和最后感知到的颜色并不同，但实际上，最开始的 L 一般是通过人去调整的，合并中间步骤，它就是一个 L 到 L' 的一个过程，而我们得到想要的颜色 L'，实际上储存的是一个 L，所以实际上我们想要的就是 L'

其中，电压到亮度的对应关系时类似于$L = V^\gamma$的关系；很多图像格式是通过$L'=L^{1/\gamma}$进行压缩的，其中gamma在sRGB中大概是2.2；人眼感知的亮度关系大约为$L'=L^{1/\gamma}$；在进行计算时应当使用线性数值，所以中间有一个线性空间，也就是在着色器中的表示

```c++
glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_SRGB, GL_UNSIGNED_BYTE, data);
// OpenGL中，通过GL_SRGB格式加载纹理后，会对图像进行一次SRGB->线性空间的伽马变换

glEnable(GL_FRAMEBUFFER_SRGB);
// 启用后，着色器输出的值会经过一次伽马逆变换
float gamma = 2.2;
vec3 diffuseColor = pow(texture(diffuse, texCoords).rgb, vec3(1/gamma));
// 也可以不启用GL_FRAMEBUFFER_SRGB，而是在着色器中进行变换
```

### 阴影映射

#### 原理

如果一个物体在光源看来，深度要比另一个物体的深度深，那么是没有直接光照的

那么我们可以渲染一张基于光源视角的深度图，通过比较摄像机能看到的点在光源视角的深度与对应位置的深度贴图的深度，就能知道有无直接光照了

#### 过程

这里我们使用帧缓冲的方式，让OpenGL自己绘制深度图（应该也可以自己用shader保存，但可读写的纹理还没接触到）

首先创建一个新的FBO，创建一个深度纹理对象作为该FBO的附件，顶点着色器的PV矩阵要传入光源视角的，片元着色器main函数为空即可（这里不设置颜色缓冲的附件，就不需要输出）

调用Draw函数后，就可以直接将阴影贴图拿来用了



随后，我们将纹理绑定到对应的纹理单元，在顶点着色器中也要传入光源的PV矩阵，计算光源的裁剪空间坐标，另外我们需要NDC坐标，需要使用透视除法（gl_Position不需要透视除法是因为赋值时OpenGL会自动进行透视除法）；再然后，就比较深度值判断是否有阴影了

<img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251017115211861.png" alt="image-20251017115211861" style="zoom: 50%;" /> 

#### 问题

##### 阴影失真

但是结果为什么出现了摩尔纹？哦，原来是因为采样的深度贴图是离散的，而非平滑过渡的；如图所示的阴影贴图，会有一部分是取到深度偏浅的部分的，就导致出现黑色的部分，这种情况叫做阴影失真

![img](https://learnopengl-cn.github.io/img/05/03/01/shadow_mapping_acne_diagram.png)

所以我们可以直接偏移一下，就能较为简单地解决问题

![img](https://learnopengl-cn.github.io/img/05/03/01/shadow_mapping_acne_bias.png)

##### 阴影悬浮

当我们的bias较大时，会出现另一个问题——阴影悬浮

<img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251017123038859.png" alt="image-20251017123038859" style="zoom: 50%;" /> 

该教程提供了使用正面剔除渲染深度图的方式，但是测试下来效果并不明显，更好的方法是让bias根据normal和lightDir进行调整

```glsl
float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.002);
```

##### 超采样

但现在又发现一个问题，在某些偏远的地方会有奇怪的阴影

<img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251017131510648.png" alt="image-20251017131510648" style="zoom: 50%;" /> 

这是因为纹理采样的规则我们设置为了Repeat，导致了超采样；我们可以让超出的边界部分深度设为1，这样就不会有阴影产生

```c++
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 边界视为“无限远”（不在阴影中）
glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
```

##### 阴影僵硬

我们发现，阴影的边缘太实了，对于平行光来说，不需要考虑半影，但我们觉得还是不好看

先说说教程中的方法，就是通过一个均值滤波来模糊边缘

<img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251017132142035.png" alt="image-20251017132142035" style="zoom: 50%;" /> 

我自己则是考虑可以模拟离得越远阴影越淡的情况，就让阴影和距离成exp的衰减关系

不过有一个问题是由于距离储存的是正面的，我实际想要背面的闭塞效果，于是我重新拾起了之前的正面剔除，计算背面和对应点的深度差了

<img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251017132634627.png" alt="image-20251017132634627" style="zoom: 50%;" /> 

不过这样做有一些限制，它只能做第一次照射物体的阴影，后面的阴影就会变淡；而且因为阴影贴图存储的是深度最浅的那个，导致后面的阴影都是以最浅的那个作为标准；而且使用正面剔除总会有点问题，总的来说只是一个特化的一个方法，没有普遍意义

<img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251017133217846.png" alt="image-20251017133217846" style="zoom:80%;" /> 

```glsl
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
//    float bias = 0.005;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.005);
//    float bias = -max(0.1 * (1.0 - dot(normal, lightDir)), 0.05); // 正面剔除使用
//    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
//    float depthDifference = currentDepth - bias - closestDepth;
//    float shadow = depthDifference > 0? exp(-depthDifference / 5) : 0.0;

    float shadow = 0.0;
    // PCF百分比渐进滤波，也就是模糊操作
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
//            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            float depthDifference = currentDepth - bias - pcfDepth;
            shadow += depthDifference > 0? exp(-depthDifference / 5) : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0 || projCoords.z < 0.0) return 0.0; // 超出光源视锥外

    return shadow;
}
```

### 万向阴影贴图

在阴影贴图超采样问题中我们就能发现，阴影贴图有个限制就是第一它是单向的，第二覆盖的范围有限；对于单向光源稍微调整一下位置或多次进行渲染可能还好，但点光源就不同了，如何有效的覆盖能看到的周围所有的深度呢？这种效果在之前就有类似的，那就是skybox，也就是说，我们也可以用一个立方体贴图进行存储，也就是万向阴影贴图

对于立方体贴图的渲染，不需要对立方体六个方向的贴图分别绑定帧缓冲附件，再做六次渲染（虽然这样做应该也是可以的）

```c++
glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
for (GLuint i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
// 创建立方体贴图
glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
// 直接绑定立方体贴图作为深度缓冲附件
glDrawBuffer(GL_NONE);
glReadBuffer(GL_NONE);
// 忽视颜色缓冲
glBindFramebuffer(GL_FRAMEBUFFER, 0);
```

然后需要创建六个方向的光照VP矩阵，至于如何规定向立方体某个面进行绘制，glsl 中有一个内建变量`gl_Layer`，用来控制几何着色器的图形送到立方体贴图的哪个面，不进行赋值就会默认输出到layer 0的那个图层中

```glsl
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // 选择输出的layer
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            // 这里shadowMatrices是不同面的VP矩阵，因此顶点着色器中只需要乘上model矩阵就可以了
            EmitVertex();
        }    
        EndPrimitive();
    }
}
```

最后阴影计算为

```glsl
float ShadowCalculation(vec3 worldPos, vec3 normal, vec3 lightPos)
{
    vec3 lightToFrag = worldPos - lightPos;
//    float closestDepth = texture(shadowMap, normalize(lightToFrag)).r * farPlane;
    float currentDepth = length(lightToFrag);
    float bias = 0.005;
//    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.005);
//    float depthDifference = currentDepth - bias - closestDepth;
//    float shadow = depthDifference > 0 ? 1.0 : 0.0;
//    float shadow = depthDifference > 0? exp(-depthDifference / 5) : 0.0;

    float shadow = 0.0;
    float offset = 0.01;
    for (int i = 0; i < 20; ++i)
    {
        vec3 randomOffset = vec3(GetRandom(worldPos, i), GetRandom(worldPos, i + 1), GetRandom(worldPos, i + 2)) * offset;
        vec3 sampleDir = normalize(lightToFrag + randomOffset);
        float closestDepth = texture(shadowMap, sampleDir).r * farPlane;
        float depthDifference = currentDepth - bias - closestDepth;
        shadow += depthDifference > 0? exp(-depthDifference / 5) : 0.0;
    }
    shadow /= 20.0;

    return shadow;
}
```

### 法线贴图

简单来说就是用贴图描述法线分布，这对于微表面模型也很有用

使用法线贴图存储，由于RGB或其他颜色的限制，它的值会是在 [0,1]，因此，想要使用法线还需要通过`norm*2-1`映射到 [-1,1]

最开始，法线贴图存储的值都是世界坐标的方向，直接应用就好，但在模型旋转后，这些方向就需要额外进行变换，因此后面就都采用了相对方向，存储在切线空间中；凹凸贴图也需要TBN，PBR也基本基于局部切线坐标系，因此更加合适

现在，法线贴图大多朝向的面的法线方向规定为 (0,0,1)，所以都偏蓝，但这也意味着如果模型的顶点法线方向并非朝向正 z 轴，效果就会很奇怪，因为这里它是作为法线的相对方向，而非绝对方向（或者说是在世界坐标下的方向）去计算的

![img](https://learnopengl-cn.github.io/img/05/04/normal_mapping_ground_normals.png)

#### TBN

因此，为了解决这个问题，我们需要将法线变换到切线空间，可以理解为将法线贴图旋转到朝向方向和表面法线方向一致，不过也要考虑切线和副切线的方向

为此，我们建立切线空间，由法线、切线、副切线三个方向组成，则有`mat3 TBN = mat3(T,B,N)`，可以理解为将原来 x y z 的三个单位向量改为了 T B N

关于**如何使用TBN**，可以将法线从法线贴图的**切线空间变换到世界坐标**，也可以通过逆矩阵将**世界空间变换到切线坐标**中

前者需要变换的目标仅有法线，而后者需要变换需要用到（这里根据是否要和法线进行运算判断）的诸如光线方向、视线方向、坐标位置等

但是后者可以在顶点着色器中就提前计算，而顶点着色器的运行次数一般要低于片元着色器，就**效率上**而言，后者是更好的

但咱们在这里就先用前者简单的实现就好了（笑）

#### 格拉姆-施密特正交化 Gram-Schmidt process

为了避免顶点的法线和切线可能不正交，需要重正交化

一个做法就是`T = normalize(T - dot(T, N)*N)`，就是利用了点乘投影映射的原理，可以画画图看看

### 视差贴图 Parallax Mapping

视差贴图是凹凸贴图的一种，他不是将顶点进行位置的变换，而是在片元着色器中改变纹理采样的坐标，从而使得看上去有了凹凸的效果

#### 思路

首先，设想一个平面，我们希望它看起来都向上移动了一段距离，如图所示，A点应该对应到B点，那么如何让他看起来在B点呢？直接使用B点的纹理坐标就好了。

然后怎么算呢？这就是简单的几何了， `viewDir.xy` 也就是向量ED，用一个“相似三角形”就能得到向量AB了，即$\frac{AB}{ED} = \frac{CE}{CA},\ AB = \frac{CE*ED}{CA}$，最后得到`offset=viewDir.xy/viewDir.z*H(A)`

<img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251021001110503.png" alt="image-20251021001110503" style="zoom:50%;" />

但是，很多时候我们需要处理边界问题，对于下图的场景下，将红线视为我们想要看到的效果，我们看向了A点，而我们想要它看上去是在B点，我们可以直接套用上面的公式，但是会有一定的偏移

![img](https://learnopengl-cn.github.io/img/05/05/parallax_mapping_scaled_height.png)

```glsl
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) // texCoords就是A的纹理坐标
{
    float height = texture(material.texture_displacement1, texCoords).r; // 取H(A)
    float heightScale = 0.1; 
    // 这个是为了控制height到纹理坐标位移的度量比例
    // 如果是1，那么就说明height=1的话，就相当于贴图的长宽的距离了
    vec2 offset = viewDir.xy / viewDir.z * height * heightScale;
    return texCoords - offset;
}
```

我们现在得到的效果看起来已经很立体了，但在视角倾斜角度较大时仍会穿帮

|                            好角度                            |                大角度，且不discard超采样部分                 |                 大角度，并discard超采样部分                  |
| :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| ![image-20251021005359211](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251021005359211.png) | ![image-20251021005538142](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251021005538142.png) | ![image-20251021005611957](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251021005611957.png) |

#### 陡峭视差映射 Steep Parallax Mapping

就是我们对陡峭的部分，不通过之前的方法，而是直接找（不管是遍历还是二分）离B点最近的取值

```glsl
float layerNum = mix(10.0, 32.0, abs(dot(vec3(0.0, 0.0, 1.0), normalize(viewDir)))); // 遍历步数
float layerDepth = (1-height) / layerNum; // 每一步的深度
float currentLayerDepth = 0.0;
vec2 P = viewDir.xy / viewDir.z * 0.1; // 每单位深度对应的offset
vec2 deltaTexCoords = P * layerDepth; // 每一步对应的offset
vec2 currentTexCoords = texCoords;
float currentDepthMapValue = texture(material.texture_displacement1, currentTexCoords).r;

while (currentLayerDepth < currentDepthMapValue)
{
    currentTexCoords -= deltaTexCoords;
    currentDepthMapValue = texture(material.texture_displacement1, currentTexCoords).r;
    currentLayerDepth += layerDepth;
}

return currentTexCoords;
```

#### 视差遮蔽映射 Parallax Occlusion Mapping

直接做陡峭视差映射结果看起来层层叠叠的，是因为这种方法是离散的结果，可以通过线性插值缓解

```glsl
vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
float afterDepth  = currentDepthMapValue - currentLayerDepth;
float beforeDepth = texture(material.texture_displacement1, prevTexCoords).r - (currentLayerDepth - layerDepth);
float weight = afterDepth / (afterDepth - beforeDepth);
currentTexCoords = mix(currentTexCoords, prevTexCoords, weight);
```

#### 二分...?

但这种结果面对较大的step时仍收效甚微，我们可以单纯提高`layerNum`增加精度，不过我们还有一个方法：二分

可以看到确实平滑了许多，但是又出现新问题：拐角处空了一块，这可能是因为它在某些角度上的深度变化并非是单调的，因此并不能作为一个正确的方法TAT

```glsl
float l = 0, r = height;
for (int i = 0; i < 10; i++)
{
    currentLayerDepth = (l + r) / 2.0;
    currentTexCoords = texCoords - P * currentLayerDepth;
    currentDepthMapValue = texture(material.texture_displacement1, currentTexCoords).r;
    if (currentDepthMapValue > currentLayerDepth)
        l = currentLayerDepth;
    else
        r = currentLayerDepth;
}
```

| 陡峭视差映射                                                 | 视差遮蔽映射                                                 | 使用更细粒度或范围的`layerDepth`后的视差遮蔽映射             | 使用二分                                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| ![image-20251021020322841](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251021020322841.png) | ![image-20251021013801264](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251021013801264.png) | ![image-20251021014832902](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251021014832902.png) | ![image-20251021015806382](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251021015806382.png) |

### HDR

HDR就是让存储的光照值能够大于 1，之后再映射到 [0,1] 的LDR范围

OpenGL中，对于GL_RGB格式的颜色缓冲，赋值前会自动约束到 [0,1]，但我们可以使用GL_FLOAT数据类型的颜色缓冲，这就是浮点帧缓冲

```c++
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
```

然后是tonemapping，通过一些映射函数将HDR映射到LDR

```glsl
// Reinhard色调映射
vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

// 曝光色调映射
vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

// Uncharted 2 色调映射 (John Hable)
// 以其在《神秘海域2》中的使用而闻名
vec3 F(vec3 x)
{
    const float A = 0.22f;
    const float B = 0.30f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.01f;
    const float F = 0.30f;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2ToneMapping(vec3 color)
{
    const float WHITE = 11.2f;
    return F(color) / F(vec3(WHITE));
}

// ACES Filmic 色调映射 (近似版，由 Krzysztof Narkowicz 提供)
// 能产生非常自然的对比度和色彩
vec3 acesFilmicToneMapping(vec3 color)
{
    color *= 0.6; // 降低输入，适应ACES曲线
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}
```

这些函数大都是接近于S形曲线，一个是人眼对暗部感知敏感，给暗部更大的映射范围有助于观感；一个是在高光处能有更柔和的过渡

### 泛光 bloom

思路是对亮度大于 1 的部分做模糊操作

#### 多渲染目标

实现上，使用MRT（多渲染目标）的方法，使用片元着色器渲染两个纹理，一个是普通的光照，另一个是保留前者亮度大于 1 的部分的结果

```glsl
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

// main函数中
FragColor = vec4(result, 1.0);

float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
if(brightness > 1.0)
    BrightColor = vec4(FragColor.rgb, 1.0);
else
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
```

绑定多纹理：

```c++
int colorBufferCount = 2;
glGenTextures(colorBufferCount, colorBufferTex);
for (int i = 0; i < colorBufferCount; i++)
{
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBufferTex[i], 0);
    // 这里使用GL_COLOR_ATTACHMENT0 + i指定多个颜色缓冲附件
}

GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
glDrawBuffers(2, attachments);
// 设置绘制的目标有哪些（只是设置附件不会让OpenGL绘制，必须指定）
```

然后，对后者的结果进行模糊操作，这里有两个思路

- 一个是通过传统渲染的流程，每次渲染对当前纹理进行一次模糊，进行若干次渲染（多次模糊），另外，由于不能同时对一张纹理进行读写，所以需要依靠两个纹理依次作为输入和输出
- 另一个是通过compute shader，为了避免对同一空间进行同时读写，要依靠barrier保证线程都写完这一次的模糊

#### ping pong缓冲区

```c++
void camera::genPingPongFrameBuffer() // 两个颜色纹理分别绑定到两个fbo上
{
    glGenFramebuffers(2, pingPongFBO);
    glGenTextures(2, pingPongColorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[i]);

        glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, opengl::window::getWidth(), opengl::window::getHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongColorBuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "ERROR::FRAMEBUFFER:: PingPong Framebuffer is not complete!" << std::endl;
    }
}

void camera::drawPingPongFrameBuffer() // 轮流作为纹理的输入方
{
    bool begin = true, horizontal = true;
    blurShader->use();
    blurShader->setInt("image", 0);
    for (int i = 0; i < 10; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[horizontal]);
        blurShader->setBool("horizontal", horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, begin ? colorBufferTex[1] : pingPongColorBuffers[!horizontal]);

        glBindVertexArray(screenVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        horizontal = !horizontal;
        begin = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
```

#### compute着色器

compute着色器的编译和其他的着色器基本一致

```c++
glDispatchCompute(
    (width + 15) / 16,  // 工作组X数量
    (height + 15) / 16, // 工作组Y数量
    1                   // 工作组Z数量
);

glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
// 等待compute结束
```

```glsl
layout (local_size_x = 16, local_size_y = 16) in;
// 决定每组的大小

gl_GlobalInvocationID
// 这个代表当前线程的全局索引坐标，由dispatch传入的组数量和layout声明的组大小决定其范围
gl_LocalInvocationID
// 这个代表当前线程在所属group中的相对位置
```

compute的思路是：共享内存设为两份，`sharedData[2][26][26]`，填充周围的边界，进行滤波

但这样做有很多问题：第一个是每个线程都会对边界进行填充，导致很多无效计算；第二个是尽管填充了边界，但进行多次模糊时，它只是对这部分区域进行模糊，结果拼接时，每个group的边界处有较明显的不同

因此，这里只当作是对compute着色器的一个初步了解和实践

|                           不加 HDR                           |                            加 HDR                            |
| :----------------------------------------------------------: | :----------------------------------------------------------: |
| ![image-20251023203201233](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251023203201233.png) | ![image-20251023203217791](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251023203217791.png) |



### 延迟着色器法

传统前向着色方法需要对每个物体依次计算光照，也就是`objects*pixels*lights`的计算量

延迟着色器法，是先通过渲染`GBuffer`，将光照计算所需要的反射率、法线、世界坐标等存储在一个纹理中，再对屏幕四边形进行光照计算，这样就只需要`pixels*lights`的计算量甚至更少

#### GBuffer

首先是`GBuffer`，就是摄像机能看到的所有片段的信息，通过MRT可以在一次渲染调用将一批实例化物体的信息绘制出来

#### 延迟渲染光照处理

得到了`GBuffer`后，就是将其传入着色器中，采样进行光照计算了

#### 延迟渲染+正向渲染

如果我们想要使用正向渲染绘制一些不方便在延迟渲染中绘制的物体，直接绘制会将延迟渲染结果覆盖

我们可以借用延迟渲染`GBuffer`得到的深度贴图，直接写入到默认帧缓冲的深度贴图中

```c++
glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO);
glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFBO); // 写入到相机帧缓冲
glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
// 注：这里需要两个帧缓冲的深度附件的格式一致，如都是GL_DEPTH_STENCIL_ATTACHMENT或都是GL_DEPTH_ATTACHMENT，否则会将最后一维模板缓冲给复制过去，导致深度测试结果有误
```

|                           延迟渲染                           |                           前向渲染                           |                      延迟渲染+前向渲染                       |
| :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| ![image-20251023202543086](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251023202543086.png) | ![image-20251023202613162](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251023202613162.png) | <img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251023202202553.png" alt="image-20251023202202553"  /> |

另外确实延迟渲染的速度要更快一些，前向渲染FPS150左右时，延迟渲染有FPS180+

#### 光体积优化

所谓光体积，就是我们认为一个光源所能影响的区域，在这个区域外就不做计算

我们可以单纯利用衰减方程计算边界距离，但这种`if-else`结构在GPU中并不讨喜，由于GPU高度并行化，它希望并行计算的代码是完全一致的，通常一个着色器运行会执行它的所有分支语句保证这一点，因此这部分的运算并没有省去

对于这个问题，我们可以使用一个球去模拟体积光的范围，可以通过模板缓冲区来设置延迟渲染的范围，并用延迟渲染光照的着色器去计算，就像是画面被印在了球面上，有点像高斯泼溅的感觉

1. 创建光体积FBO，绑定模板缓冲附件，设置各个球的模型矩阵
2. 渲染得到模板缓冲，使用 blit 复制到延迟渲染FBO的模板缓冲附件
3. 进行延迟渲染计算，这样就只显示在体积光范围内的物体了

### 屏幕环境光遮蔽 SSAO

首先现实中会有这么一种现象：相互靠近的物体间的光照往往会更暗一些（在光追中就是能打到的光线少），这种效果就叫做AO

为了在传统光栅化中去模拟这个效果，提出了SSAO，即基于屏幕空间来进行AO的判定

我们在屏幕上进行圆形（空间中球形）采样，统计比该点的深度更浅的点，并进行加权求值，根据结果影响该点的明暗

![img](https://learnopengl-cn.github.io/img/05/09/ssao_overview.png)

这个思想主要在于认为AO发生的地方主要在于夹角之中，因此判断是否存在深度的变化，并且由于角落中的光照方向一般和视线方向相似，所以这个方法在很多情况下都比较正确

不过，圆形采样存在问题，即平面也会有周围采样的贡献，导致整个画面看起来都是灰灰的，因此我们基于法向量只采样半球形的区域，这对夹角很敏感

![img](https://learnopengl-cn.github.io/img/05/09/ssao_hemisphere.png)

另外，由于被遮挡的物体的深度也是更深的，会导致背景也会因此出现AO的效果，导致物体蒙上一圈黑影；我们可以进行范围检测，

```glsl
float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth)); // 当第三个参数落在0~1时，返回值为0~1
occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;    
// 这里使用smoothstep是考虑直接硬截断会有一圈较为明显的黑边
```

|                           SSAO关闭                           | SSAO开启+范围检测不开启                                      |                    SSAO开启+范围检测开启                     |
| :----------------------------------------------------------: | ------------------------------------------------------------ | :----------------------------------------------------------: |
| ![image-20251025110615037](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251025110615037.png) | <img src="C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251025124910005.png" alt="image-20251025124910005" style="zoom:138%;" /> | ![image-20251025110610509](C:\Users\WQC20\AppData\Roaming\Typora\typora-user-images\image-20251025110610509.png) |

由于我们是通过随机采样，并且采样纹理是repeat的，所以它的噪声会显得很规律，我们可以滤波去噪

## PBR

### 理论

本来想写“见GAMES101”，但似乎还是要通一遍

#### 反射方程

$L_o(p,\omega_o)=\int_\Omega f_r(p,\omega_i\omega_o)L_i(p,\omega_i)n\cdot\omega_i d\omega_i$ 这个式子列出了对各个立体角方向入射的光进行积分后，对某个出射立体角的光的贡献

其中的 f 函数就是BRDF，是对 p 点，某个入射立体角到某个出射立体角，会有多少光被反射，而 L 函数就是某个入射立体角打到 p 点的辐射度

#### Cook-Torrance BRDF

这个BRDF模型的公式为$f_r=k_df_{lambert}+k_sf_{cook-torrance}$，也就是漫反射（或者说是折射）和反射

对于$f_{lambert}$，lambert漫反射效果就不错，其等于$\frac{c}{\pi}$，其中 c 时表面颜色，pi 是对于半球的积分（带cos），如果入射光恒为1，结果为pi，我们希望BRDF的结果是 <= 1 的，因此我们除以一个pi，使得其满足比率

对于$f_{cook-torrance}$，其等于$\frac{DFG}{4(\omega_o\cdot n)(\omega_i\cdot n)}$，它主要部分包括了法线分布函数 D、几何函数 G、菲涅尔项 F

##### 法线分布函数

其近似表示了微表面法线中与某个半程向量H取向一致的比率

对于Trowbridge-Reitz GGX，其表示为$NDF_{GGXTR}(n,h,\alpha)=\frac{\alpha^2}{\pi((n\cdot h)^2(\alpha^2-1)+1)^2}$，其中$\alpha$表示粗糙度

法线分布函数随不同粗糙度的效果：
![img](https://learnopengl-cn.github.io/img/07/01/ndf.png)

##### 几何函数

其近似表示了微表面间遮蔽的比率

对于Schlick-GGX，其表示为$G_{SchilickGG(n,v,k)=\frac{n\cdot v}{(n\cdot v)(1k)+k}}$其中k是与粗糙度相关的函数，对于直接光照或是对于IBL，$k_{direct}=\frac{(\alpha+1)^2}{8}，k_{IBL}=\frac{\alpha^2}{2}$

几何函数效果：
![img](https://learnopengl-cn.github.io/img/07/01/geometry.png)

##### 菲涅尔

菲涅尔表示的是被反射的光占总光线的比率，对应的其他光线（如只考虑漫反射）就是一减去该比率

对于Fresnel-Schlick，其表示为$F_{Schilick}(h,v,F_0)=F_0+(1-F_0)*(1-(h\cdot v))^5)$，也就是夹角越大反射越强，$F_0$表示角度为0时的菲涅尔值

引入粗糙度，有$F_{Schilick}(HdotV,F_0, \alpha)=F_0+(max(1-\alpha,\ F_0)-F_0) * (1-HdotV)^5$，粗糙度越大，角度的影响越小

菲涅尔效果：
![img](https://learnopengl-cn.github.io/img/07/01/fresnel.png)

### 光照

将上面的公式代入到最开始的反射方程中，得到$L_o(p,\omega_o)=\int_\Omega(k_d\frac{c}{\pi}+k_s\frac{DFG}{4(\omega_o\cdot n)(\omega_i\cdot n)})L_i(p,\omega_i)n\cdot\omega_id\omega_i$

```glsl
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
```

顺带一提，这里没用 obj 模型，而是用了 glb 模型，是专门支持 pbr 的模型格式

这种模型的贴图是内嵌在模型文件中的，所以要额外添加内嵌加载贴图的处理，而且blender导出的glb模型将roughness和metallic合并为一个roughness，分别存储于 g b 分量

![image-20251027163118418](E:\code\Learn\opengl\assets\image-20251027163118418.png) 

### IBL

IBL即Image based lighting，通过一个环境贴图捕捉周围的环境光照，这个贴图可以是天空盒、探针等

#### 从三维uv到等距柱状投影uv

一个贴图如何得到？它可以是立方体贴图，我们沿用之前天空盒的采样即可。不过很多是等距柱状投影（参考经纬球uv展开的等距投影）得到的一张贴图，我们可以就将方向向量映射到这个贴图的坐；也可以将其**先绘制到立方体贴图**上进行采样（这样做每次采样不需要额外转换uv，计算开销更小）

```glsl
const vec2 invAtan = vec2(1/(2*pi), 1/pi);
vec2 SampleSphericalMap(vec3 v) // 将三维uv映射到等距柱状投影图的uv
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    // uv.x \in [-PI, PI], uv.y \in [-PI/2, PI/2]
    // 将其映射到 [0,1] 范围内
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
```

关于这个关系的推导，首先我们知道一个单位球的由经纬度表示为$\begin{cases}x=\cos\theta\cos\phi\\y=\sin\theta\\z=\cos\theta\sin\phi\end{cases}$，而等距柱状投影就是 x 与经度线性相关，y 与维度线性相关，也就是我们先将 XYZ 映射为经纬度后，再将经纬度映射为 [0, 1] 就可以了

关于其中的atan，传统我们求取反正切是通过 y/x 求，但这样做只能得到 [-2/pi, 2/pi]范围的角度，想要得到 (-pi, pi] 的范围的角，需要额外给x与y的符号，也就是传入这两个值

#### 辐照度图

那么一个环境贴图如何贡献光照？由于对于一个点，它法向的半球方向的所有方向都有可能贡献光照，我们只需要取得这些方向的所有环境贴图的值就好，但是实时进行采样的操作实在是太费性能，我们可以对贴图进行**预卷积**得到周围光照的贡献（辐照度图），在采样时只取**法向方向**的卷积后的辐射度贡献即可

这样做看起来是正确的，在公式上呢？我们发现反射方程是可以拆成
$$
L_o(p,\omega_o)=\int_\Omega(k_d\frac{c}{\pi})L_i(p,\omega_i)n\cdot\omega_id\omega_i+\int_\Omega(k_s\frac{DFG}{4(\omega_o\cdot n)(\omega_i\cdot n)})L_i(p,\omega_i)n\cdot\omega_id\omega_i
$$
其中前一项是与半程向量无关的，只与入射角度、出射角度、法线等有关，于是我们可以将前一项预先进行积分，得到$L_{diffuse}(p,\omega_o,\omega_i)$

对于积分公式，就是通过球面积分，$\text{立体角}d\Omega=sin(\theta)d\Phi d\theta$，$L_o(p,\Phi_o,\theta_o)=k_d\int_{\Omega=0}^{4\pi} L_i(p,\Phi_i,\theta_i)cos(\theta)d\Omega$，代入得到$L_o(p,\Phi_o,\theta_o)=k_d \int_{\Phi=0}^{2\pi}\int_{\theta=0}^{\pi/2}L_i(p,\Phi_i,\theta_i)cos(\theta)sin(\theta)d\Phi d\theta$，这里的结果是不考虑出射参数的影响，因为是漫反射

![img](https://learnopengl-cn.github.io/img/07/03/01/ibl_spherical_integrate.png)

```glsl
for (float phi = 0; phi < 2 * PI; phi += sampleDelta)
{
    for (float theta = 0; theta < 0.5 * PI; theta += sampleDelta)
    {
        vec3 offset = vec3(sin(theta)*vec2(cos(phi), sin(phi)), cos(theta));
        vec3 sampleVec = TBN * offset;

        irradiance += textureLod(cubeMap, sampleVec, 0).rgb * cos(theta) * sin(theta);
        sampleCount++;
    }
}
irradiance = irradiance * PI / sampleCount;
```

这里的积分是离散版本的（黎曼和），也就是$\sum_{i=0}^{n_{\theta}-1}\sum_{j=0}^{n_{\phi}-1}f(\theta_i,\phi_j)cos\theta sin\theta \Delta\theta\Delta\phi$，其中$\Delta\theta=\frac{\pi/2}{n_\theta},\ \Delta\phi=\frac{2\pi}{n_\phi}$，展开就是$\frac{pi*pi}{n_\theta n_\phi}$，和之前的 c/pi 相乘得到$\frac{c*pi}{n_\theta n_\phi}$，不过这里不计算 c，因为预卷积是不知道这个的

|                          使用IBL前                           |                          使用IBL后                           |              仅IBL（有HDR色调映射和gamma校正）               |
| :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| ![image-20251028130901665](E:\code\Learn\opengl\assets\image-20251028130901665.png) | ![image-20251028130917781](E:\code\Learn\opengl\assets\image-20251028130917781.png) | ![image-20251028131012451](E:\code\Learn\opengl\assets\image-20251028131012451.png) |

这里不知道是HDR贴图的问题还是采样数量不够的问题，对于之前的HDR场景，它的卷积结果有很多的光斑，不像是模糊的样子，而上面这个结果就很模糊，亮度看起来也很合适

对于这个光斑，learnOpenGL中提出了一个方法（虽然是在反射IBL中提出的），即生成立方体环境贴图的mipmap，求预滤波的HDR环境贴图时，根据粗糙度改变采样的level，就可以让结果光斑不集中了

#### 镜面反射IBL

首先我们将上述反射方程剩下的的镜面反射项拎出来，可以写作$\int_\Omega L_i(p,\omega_i)d\omega_i*\int_\Omega f_r(p,\omega_i,\omega_o)n\cdot \omega_id\omega_i$

我们将这两项分别预计算，这种方法被称为分割求和近似法

##### 预滤波环境贴图

对于第一部分，我们可以采用类似辐照图贴图的思路，得到的结果被称作预滤波环境贴图

不一样的是采样方式，首先，我们不知道视线方向，这里我们选择将采样方向作为作为反射方向，也是视线方向

```glsl
vec3 N = normalize(localPos);    
vec3 R = N;
vec3 V = R;
```

这种做法有个缺点，我们是基于V=N=R的前提得到的朝向某个方向的点，它得到的入射光照的结果，但这些采样的方向是根据这个方向采样的，这导致掠角时，我们采样的结果包含了到比这个点所在平面更低的方向来的光

我们使用蒙特卡洛积分，当我们使用完全随机的采样，这时的蒙特卡罗是无偏的，当我们集中于特定的值或方向，是有偏的，后者可以快速得到较为精准的值，但有可能收敛不到正确的值

采样的过程为：生成随机数序列、生成采样样本向量、采样并积分

当我们使用一个低差异序列生成样本向量时，叫做拟蒙特卡罗积分，它仍然是无偏的，但在于低差异序列是分布均匀的，所以能更快收敛

这里我们使用 Hammersley 序列，并进行GGX重要性采样：

```glsl
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// 生成 Hammersley 序列

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
	// 这里的公式是通过GGX分布函数推导，我们要在这个分布中采样一个H，希望它符合这个分布
    // 我们将这个分布的xyz改为用球坐标进行表示，最终就能得到下面costheta的关系，其中Xi是随机数
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	// 将采到的方位角转换为切线空间下的xyz坐标
    vec3 H = vec3(sinTheta*vec2(cos(phi), sin(phi)), cosTheta);

    // 切线空间变换到世界空间
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}
```

采样、积分过程如下：

```glsl
vec3 prefilteredColor = vec3(0.0);
for(uint i = 0u; i < SAMPLE_COUNT; ++i)
{
    vec2 Xi = Hammersley(i, SAMPLE_COUNT);
    vec3 H  = ImportanceSampleGGX(Xi, N, roughness); // 重要性采样一个半程向量
    vec3 L  = normalize(2.0 * dot(V, H) * H - V);

    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    float D   = DistributionGGX(NdotH, roughness); // H方向的概率密度
    float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;
    // 这个pdf是对于L方向的概率密度，且有p(l)=p(h)*(n*h)/(4*(v*h))

    float resolution = 512.0; // resolution of source cubemap (per face)
    float saTexel  = 4.0 * PI / (6.0 * resolution * resolution); // 立方体贴图每个 texel 覆盖的平均立体角
    float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001); // 每个样本覆盖的立体角
    float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
    // 当saSample > saTexel时，说明一个采样覆盖了多个texel，需要采样更高mip级别的贴图

    float NdotL = max(dot(N, L), 0.0);
    if(NdotL > 0.0)
    {
        prefilteredColor += textureLod(cubeMap, L, mipLevel).rgb * NdotL;
        totalWeight      += NdotL;
    }
}

prefilteredColor = prefilteredColor / totalWeight;
```

##### 预计算BRDF

第二部分中，所有的计算实际上只需要的外部输入只有 NdotV 和roughness，而且这两个变量的取值都是[0, 1]，因此，我们可以将他们对应的结果存储到一张纹理中

我们将菲涅尔项F从BRDF的表达式中移出，并展开为Fresnel-Schlick 近似公式，最终我们就能得到
$$
\int_\Omega f_r(p,\omega_i,\omega_o)n\cdot \omega_id\omega_i = F_0\int_\Omega f_r(p,\omega_i,\omega_o)(1-(1-\omega_o\cdot h)^5)n\cdot \omega_id\omega_i+\int_\Omega f_r(p,\omega_i,\omega_o)(1-\omega_o\cdot h)^5n\cdot \omega_id\omega_i
$$
其中的$f_r$项如上面所说，其中的菲涅尔项F已经被移出，不做计算了；另外，我们重要性采样的是h，所以对于L的pdf，我们需要进行一个转换$ pdf = (D * NdotH / (4.0 * HdotV))$，除以这个概率密度后，就得到如代码所示，其中$G_{Vis}=G(\omega_i,\omega_o)\frac{\omega_o\cdot h}{(n\cdot h)(n\cdot \omega_o)}$

```glsl
vec2 IntegrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0; // A和B分别是拆分后的两项

    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV); // 去掉F的BRDF函数，消去了n*w_i
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}
```

|                            仅光照                            |                          漫反射IBL                           |
| :----------------------------------------------------------: | :----------------------------------------------------------: |
| ![image-20251028161859120](E:\code\Learn\opengl\assets\image-20251028161859120.png) | ![image-20251028161838215](E:\code\Learn\opengl\assets\image-20251028161838215.png) |
|                      漫反射IBL+镜面IBL                       |                         换个HDR贴图                          |
| ![image-20251028161821944](E:\code\Learn\opengl\assets\image-20251028161821944.png) | ![image-20251028162107133](E:\code\Learn\opengl\assets\image-20251028162107133.png) |

## 实战

