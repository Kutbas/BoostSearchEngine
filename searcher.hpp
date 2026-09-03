#pragma once
#include "util.hpp"
#include <algorithm>
#include <unordered_map> // 引入哈希表用于去重
#include "index.hpp"
#include <algorithm>
#include <jsoncpp/json/json.h>

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

        // 核心搜索接口
        void Search(const std::string &query, std::string *json_string)
        {
            // 防御性校验
            if (json_string == nullptr)
                return;

            // 1. 查询词分词 (修复：去掉 & 符号)
            std::vector<std::string> words;
            ns_util::JiebaUtil::CutString(query, words);

            // 2. 检索倒排索引并去重聚合 (推荐做法：使用哈希表按 doc_id 聚合权重)
            // 如果暂时保留原逻辑，请注意循环变量使用引用避免拷贝：
            std::unordered_map<uint64_t, ns_index::InvertedElem> tokens_map;
            for (std::string &word : words) // 修复：使用引用避免拷贝
            {
                boost::to_lower(word);
                ns_index::InvertedList *inverted_list = index->GetInvertedList(word);
                if (inverted_list == nullptr)
                {
                    continue;
                }

                // 遍历拉链，按 doc_id 去重并累加权重
                for (const auto &elem : *inverted_list)
                {
                    auto &item = tokens_map[elem.doc_id];
                    item.doc_id = elem.doc_id;
                    item.weight += elem.weight; // 多词命中时权重累加
                }
            }

            // 转移到 vector 中准备排序
            ns_index::InvertedList inverted_list_all;
            for (const auto &pair : tokens_map)
            {
                inverted_list_all.push_back(pair.second);
            }

            // 3. 按相关性权重降序排序
            std::sort(inverted_list_all.begin(), inverted_list_all.end(),
                      [](const ns_index::InvertedElem &e1, const ns_index::InvertedElem &e2)
                      {
                          return e1.weight > e2.weight;
                      });

            // 4. 根据排序结果查正排，组装 JSON 数组
            Json::Value root;

            // 设定最大返回条数（比如只展示最相关的 Top 50 条结果）
            const std::size_t MAX_RESULTS = 50;

            for (const auto &item : inverted_list_all)
            {
                // 性能优化：达到最大条数直接截断退出，避免对成千上万个文档无脑生成摘要
                if (root.size() >= MAX_RESULTS)
                {
                    break;
                }

                ns_index::DocInfo *doc = index->GetForwardIndex(item.doc_id);
                if (doc == nullptr)
                {
                    continue;
                }

                Json::Value elem;
                elem["title"] = doc->title;

                // 健壮性优化：优先使用 item.word，如果为空则兜底使用 words[0]
                std::string target_word = item.word.empty() ? (words.empty() ? "" : words[0]) : item.word;

                // 调用动态摘要提取
                elem["desc"] = GetDesc(doc->content, target_word);
                elem["url"] = doc->url;

                // 调试用（可选）
                elem["id"] = (Json::UInt64)doc->doc_id;
                elem["weight"] = item.weight;

                root.append(elem);
            }

            // 5. 序列化为 JSON 字符串
            Json::StyledWriter writer;
            *json_string = writer.write(root);
        }

    private:
        std::string GetDesc(const std::string &html_content, const std::string &word)
        {
            // 防御性校验：空内容直接返回
            if (html_content.empty() || word.empty())
            {
                return "";
            }

            // 1. 设定向前和向后截取的步长 (经验值)
            const std::size_t prev_step = 50;
            const std::size_t next_step = 100;

            // 2. 修复：大小写无关查找关键字首次出现的位置 (使用 std::search 配合 tolower)
            auto it = std::search(
                html_content.begin(), html_content.end(),
                word.begin(), word.end(),
                [](unsigned char c1, unsigned char c2)
                {
                    return std::tolower(c1) == std::tolower(c2);
                });

            // 如果未找到 (防御性设计)，降级返回文章开头的一小段
            if (it == html_content.end())
            {
                std::size_t len = std::min(html_content.size(), (std::size_t)150);
                return html_content.substr(0, len) + "...";
            }

            // 将迭代器转换为下标位置 pos
            std::size_t pos = std::distance(html_content.begin(), it);

            // 3. 计算截取窗口 [start, end)
            std::size_t start = 0;
            std::size_t end = html_content.size(); // 规范为 [start, end) 区间，防溢出

            // 处理左边界：若前面字符足够，往前偏移 prev_step
            if (pos > prev_step)
            {
                start = pos - prev_step;
            }

            // 处理右边界：若后面字符足够，往后偏移 next_step
            if (pos + next_step < end)
            {
                end = pos + next_step;
            }

            // 4. 边界校验
            if (start >= end)
            {
                return "";
            }

            // 5. 组装摘要并添加省略号标记
            std::string desc = html_content.substr(start, end - start);

            // 体验优化：如果前面被截断了，补上前置 "..."
            if (start > 0)
            {
                desc = "..." + desc;
            }

            // 如果后面被截断了，补上后置 "..."
            if (end < html_content.size())
            {
                desc += "...";
            }

            return desc;
        }
    };
} // namespace ns_searcher