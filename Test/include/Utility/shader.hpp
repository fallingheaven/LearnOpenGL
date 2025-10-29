#pragma once

#include <string>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Shader
{
public:
    unsigned int ID{};

    // Shader(const std::string& computePath)
    // {
    //     setShader(computePath);
    // }

    // 构造函数：从文件加载顶点和片段着色器
    Shader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        setShader(vertexPath, fragmentPath);
    }

    Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath)
    {
        setShader(vertexPath, fragmentPath, geometryPath);
    }

    // 使用着色器程序
    void use() const
    {
        glUseProgram(ID);
    }

    void setShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "")
    {
        // 读取着色器代码
        std::string vertexCode, fragmentCode, geometryCode;
        std::ifstream vShaderFile, fShaderFile, gShaderFile;
        bool hasGeometry = !geometryPath.empty();

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);

            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();

            if (hasGeometry)
            {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n";
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        const char* gShaderCode = geometryCode.c_str();

        // 编译着色器
        unsigned int vertex, fragment, geometry;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, nullptr);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, nullptr);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        if (hasGeometry)
        {
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, nullptr);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }

        // 链接着色器程序
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (hasGeometry)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // 删除着色器
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (hasGeometry)
            glDeleteShader(geometry);
    }

    // void setShader(const std::string& computePath)
    // {
    //     // 读取着色器代码
    //     std::string computeCode;
    //     std::ifstream cShaderFile;
    //
    //     cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    //
    //     try
    //     {
    //         cShaderFile.open(computePath);
    //         std::stringstream cShaderStream;
    //         cShaderStream << cShaderFile.rdbuf();
    //         cShaderFile.close();
    //         computeCode = cShaderStream.str();
    //     }
    //     catch (std::ifstream::failure& e)
    //     {
    //         std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n";
    //     }
    //
    //     const char* cShaderCode = computeCode.c_str();
    //
    //     // 编译着色器
    //     unsigned int compute;
    //
    //     compute = glCreateShader(GL_COMPUTE_SHADER);
    //     glShaderSource(compute, 1, &cShaderCode, nullptr);
    //     glCompileShader(compute);
    //     checkCompileErrors(compute, "COMPUTE");
    //
    //     // 链接着色器程序
    //     ID = glCreateProgram();
    //     glAttachShader(ID, compute);
    //     glLinkProgram(ID);
    //     checkCompileErrors(ID, "PROGRAM");
    //
    //     // 删除着色器
    //     glDeleteShader(compute);
    // }

    // 设置 uniform 变量
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
    }

    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string& name, const glm::vec2& value, int count = 1) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), count, &value[0]);
    }

    void setVec3(const std::string& name, const glm::vec3& value, int count = 1) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), count, &value[0]);
    }

    void setVec4(const std::string& name, const glm::vec4& value, int count = 1) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), count, &value[0]);
    }

    void setMat2(const std::string& name, const glm::mat2& mat, int count = 1) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat3(const std::string& name, const glm::mat3& mat, int count = 1) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4(const std::string& name, const glm::mat4& mat, int count = 1) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void bindTexture(const std::string& name, unsigned int textureID, unsigned int unit) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, textureID);
        setInt(name, unit);
    }

private:
    // 检查编译或链接错误
    static void checkCompileErrors(const unsigned int shader, const std::string& type)
    {
        int success;
        char infoLog[1024];

        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            }
        }
    }
};
