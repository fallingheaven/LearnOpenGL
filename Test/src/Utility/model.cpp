#include <model.h>

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::vector<Texture> &textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    // 顶点位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // 顶点法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // 顶点纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // 顶点切线
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // 骨骼ID，4~7留出来是因为之前是用来存实例化模型矩阵的
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    // 骨骼权重
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));


    glBindVertexArray(0);
}

void Mesh::Draw(Shader &shader, int baseTextureUnit) const
{
    unsigned int diffuseNr  = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr   = 1;
    unsigned int heightNr   = 1;
    unsigned int reflectionNr = 1;
    unsigned int displacementNr = 1;
    unsigned int albedoNr = 1;
    unsigned int metallicNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int aoNr = 1;

    unsigned int textureUnit = baseTextureUnit;

    for(const auto & texture : textures)
    {
        std::string number;
        std::string name = texture.type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);
        else if(name == "texture_normal")
            number = std::to_string(normalNr++);
        else if(name == "texture_reflection")
            number = std::to_string(reflectionNr++);
        else if(name == "texture_displacement")
            number = std::to_string(displacementNr++);
        else if(name == "texture_albedo")
            number = std::to_string(albedoNr++);
        else if(name == "texture_metallic")
            number = std::to_string(metallicNr++);
        else if(name == "texture_roughness")
            number = std::to_string(roughnessNr++);
        else if(name == "texture_ao")
            number = std::to_string(aoNr++);

        std::string uniformName = "material." + name + number;
        // 检查 uniform 是否存在
        if (glGetUniformLocation(shader.ID, uniformName.c_str()) != -1)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            shader.setInt(uniformName.c_str(), textureUnit);
            glBindTexture(GL_TEXTURE_2D, texture.id);
            textureUnit++; // 只有在 uniform 存在时才增加单元计数
        }
    }
    glActiveTexture(GL_TEXTURE0);

    // 绘制网格
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::DrawInstanced(Shader& shader, unsigned int amount, int baseTextureUnit) const
{
    unsigned int diffuseNr  = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr   = 1;
    unsigned int heightNr   = 1;
    unsigned int reflectionNr = 1;
    unsigned int displacementNr = 1;
    unsigned int albedoNr = 1;
    unsigned int metallicNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int aoNr = 1;

    unsigned int textureUnit = baseTextureUnit;

    for(const auto & texture : textures)
    {
        std::string number;
        std::string name = texture.type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);
        else if(name == "texture_normal")
            number = std::to_string(normalNr++);
        else if(name == "texture_reflection")
            number = std::to_string(reflectionNr++);
        else if(name == "texture_displacement")
            number = std::to_string(displacementNr++);
        else if(name == "texture_albedo")
            number = std::to_string(albedoNr++);
        else if(name == "texture_metallic")
            number = std::to_string(metallicNr++);
        else if(name == "texture_roughness")
            number = std::to_string(roughnessNr++);
        else if(name == "texture_ao")
            number = std::to_string(aoNr++);

        std::string uniformName = "material." + name + number;
        if (glGetUniformLocation(shader.ID, uniformName.c_str()) != -1)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            shader.setInt(uniformName.c_str(), textureUnit);
            glBindTexture(GL_TEXTURE_2D, texture.id);
            textureUnit++;
        }
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, amount);
    glBindVertexArray(0);
}

void Mesh::SetAttributeFloat(unsigned int vbo, int index, int divisor)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glVertexAttribDivisor(index, divisor);
    glBindVertexArray(0);
}

void Mesh::SetAttributeVec2(unsigned int vbo, int index, int divisor)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glVertexAttribDivisor(index, divisor);
    glBindVertexArray(0);
}

void Mesh::SetAttributeVec3(unsigned int vbo, int index, int divisor)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(index, divisor);
    glBindVertexArray(0);
}

void Mesh::SetAttributeVec4(unsigned int vbo, int index, int divisor)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glVertexAttribDivisor(index, divisor);
    glBindVertexArray(0);
}

