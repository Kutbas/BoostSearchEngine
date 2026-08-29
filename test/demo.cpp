#include "inc/cppjieba/Jieba.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const char *const DICT_PATH = "./dict/jieba.dict.utf8";
const char *const HMM_PATH = "./dict/hmm_model.utf8";
const char *const USER_DICT_PATH = "./dict/user.dict.utf8";
const char *const IDF_PATH = "./dict/idf.utf8";
const char *const STOP_WORD_PATH = "./dict/stop_words.utf8";

int main(int argc, char **argv)
{
    // 1. 初始化 Jieba 对象
    cppjieba::Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);

    vector<string> words;
    string s = "小明硕士毕业于中国科学院计算所，后在日本京都大学深造";
    cout << "原始句子: " << s << endl;
    cout << "[demo] CutForSearch 模式分词结果:" << endl;

    // 2. 调用 CutForSearch 进行分词
    jieba.CutForSearch(s, words);

    // 3. 打印分词结果 (使用标准 C++ 循环，完全抛弃 limonp)
    for (size_t i = 0; i < words.size(); ++i)
    {
        cout << words[i];
        // 如果不是最后一个词，就打印分隔符 "/"
        if (i != words.size() - 1)
        {
            cout << "/";
        }
    }
    cout << endl;

    return EXIT_SUCCESS;
}