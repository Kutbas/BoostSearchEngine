#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream> // 引入 stringstream

namespace ns_util
{
    // 直接使用自由函数，无需定义 class
    // 将 out 改为引用类型 std::string&
    inline bool ReadFile(const std::string &file_path, std::string &out)
    {
        // 1. 打开文件 (ifstream 默认就是只读模式)
        std::ifstream in(file_path);
        if (!in.is_open())
        {
            std::cerr << "open file " << file_path << " error!" << std::endl;
            return false;
        }

        // 2. 使用 stringstream 一次性读取整个文件内容
        // 这种方式比逐行 += 快得多，并且完美保留了文件中的换行符等原始格式
        std::ostringstream oss;
        oss << in.rdbuf();

        // 3. 将读取到的内容赋值给 out
        out = oss.str();

        // 不需要手动 in.close()，ifstream 析构时会自动关闭
        return true;
    }

} // namespace ns_util