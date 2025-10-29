#include <model.h>
#include <common.h>
#include <utility.h>

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
    // BindEmptyTexture(shader);
    for(auto & mesh : meshes)
        mesh.Draw(shader, baseTextureUnit);
}

void Model::DrawInstanced(Shader& shader, unsigned int amount, int baseTextureUnit) const
{
    shader.use();
    // BindEmptyTexture(shader);
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

        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.Tangent = vector;

        if(mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
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