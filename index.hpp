#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstdint>
#include <boost/algorithm/string.hpp>
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

        // 1. 私有化构造函数与析构函数
        Index() = default;
        ~Index() = default;

        // 内部辅助函数声明
        DocInfo *BuildForwardIndex(const std::string &line);
        bool BuildInvertedIndex(const DocInfo &doc);

    public:
        // 2. 禁用拷贝构造和赋值运算符
        Index(const Index &) = delete;
        Index &operator=(const Index &) = delete;

        // 3. 获取单例的全局静态接口 (Meyers Singleton)
        // C++11 保证了局部静态变量在多线程环境下的初始化是绝对安全的
        static Index *GetInstance()
        {
            static Index instance; // 静态局部变量，生命周期随程序，且自动调用析构函数
            return &instance;      // 返回地址，用法和你的设计完全一致
        }

    public:
        // 根据 doc_id 找到文档内容 (正排查询)
        DocInfo *GetForwardIndex(uint64_t doc_id)
        {
            if (doc_id >= forward_index.size())
            {
                std::cerr << "doc_id out of range, error!" << std::endl;
                return nullptr;
            }
            return &forward_index[doc_id];
        }

        // 根据关键字 word，获得倒排拉链 (倒排查询)
        InvertedList *GetInvertedList(const std::string &word)
        {
            auto iter = inverted_index.find(word);
            if (iter == inverted_index.end())
            {
                return nullptr;
            }
            return &(iter->second);
        }

        // 构建索引
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
                DocInfo *doc = BuildForwardIndex(line);
                if (doc == nullptr)
                {
                    std::cerr << "build forward index error for line: " << line << std::endl;
                    continue;
                }

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

        ns_util::StringUtil::CutString(line, results, sep);

        if (results.size() != 3)
        {
            return nullptr;
        }

        DocInfo doc;
        doc.title = results[0];
        doc.content = results[1];
        doc.url = results[2];
        doc.doc_id = forward_index.size();

        forward_index.push_back(std::move(doc));

        return &forward_index.back();
    }

    inline bool Index::BuildInvertedIndex(const DocInfo &doc)
    {
        struct WordCnt
        {
            int title_cnt;
            int content_cnt;
            WordCnt() : title_cnt(0), content_cnt(0) {}
        };

        std::unordered_map<std::string, WordCnt> word_map;

        // 1. 标题分词与统计
        std::vector<std::string> title_words;
        ns_util::JiebaUtil::CutString(doc.title, title_words);

        for (std::string &s : title_words)
        {
            boost::to_lower(s);
            word_map[s].title_cnt++;
        }

        // 2. 正文分词与统计
        std::vector<std::string> content_words;
        ns_util::JiebaUtil::CutString(doc.content, content_words);

        for (std::string &s : content_words)
        {
            boost::to_lower(s);
            word_map[s].content_cnt++;
        }

        // 3. 计算权重并构建倒排拉链
        const int TITLE_WEIGHT = 10;
        const int CONTENT_WEIGHT = 1;

        for (auto &word_pair : word_map)
        {
            InvertedElem elem;
            elem.doc_id = doc.doc_id;
            elem.word = word_pair.first;
            elem.weight = TITLE_WEIGHT * word_pair.second.title_cnt + CONTENT_WEIGHT * word_pair.second.content_cnt;

            inverted_index[word_pair.first].push_back(std::move(elem));
        }

        return true;
    }
} // namespace ns_index