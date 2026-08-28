#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "util.hpp"

const std::string src_path = "/home/frioi/data_share/Projects/BoostSearchEngine/data/input/html";
const std::string output = "/home/frioi/data_share/Projects/BoostSearchEngine/data/raw_html/raw.txt";

namespace fs = std::filesystem;

struct DocInfo
{
    std::string title;
    std::string content;
    std::string url;
};

// --- 函数声明 ---
bool EnumFile(const std::string &src_path, std::vector<std::string> &files_list);
bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo> &results);
bool SaveHtml(const std::vector<DocInfo> &results, const std::string &output);

// 临时调试函数：修复了参数类型为 DocInfo
void ShowDoc(const DocInfo &doc)
{
    std::cout << "=========================================" << std::endl;
    std::cout << "Title: " << doc.title << std::endl;
    std::cout << "Content: " << doc.content << std::endl;
    std::cout << "URL: " << doc.url << std::endl;
    std::cout << "=========================================" << std::endl;
}

int main()
{
    std::vector<std::string> files_list;

    if (!EnumFile(src_path, files_list))
    {
        std::cerr << "enum file name error!" << std::endl;
        return 1;
    }

    std::vector<DocInfo> results;
    if (!ParseHtml(files_list, results))
    {
        std::cerr << "parse html error" << std::endl;
        return 2;
    }

    if (!SaveHtml(results, output))
    {
        std::cerr << "save html error" << std::endl;
        return 3;
    }

    std::cout << "Process finished successfully." << std::endl;
    return 0;
}

// --- 函数实现 ---

bool EnumFile(const std::string &src_path, std::vector<std::string> &files_list)
{
    fs::path root_path(src_path);
    if (!fs::exists(root_path))
    {
        std::cerr << src_path << " not exists" << std::endl;
        return false;
    }

    for (const auto &entry : fs::recursive_directory_iterator(root_path))
    {
        if (!fs::is_regular_file(entry))
            continue;
        if (entry.path().extension() != ".html")
            continue;
        files_list.push_back(entry.path().string());
    }
    return true;
}

// 修复了参数为引用 &results
bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo> &results)
{
    for (const std::string &file : files_list)
    {
        // 1. 读取文件内容
        std::string file_content; // 统一使用 file_content 接收读取结果
        if (!ns_util::ReadFile(file, file_content))
        {
            continue;
        }

        // 声明 title 和 content 变量
        std::string title;
        std::string content;

        // 2. 解析 HTML，提取 title
        std::size_t begin = file_content.find("<title>");
        if (begin == std::string::npos)
        {
            continue; // 修复：找不到标签时跳过当前文件，而不是 return false 中断整个程序
        }

        std::size_t end = file_content.find("</title>");
        if (end == std::string::npos)
        {
            continue; // 同上
        }

        begin += std::string("<title>").size();
        if (begin >= end)
        {
            continue; // 同上
        }

        title = file_content.substr(begin, end - begin);

        // 3. 解析 HTML 正文，去标签提取 content
        enum State
        {
            LABEL,
            CONTENT
        };
        State s = LABEL;

        for (char c : file_content)
        {
            switch (s)
            {
            case LABEL:
                if (c == '>')
                    s = CONTENT;
                break;
            case CONTENT:
                if (c == '<')
                {
                    s = LABEL;
                }
                else
                {
                    if (c == '\n')
                        c = ' ';
                    content += c;
                }
                break;
            default:
                break;
            }
        }

        // 4. 根据文件路径构建对应的 URL
        // 修复：去掉了 url_head 末尾的 '/'，避免与 url_tail 开头的 '/' 重复拼接成双斜杠
        const std::string url_head = "https://www.boost.org/doc/libs/1_92_0/doc/html";
        std::string url_tail = file.substr(src_path.size()); // 注意这里使用的是循环变量 file
        std::string url = url_head + url_tail;

        // 5. 将解析好的内容存入结构体
        DocInfo doc; // 修复：使用 DocInfo 而不是 DocInfo_t
        doc.title = title;
        doc.content = content;
        doc.url = url;

        // --- 接入调试函数 ---
        // 提示：如果文件很多，打印会非常刷屏。测试没问题后可以把这行注释掉。
        ShowDoc(doc);

        // 6. 将解析结果存入 results
        results.push_back(doc);
    }
    return true;
}

// 将解析完毕的文档结果集写入到目标文件中
bool SaveHtml(const std::vector<DocInfo> &results, const std::string &output)
{
    // 1. 以二进制模式打开输出文件
    std::ofstream out(output, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "open " << output << " failed!" << std::endl;
        return false;
    }

    // 2. 遍历结果集，直接写入文件流
    for (const auto &item : results)
    {
        // 直接使用 operator<< 写入，数据直达文件流缓冲区
        // 避免了临时 std::string 的海量内存分配和拷贝开销
        out << item.title << '\3'
            << item.content << '\3'
            << item.url << '\n';

        // 可选：检查写入过程中是否发生严重错误（如磁盘满）
        if (out.bad())
        {
            std::cerr << "write error occurred!" << std::endl;
            return false;
        }
    }

    // 3. 离开作用域时，out 对象会自动析构并安全关闭文件，无需手动 close()
    return true;
}