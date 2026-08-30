#pragma once
#include "util.hpp"
#include <algorithm>
#include <unordered_map> // 引入哈希表用于去重
#include "index.hpp"

namespace ns_searcher
{
    class Searcher
    {
    private:
        ns_index::Index *index; // 底层索引的单例指针

    public:
        Searcher() = default;
        ~Searcher() = default;

        // 初始化搜索器
        void InitSearcher(const std::string &input)
        {
            // 1. 获取索引单例对象
            index = ns_index::Index::GetInstance();
            // 2. 调用底层接口，构建正排与倒排索引
            std::cout << "获取索引单例成功，开始构建索引..." << std::endl;
            index->BuildIndex(input);
            std::cout << "索引构建完成！" << std::endl;
        }

        // 核心搜索方法 (将 json_string 改为引用)
        void Search(const std::string &query, std::string &json_string)
        {
            // 1. 分词：对用户的 query 进行切词
            std::vector<std::string> words;
            ns_util::JiebaUtil::CutString(query, words); // 修复：去掉 & 符号

            // 2. 触发倒排与合并去重
            // 使用哈希表按 doc_id 去重，并累加权重
            // key: doc_id, value: 聚合后的倒排节点
            std::unordered_map<uint64_t, ns_index::InvertedElem> tokens_map;

            for (std::string &word : words) // 修复：使用引用避免拷贝
            {
                boost::to_lower(word);
                ns_index::InvertedList *inverted_list = index->GetInvertedList(word);
                if (inverted_list == nullptr)
                {
                    continue;
                }

                // 遍历当前词的倒排拉链，聚合到哈希表中
                for (const auto &elem : *inverted_list)
                {
                    auto &item = tokens_map[elem.doc_id]; // 如果不存在会自动创建
                    item.doc_id = elem.doc_id;
                    item.weight += elem.weight; // 核心：累加权重！命中词越多，权重越高

                    // 拓展：你甚至可以把命中的词拼起来，方便后续做摘要高亮
                    // item.word += elem.word + " ";
                }
            }

            // 3. 将聚合后的结果转移到 vector 中准备排序
            ns_index::InvertedList inverted_list_all;
            for (const auto &pair : tokens_map)
            {
                inverted_list_all.push_back(pair.second);
            }

            // 4. 排序：根据相关性权重 (weight) 进行降序排序
            std::sort(inverted_list_all.begin(), inverted_list_all.end(),
                      [](const ns_index::InvertedElem &e1, const ns_index::InvertedElem &e2)
                      {
                          return e1.weight > e2.weight;
                      });

            // 5. TODO: 根据排好序的 inverted_list_all，去正排索引里拿文档内容，构建 JSON 返回
            // ...
        }
    };
} // namespace ns_searcher