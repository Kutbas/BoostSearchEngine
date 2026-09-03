// server.cc
#include "searcher.hpp"
#include <iostream>
#include <string>

// 清洗后的原始数据路径
const std::string input = "data/raw_html/raw.txt";

int main()
{
    // 1. 实例化搜索器对象
    ns_searcher::Searcher searcher;

    // 2. 初始化搜索器：获取单例并构建底层索引
    searcher.InitSearcher(input);

    std::string query;

    // 3. 命令行交互循环
    while (true)
    {
        std::cout << "Please enter your search query# ";
        if (!std::getline(std::cin, query))
        {
            // 捕获 Ctrl + D 或输入流结束，优雅退出
            std::cout << "\nGoodbye!" << std::endl;
            break;
        }

        // 优化 1：过滤直接按回车的空输入
        if (query.empty())
        {
            continue;
        }

        // 优化 2：局部变量，每次循环拥有独立的空字符串，防止脏数据残留
        std::string json_string;

        // 4. 执行搜索
        searcher.Search(query, &json_string);

        // 5. 打印检索结果
        std::cout << json_string << std::endl;
    }

    return 0;
}