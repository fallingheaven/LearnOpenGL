// 这是一个更完整的版本，但依赖于 __FILE__ 宏
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>

static std::string getDirectoryPath(const std::string& path)
{
    size_t found = path.find_last_of("/\\");
    return (found != std::string::npos) ? path.substr(0, found) : "./";
}

class FileSystem
{
public:
    // 基于相对路径获取绝对路径
    static std::string getPath(const std::string& path)
    {
        static std::string basePath = getBasePath();
        return basePath + "/" + path;
    }

private:
    // 获取项目根目录，根据本hpp文件的位置需要调整getDirectoryPath的数量
    static std::string getBasePath()
    {
        // 使用 __FILE__ 宏获取当前文件的**绝对路径**
        std::string path = getDirectoryPath(__FILE__);
        // 通常 __FILE__ 会指向 "project_root/include/learnopengl/filesystem.hpp"
        if (path.find("include") != std::string::npos) {
            return getDirectoryPath(getDirectoryPath(path));
        }
        // 如果不在，可能就在源代码根目录
        return path;
    }
};

#endif