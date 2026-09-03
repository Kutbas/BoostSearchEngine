// http_server.cc
#include "searcher.hpp"
#include <iostream>

// 清洗后的原始数据源
const std::string input = "data/raw_html/raw.txt";
// 静态网页根目录 (后续用于挂载前端搜索页面 index.html)
const std::string root_path = "./wwwroot";
// HTTP 监听端口
const int port = 8080;

int main()
{
    // 1. 实例化核心搜索器
    ns_searcher::Searcher searcher;

    // 2. 初始化搜索器：完成全局正排与倒排索引的构建
    std::cout << "Starting to build search index..." << std::endl;
    searcher.InitSearcher(input);
    std::cout << "Search index built successfully!" << std::endl;

    // 3. 提示信息
    std::cout << "Searcher ready. Preparing to launch HTTP Server on port " << port << "..." << std::endl;

    // TODO: 下一步引入 cpp-httplib，开启网络路由与监听：
    // httplib::Server svr;
    // svr.set_base_dir(root_path.c_str());
    // svr.Get("/s", [&searcher](const httplib::Request &req, httplib::Response &rsp) {
    //     // 接收前端请求参数并调用 searcher.Search(...)
    // });
    // svr.listen("0.0.0.0", port);

    return 0;
}