#pragma once
#include <common.h>

// 顶点
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
};

// 材质
struct Texture {
    unsigned int id{};
    std::string type;
    aiString path;
};

// 一个网格对象，mesh=顶点+材质
class Mesh {
public:

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::vector<Texture> &textures);
    void Draw(Shader &shader, int baseTextureUnit = 0) const;
    void DrawInstanced(Shader& shader, unsigned int amount, int baseTextureUnit = 0) const;

    void SetAttributeFloat(unsigned int vbo, int index, int divisor = 0);
    void SetAttributeVec2(unsigned int vbo, int index, int divisor = 0);
    void SetAttributeVec3(unsigned int vbo, int index, int divisor = 0);
    void SetAttributeVec4(unsigned int vbo, int index, int divisor = 0);
    void SetAttributeMat4(unsigned int vbo, int index, int divisor = 0);
private:
    unsigned int VAO, VBO, EBO;

    void setupMesh();

    const int DIFFUSE_UNIT = 0;
    const int SPECULAR_UNIT = 1;
    const int NORMAL_UNIT = 2;
    const int REFLECTION_UNIT = 3;
    const int DISPLACEMENT_UNIT = 1;
};

// 一个模型，model=mesh*n
class Model
{
public:

    explicit Model(const char *path)
    {
        loadModel(path);
    }
    explicit Model(std::string &path)
    {
        loadModel(path);
    }
    void Draw(Shader &shader, int baseTextureUnit = 0) const;
    void DrawInstanced(Shader &shader, unsigned int amount, int baseTextureUnit = 0) const;

    void SetAttributeFloat(unsigned int vbo, int index, int divisor = 0);
    void SetAttributeVec2(unsigned int vbo, int index, int divisor = 0);
    void SetAttributeVec3(unsigned int vbo, int index, int divisor = 0);
    void SetAttributeVec4(unsigned int vbo, int index, int divisor = 0);
    void SetAttributeMat4(unsigned int vbo, int index, int divisor = 0);

private:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(const std::string& path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                              const std::string& typeName, const aiScene *scene);
};