void Mesh::SetAttributeMat4(unsigned int vbo, int index, int divisor)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // 一个 mat4 等于 4 个 vec4
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(index + 1);
    glVertexAttribPointer(index + 1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(index + 2);
    glVertexAttribPointer(index + 2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(index + 3);
    glVertexAttribPointer(index + 3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(index, divisor);
    glVertexAttribDivisor(index + 1, divisor);
    glVertexAttribDivisor(index + 2, divisor);
    glVertexAttribDivisor(index + 3, divisor);

    glBindVertexArray(0);
}

void BindEmptyTexture(Shader& shader)
{
    GLuint emptyTex;
    glGenTextures(1, &emptyTex);
    glBindTexture(GL_TEXTURE_2D, emptyTex);
    unsigned char white[4] = {0, 0, 0, 0};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);

    shader.setInt("material.texture_diffuse1", 0);
    shader.setInt("material.texture_specular1", 1);
    shader.setInt("material.texture_displacement1", 1);
    shader.setInt("material.texture_normal1", 2);
    shader.setInt("material.texture_reflection1", 3);

    for (int i = 0; i < 4; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, emptyTex);
    }
}

void Model::Draw(Shader &shader, int baseTextureUnit) const
{
    shader.use();
    BindEmptyTexture(shader);
    for(auto & mesh : meshes)
        mesh.Draw(shader, baseTextureUnit);
}

void Model::DrawInstanced(Shader& shader, unsigned int amount, int baseTextureUnit) const
{
    shader.use();
    BindEmptyTexture(shader);
    for (auto & mesh : meshes)
        mesh.DrawInstanced(shader, amount, baseTextureUnit);
}

void Model::SetAttributeFloat(unsigned int vbo, int index, int divisor)
{
    for (auto & mesh : meshes)
    {
        mesh.SetAttributeFloat(vbo, index, divisor);
    }
}

void Model::SetAttributeVec2(unsigned int vbo, int index, int divisor)
{
    for (auto & mesh : meshes)
    {
        mesh.SetAttributeVec2(vbo, index, divisor);
    }
}

void Model::SetAttributeVec3(unsigned int vbo, int index, int divisor)
{
    for (auto & mesh : meshes)
    {
        mesh.SetAttributeVec3(vbo, index, divisor);
    }
}

void Model::SetAttributeVec4(unsigned int vbo, int index, int divisor)
{
    for (auto & mesh : meshes)
    {
        mesh.SetAttributeVec4(vbo, index, divisor);
    }
}

void Model::SetAttributeMat4(unsigned int vbo, int index, int divisor)
{
    for (auto & mesh : meshes)
    {
        mesh.SetAttributeMat4(vbo, index, divisor);
    }
}

void Model::loadModel(const std::string &path)
{
    std::cout << "Loading model: " << path << std::endl;
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    std::cout << "Model loaded successfully with " << scene->mNumMeshes << " meshes" << std::endl;
    directory = path.substr(0, path.find_last_of('/'));
    std::cout << "Texture directory: " << directory << std::endl;

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // 处理节点所有的网格（如果有的话）
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // 接下来对它的子节点重复这一过程
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};
        // 处理顶点位置、法线和纹理坐标
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        if (mesh->mTangents) // 检查切线是否存在
        {
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
        }

        if(mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        setVertexBoneDataToDefault(vertex);

        vertices.push_back(vertex);
    }

    extractBoneWeightForVertices(vertices,mesh,scene);

    // 处理索引
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // 处理材质
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        // for (int i = 0; i < 27; i++)
        // {
        //     std::cout << loadMaterialTextures(material, (aiTextureType)i, "texture_albedo", scene).size() << ' ';
        // } std::cout << std::endl;
        // PBR 反照率贴图
        // 对于 PBR 材质，glTF 标准定义了基础颜色（Albedo）贴图。然而，为了向后兼容，Assimp 可能会同时将这个贴图报告为 aiTextureType_BASE_COLOR 和 aiTextureType_DIFFUSE。
        std::vector<Texture> albedoMaps = loadMaterialTextures(material,
                                                                aiTextureType_BASE_COLOR, "texture_albedo", scene);
        textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());

        // 传统漫反射贴图
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                                            aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // 传统高光贴图
        std::vector<Texture> specularMaps = loadMaterialTextures(material,
                                                                 aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // 反射贴图
        std::vector<Texture> reflectionMaps = loadMaterialTextures(material,
                                                                 aiTextureType_REFLECTION, "texture_reflection", scene);
        // std::vector<Texture> reflectionMaps = loadMaterialTextures(material,
        //                                                          aiTextureType_AMBIENT, "texture_reflection", scene);// 之前Ambient就是reflection
        textures.insert(textures.end(), reflectionMaps.begin(), reflectionMaps.end());

        // 法线贴图
        std::vector<Texture> normalMaps = loadMaterialTextures(material,
                                                              aiTextureType_NORMALS, "texture_normal", scene);
        // std::vector<Texture> normalMaps = loadMaterialTextures(material,
        //                                                         aiTextureType_HEIGHT, "texture_normal", scene);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // 位移贴图
        std::vector<Texture> displacementMaps = loadMaterialTextures(material,
                                                                aiTextureType_DISPLACEMENT, "texture_displacement", scene);
        textures.insert(textures.end(), displacementMaps.begin(), displacementMaps.end());

        // PBR 金属度贴图
        std::vector<Texture> metallicMaps = loadMaterialTextures(material,
                                                                aiTextureType_METALNESS, "texture_metallic", scene);
        textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
        // PBR 粗糙度贴图
        std::vector<Texture> roughnessMaps = loadMaterialTextures(material,
                                                                aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness", scene);
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
        // PBR AO 贴图
        std::vector<Texture> aoMaps = loadMaterialTextures(material,
                                                                aiTextureType_AMBIENT_OCCLUSION, "texture_ao", scene);
        textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    }

    return {vertices, indices, textures};
}

