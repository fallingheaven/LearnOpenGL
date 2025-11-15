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

struct Plane
{
    glm::vec3 point; // 平面上一点

    glm::vec3 normal = { 0.f, 0.f, 0.f };

    float getSignedDistanceToPlane(const glm::vec3& point) const
    {
        glm::vec3 offset = point - this->point;
        return glm::dot(offset, normal);
    }
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;
    Plane leftFace;
    Plane rightFace;
    Plane nearFace;
    Plane farFace;
};

inline Frustum createFrustumFromCamera(const opengl::camera& cam, float aspect, float fovY,
                                       float zNear, float zFar) // 得到的平面法线都是指向内部的
{
    Frustum     frustum;
    const float halfVSide = zFar * tanf(fovY * .5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * cam.Front;

    frustum.nearFace = { cam.Position + zNear * cam.Front, cam.Front };
    frustum.farFace = { cam.Position + frontMultFar, -cam.Front };
    frustum.rightFace = { cam.Position,
                            normalize(glm::cross(cam.Up, frontMultFar + cam.Right * halfHSide)) };
    frustum.leftFace = { cam.Position,
                            normalize(glm::cross(frontMultFar - cam.Right * halfHSide, cam.Up)) };
    frustum.topFace = { cam.Position,
                            normalize(glm::cross(frontMultFar + cam.Up * halfVSide, cam.Right)) };
    frustum.bottomFace = { cam.Position,
                            normalize(glm::cross(cam.Right, frontMultFar - cam.Up * halfVSide)) };

    return frustum;
}

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::mat4 modelMat;
    bool isDirty = true;
    Transform()
    {
        position = glm::vec3(0.0f, 0.0f, 0.0f);
        rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        scale    = glm::vec3(1.0f, 1.0f, 1.0f);
        modelMat = glm::mat4(1.0f);
    }

    glm::mat4 getModelMatrix()
    {
        if (isDirty)
        {
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, scale);

            modelMat = model;
            isDirty = false;
        }
        return modelMat;
    }
};

struct Volume
{
    virtual ~Volume() = default;

    virtual bool isOnFrustum(Frustum& camFrustum, const glm::mat4& worldModel, const glm::vec3& worldScale) const = 0;
};

struct SphereVolume : public Volume
{
    glm::vec3 center;
    float radius;

    SphereVolume(glm::vec3 center, float radius) : center(center), radius(radius) {}

    bool isOnOrForwardPlane(const Plane& plane) const
    {
        return plane.getSignedDistanceToPlane(center) > -radius;
    }

    bool isOnFrustum(Frustum& camFrustum, const glm::mat4& worldModel, const glm::vec3& worldScale) const override
    {
        glm::vec4 worldCenter = worldModel * glm::vec4(center, 1.0f);
        float worldRadius = std::max(std::max(worldScale.x, worldScale.y), worldScale.z) * radius;

        SphereVolume worldSphere({worldCenter.x, worldCenter.y, worldCenter.z}, worldRadius);

        return worldSphere.isOnOrForwardPlane(camFrustum.topFace) &&
               worldSphere.isOnOrForwardPlane(camFrustum.bottomFace) &&
               worldSphere.isOnOrForwardPlane(camFrustum.leftFace) &&
               worldSphere.isOnOrForwardPlane(camFrustum.rightFace) &&
               worldSphere.isOnOrForwardPlane(camFrustum.nearFace) &&
               worldSphere.isOnOrForwardPlane(camFrustum.farFace);
    }
};

struct AABBVolume : public Volume
{
    glm::vec3 center;
    glm::vec3 halfSize;

    AABBVolume(glm::vec3 center, glm::vec3 halfSize) : center(center), halfSize(halfSize) {}

    bool isOnOrForwardPlane(const Plane& plane) const
    {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        const float r = halfSize.x * std::abs(plane.normal.x) +
                halfSize.y * std::abs(plane.normal.y) + halfSize.z * std::abs(plane.normal.z);

        return -r <= plane.getSignedDistanceToPlane(center);
    }

    bool isOnFrustum(Frustum& camFrustum, const glm::mat4& worldModel, const glm::vec3& worldScale) const override
    {
        const glm::vec3 globalCenter{ worldModel * glm::vec4(center, 1.f) };

        const glm::vec3 right   = glm::normalize(worldModel * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)) * worldScale.x * halfSize.x;
        const glm::vec3 up      = glm::normalize(worldModel * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)) * worldScale.y * halfSize.y;
        const glm::vec3 forward = glm::normalize(worldModel * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)) * worldScale.z * halfSize.z;

        const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
            std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
            std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

        const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
            std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
            std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

        const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
            std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
            std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

        //We not need to divise scale because it's based on the half extention of the AABB
        const AABBVolume globalAABB(globalCenter, {newIi, newIj, newIk});

        return (globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.farFace));
    }
};

class Entity
{
public:
    Transform transform;
    Volume* volume;

    Entity(const char* path): volume() { this->model = new Model(path); }
    Entity(Model* model): volume() { this->model = model; }
    ~Entity() {
        // delete model;
        for (auto child : children) {
            delete child;
        }
    }

    glm::mat4 getModelMatrix(); // 得到全局模型矩阵
    std::list<Entity*>& getChildren() { return children; }
    Entity* getParent() { return parent; }

    void setPosition(const glm::vec3& position) {
        transform.position = position;
        transform.isDirty = true;
    }
    void setRotation(const glm::vec3& rotation) {
        transform.rotation = rotation;
        transform.isDirty = true;
    }
    void setScale(const glm::vec3& scale) {
        transform.scale = scale;
        transform.isDirty = true;
    }
    void Draw(Shader& shader) {
        model->Draw(shader);
    }
    void addChild(Entity* child) {
        child->parent = this;
        children.push_back(child);
    }
    void setVolume(Volume* volume) {
        this->volume = volume;
    }
    glm::vec3 getWorldScale() const {
        glm::vec3 s = transform.scale;
        if (parent) {
            s *= parent->getWorldScale();
        }
        return s;
    }
    bool isOnFrustum(Frustum& camFrustum) {
        if (volume == nullptr) return true;
        glm::mat4 worldMat = getModelMatrix();
        glm::vec3 worldScale = getWorldScale();
        return volume->isOnFrustum(camFrustum, worldMat, worldScale);
    }

private:
    Model* model = nullptr;
    Entity* parent = nullptr;
    std::list<Entity*> children;
};