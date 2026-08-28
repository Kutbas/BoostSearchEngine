#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstdint>
#include "util.hpp"

namespace ns_index
{
    struct DocInfo
    {
        std::string title;
        std::string content;
        std::string url;
        uint64_t doc_id;
    };

    struct InvertedElem
    {
        uint64_t doc_id;
        std::string word;
        int weight;
    };

    typedef std::vector<InvertedElem> InvertedList;

    class Index
    {
    private:
        std::vector<DocInfo> forward_index;
        std::unordered_map<std::string, InvertedList> inverted_index;

        // 内部辅助函数声明
        // 按照需求：返回刚刚插入的文档的地址，失败返回 nullptr
        DocInfo *BuildForwardIndex(const std::string &line);

        // 接收构建好的文档信息，进行分词并构建倒排索引
        bool BuildInvertedIndex(const DocInfo &doc);

    public:
        Index() = default;
        ~Index() = default;

        Index(const Index &) = delete;
        Index &operator=(const Index &) = delete;

    public:
        DocInfo *GetForwardIndex(uint64_t doc_id)
        {
            if (doc_id >= forward_index.size())
            {
                std::cerr << "doc_id out of range, error!" << std::endl;
                return nullptr;
            }
            return &forward_index[doc_id];
        }

        InvertedList *GetInvertedList(const std::string &word)
        {
            auto iter = inverted_index.find(word);
            if (iter == inverted_index.end())
            {
                return nullptr;
            }
            return &(iter->second);
        }

        bool BuildIndex(const std::string &input)
        {
            std::ifstream in(input, std::ios::in | std::ios::binary);
            if (!in.is_open())
            {
                std::cerr << "open " << input << " failed!" << std::endl;
                return false;
            }

            std::string line;
            int count = 0;
            while (std::getline(in, line))
            {
                // 3. 构建正排索引，拿到刚刚插入的文档地址
                DocInfo *doc = BuildForwardIndex(line);
                if (doc == nullptr)
                {
                    std::cerr << "build forward index error for line: " << line << std::endl;
                    continue;
                }

                // 4. 构建倒排索引 (直接解引用传入)
                BuildInvertedIndex(*doc);

                count++;
                if (count % 50 == 0)
                {
                    std::cout << "当前已建立索引的文档数量: " << count << std::endl;
                }
            }

            in.close();
            return true;
        }
    };

    // --- 函数实现 ---

    inline DocInfo *Index::BuildForwardIndex(const std::string &line)
    {
        std::vector<std::string> results;
        const std::string sep = "\3";

        // 注意：这里去掉了 & 符号，配合上一节 util.hpp 的引用修改
        ns_util::StringUtil::CutString(line, results, sep);

        if (results.size() != 3)
        {
            return nullptr; // 解析失败返回 nullptr
        }

        DocInfo doc;
        doc.title = results[0];
        doc.content = results[1];
        doc.url = results[2];
        doc.doc_id = forward_index.size();

        // 插入到正排索引中
        forward_index.push_back(std::move(doc));

        // 返回刚刚插入的文档的地址
        return &forward_index.back();
    }
} // namespace ns_index