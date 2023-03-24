#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <mutex>

#include "util.hpp"
#include "log.hpp"

namespace ns_index {
    struct DocInfo {
        std::string title;
        std::string content;
        std::string url;
        uint64_t doc_id;
    };

    struct InversedElem {
        uint64_t index_id;
        std::string word;
        int weight;
    };

    typedef std::vector<InversedElem> InversedList;

    class Index {
        private:
            // 正排索引使用数组，通过id来连接到对应的文档
            std::vector<DocInfo> forward_index;
            // 倒排索引通过关键字链接到对应的多个文档
            std::unordered_map<std::string, InversedList> Inversed_index;
        private:
            Index() {}
            Index(const Index&) = delete;
            Index& operator=(const Index&) = delete;

            static std::mutex mtx;// 锁
            static Index* instance;
        public:
            static Index* GetInstance() {
                if (instance == nullptr) {
                    mtx.lock();
                    if (nullptr == instance) { 
                        instance = new Index();
                    }
                    mtx.unlock();
                }
                return instance;
            }

            DocInfo* GetForwardIndex(uint64_t index_id) {
                if (index_id >= forward_index.size()) {
                    std::cerr << "input index id out of range!!" << std::endl;
                    return nullptr;
                }
                return &forward_index[index_id];
            }

            InversedList* GetInversedIndex(const std::string &word) {
                auto iter = Inversed_index.find(word);
                if (iter == Inversed_index.end()) {
                    std::cerr << "no inversedList!!" << std::endl;
                    return nullptr;
                }
                return &(iter->second);
            }

            // 构建索引表，输入处理完的文档路径
            bool BulidIndex(const std::string &input_path) {
                std::ifstream in(input_path, std::ios::in | std::ios::binary);
                if (!in.is_open()) {
                    std::cerr << input_path << "open fail!!" << std::endl;
                    return false;
                }
                int count = 0;
                std::string line;
                while (std::getline(in, line)) {
                    DocInfo* doc =  BuildForwardIndex(line);
                    if (nullptr == doc) {
                        std::cerr << "build " << line << "error" << std::endl;
                        continue;
                    }

                    BuildInversedIndex(*doc);
                    count++;
                    LOG(NORMAL, "当前的已经建立的索引文档: " + std::to_string(count));
                }
                std::cout << "建立索引成功!" << std::endl;
                return true;
            }
        private:
            DocInfo* BuildForwardIndex(const std::string& input) {
                // 1.解析input，分割字符
                std::vector<std::string> result;
                const std::string sep = "\3";
                ns_util::StringUtil::CutString(input, &result, sep);
                if (result.size() != 3) {
                    return nullptr;
                }
                // 2.将解析好的内容存入到DocInfo中
                DocInfo doc;
                doc.title = result[0];
                doc.content = result[1];
                doc.url = result[2];
                doc.doc_id = forward_index.size();
                // 3.插入到ForwardIndex表中
                forward_index.push_back(std::move(doc));

                return &forward_index.back();
            }

            bool BuildInversedIndex(const DocInfo &doc) {
                // 倒排是建立word->倒排拉链的map
                // 用来统计某一个word在title和内容中出现的次数
                struct word_cnt {
                    int title_cnt;
                    int content_cnt;
                    word_cnt(): title_cnt(0), content_cnt(0) {}
                };
                // 建立某一个word到一个word_cnt的映射
                std::unordered_map<std::string, word_cnt> word_map;
                std::vector<std::string> title_words;
                // 对title进行分词
                ns_util::JiebaUtil::CutString(doc.title, &title_words);
                // 对title进行词频统计
                for (auto title : title_words) {
                    // 将对应的词频转换为小写，因为用户在搜索时不区分大小写
                    boost::to_lower(title);
                    word_map[title].title_cnt++;
                }
                
                std::vector<std::string> content_words;
                // 对content内容进行分词
                ns_util::JiebaUtil::CutString(doc.content, &content_words);
                // 对content进行词频统计
                for (auto content : content_words) {
                    boost::to_lower(content);
                    word_map[content].content_cnt++;
                }
                
#define X 10
#define Y 1
                // 对每一个词构建一个倒排索引
                for (auto &word_pair : word_map) {
                    InversedElem elem;
                    elem.index_id = doc.doc_id;
                    elem.word = word_pair.first;
                    elem.weight = X*word_pair.second.title_cnt + Y*word_pair.second.content_cnt;
                    InversedList &inversedlist = Inversed_index[word_pair.first];
                    inversedlist.push_back(std::move(elem));
                }
                return true;
            }
    };
    Index* Index::instance = nullptr;
    std::mutex Index::mtx;// 锁
}
