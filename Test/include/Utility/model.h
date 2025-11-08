#pragma once
#include <common.h>
#define MAX_BONE_INFLUENCE 4

// 顶点
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;

    int m_BoneIDs[MAX_BONE_INFLUENCE]; // 影响该顶点的骨骼ID
    float m_Weights[MAX_BONE_INFLUENCE]; // 影响该顶点的骨骼权重
};

struct BoneInfo
{
    // 每个骨骼的唯一ID
    int id;

    // 偏移矩阵从模型空间转换到骨骼空间
    glm::mat4 offset;

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

    auto& getBoneInfoMap() { return m_BoneInfoMap; }
    int& getBoneCount() { return m_BoneCounter; }

private:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    void loadModel(const std::string& path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    void setVertexBoneDataToDefault(Vertex& vertex);
    void setVertexBoneData(Vertex& vertex, int boneID, float weight);
    void extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                              const std::string& typeName, const aiScene *scene);
};

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

// Bone存储了所有与单个骨骼相关的数据和方法
class Bone
{
public:
    Bone(std::string  name, int ID, const aiNodeAnim* channel);
    void update(float animationTime);

    int getPositionIndex(float animationTime);
    int getRotationIndex(float animationTime);
    int getScaleIndex(float animationTime);

    glm::mat4 getLocalTransform() { return m_LocalTransform; }
    auto getBoneName() const { return m_Name; }
    int getBoneID() const { return m_ID; }
private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    glm::mat4 interpolatePosition(float animationTime);
    glm::mat4 interpolateRotation(float animationTime);
    glm::mat4 interpolateScaling(float animationTime);
};

// 用于存储Assimp节点数据的结构体
struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

// Animation类存储了动画数据和方法，包括骨骼和层次结构
class Animation
{
public:
    Animation() = default;
    ~Animation() = default;

    Animation(const std::string& animationPath, Model* model);

    Bone* findBone(const std::string& name);
    inline float getTicksPerSecond() { return m_TicksPerSecond; }

    inline float getDuration() { return m_Duration;}

    inline const AssimpNodeData& getRootNode() { return m_RootNode; }

    inline const std::map<std::string,BoneInfo>& getBoneIDMap() { return m_BoneInfoMap; }
    inline size_t getBoneCount() const { return m_BoneInfoMap.size(); }
private:
    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;

    void readMissingBones(const aiAnimation* animation, Model& model);
    void readHierarchyData(AssimpNodeData& dest, const aiNode* src);
};

class Animator
{
public:
    Animator(Animation* Animation);
    void updateAnimation(float dt);
    void playAnimation(Animation* pAnimation);
    void calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
    std::vector<glm::mat4> getFinalBoneMatrices() { return m_FinalBoneMatrices;  }
private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
};