void Model::setVertexBoneDataToDefault(Vertex& vertex)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

void Model::setVertexBoneData(Vertex& vertex, int boneID, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.m_BoneIDs[i] < 0)
        {
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;
            break; // 只分配给第一个空槽
        }
    }
}

void Model::extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) // 没有预存该骨骼信息
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offset = Utility::convertMatrixToGLMFormat(
                mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter;
            m_BoneCounter++;
        }
        else
        {
            boneID = m_BoneInfoMap[boneName].id;
        }

        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) // 将aiMesh的骨骼权重赋值给自己Mesh中的顶点
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId <= vertices.size());
            setVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string& typeName, const aiScene *scene)
{
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(auto & tex : textures_loaded)
        {
            if(std::strcmp(tex.path.C_Str(), str.C_Str()) == 0)
            {
                textures.push_back(tex);
                skip = true;
                break;
            }
        }
        if(!skip)
        {   // 如果纹理还没有被加载，则加载它
            Texture texture;
            if (const auto* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str()))
            {
                // 是嵌入式纹理
                std::cout << "Loading embedded texture: " << str.C_Str() << " type: " << typeName << std::endl;
                if (Utility::getFileExtension(str.C_Str()) == "png") {
                    stbi_set_flip_vertically_on_load(true);
                } else {
                    stbi_set_flip_vertically_on_load(false);
                }
                texture.id = Utility::TextureFromMemory(embeddedTexture, false);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
            else
            {
                std::cout << "Loading texture: " << str.C_Str() << " from " << directory << "type: " << typeName << std::endl;
                texture.id = Utility::TextureFromFile(str.C_Str(), directory, false);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // 添加到已加载的纹理中
            }
        }
    }
    return textures;
}

Bone::Bone(std::string  name, int ID, const aiNodeAnim* channel)
       :
       m_Name(std::move(name)),
       m_ID(ID),
       m_LocalTransform(1.0f)
{
    // 从 aiNodeAnim 提取关键帧数据
    m_NumPositions = channel->mNumPositionKeys;

    for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
    {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data{};
        data.position = Utility::getGLMVec(aiPosition);
        data.timeStamp = timeStamp;
        m_Positions.push_back(data);
    }

    m_NumRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
    {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data{};
        data.orientation = Utility::getGLMQuat(aiOrientation);
        data.timeStamp = timeStamp;
        m_Rotations.push_back(data);
    }

    m_NumScalings = channel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
    {
        aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        float timeStamp = channel->mScalingKeys[keyIndex].mTime;
        KeyScale data{};
        data.scale = Utility::getGLMVec(scale);
        data.timeStamp = timeStamp;
        m_Scales.push_back(data);
    }
}

void Bone::update(float animationTime)
{
    glm::mat4 translation = interpolatePosition(animationTime);
    glm::mat4 rotation = interpolateRotation(animationTime);
    glm::mat4 scale = interpolateScaling(animationTime);
    m_LocalTransform = translation * rotation * scale;
}

// 找到当前时间点 animationTime 所在位置关键帧的索引
int Bone::getPositionIndex(float animationTime)
{
    for (int index = 0; index < m_NumPositions - 1; ++index)
    {
        if (animationTime < m_Positions[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

int Bone::getRotationIndex(float animationTime)
{
    for (int index = 0; index < m_NumRotations - 1; ++index)
    {
        if (animationTime < m_Rotations[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

int Bone::getScaleIndex(float animationTime)
{
    for (int index = 0; index < m_NumScalings - 1; ++index)
    {
        if (animationTime < m_Scales[index + 1].timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

// 通过时间计算插值t
float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 Bone::interpolatePosition(float animationTime)
{
    if (1 == m_NumPositions)
        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

    int p0Index = getPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_Positions[p0Index].timeStamp,
        m_Positions[p1Index].timeStamp, animationTime); // 获取插值t
    glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position,
        m_Positions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(float animationTime)
{
    if (1 == m_NumRotations)
    {
        auto rotation = glm::normalize(m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = getRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_Rotations[p0Index].timeStamp,
        m_Rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation,
        m_Rotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::interpolateScaling(float animationTime)
{
    if (1 == m_NumScalings)
        return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

    int p0Index = getScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_Scales[p0Index].timeStamp,
        m_Scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
        , scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

Animation::Animation(const std::string& animationPath, Model* model)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto animation = scene->mAnimations[0];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    readHierarchyData(m_RootNode, scene->mRootNode);
    readMissingBones(animation, *model);
}

Bone* Animation::findBone(const std::string& name)
{
    auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
        [&](const Bone& Bone)
        {
            return Bone.getBoneName() == name;
        }
    );
    if (iter == m_Bones.end()) return nullptr;
    else return &(*iter);
}

void Animation::readMissingBones(const aiAnimation* animation, Model& model)
{
    int size = animation->mNumChannels;

    auto& boneInfoMap = model.getBoneInfoMap();// 获取模型的骨骼信息映射，ID+偏移矩阵
    int& boneCount = model.getBoneCount(); // 获取模型的骨骼计数

    // 遍历动画中的所有通道
    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) // 该骨骼不在模型的骨骼信息映射中，则添加它
        {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        m_Bones.emplace_back(channel->mNodeName.data,
            boneInfoMap[boneName].id, channel);
    }

    m_BoneInfoMap = boneInfoMap;
}

void Animation::readHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = Utility::convertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        readHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

Animator::Animator(Animation* Animation)
{
    m_CurrentTime = 0.0;
    m_CurrentAnimation = Animation;

    // 根据动画中的骨骼数量动态调整大小
    if (m_CurrentAnimation) {
        m_FinalBoneMatrices.resize(m_CurrentAnimation->getBoneCount(), glm::mat4(1.0f));
        for (int i = 0; i < m_CurrentAnimation->getBoneCount(); i++)
            m_FinalBoneMatrices.emplace_back(1.0f);
    }
    else {
        // 如果没有动画，可以设置一个默认大小或留空
        m_FinalBoneMatrices.resize(100, glm::mat4(1.0f));
        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.emplace_back(1.0f);
    }
}

void Animator::updateAnimation(float dt)
{
    m_DeltaTime = dt;
    if (m_CurrentAnimation)
    {
        m_CurrentTime += m_CurrentAnimation->getTicksPerSecond() * dt;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->getDuration());
        calculateBoneTransform(&m_CurrentAnimation->getRootNode(), glm::mat4(1.0f)); // 根节点的父变换矩阵是单位矩阵
    }
}

void Animator::playAnimation(Animation* pAnimation)
{
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
}

void Animator::calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* Bone = m_CurrentAnimation->findBone(nodeName);

    // 更新骨骼的本地变换矩阵
    if (Bone)
    {
        Bone->update(m_CurrentTime);
        nodeTransform = Bone->getLocalTransform();
    }

    // 计算世界坐标系的变换矩阵
    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->getBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        // 获取骨骼的offset矩阵并计算最终变换矩阵
        // offset矩阵让顶点相对于骨骼进行变换，而不是相对于模型的原点
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        calculateBoneTransform(&node->children[i], globalTransformation);
}

glm::mat4 Entity::getModelMatrix()
{
    auto globalMat = glm::mat4(1.0f);
    if (parent != nullptr)
    {
        globalMat = parent->getModelMatrix();
    }

    if (transform.isDirty)
    {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, transform.position);
        model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, transform.scale);

        transform.modelMat = model;
        transform.isDirty = false;
    }

    return globalMat * transform.modelMat;
}

