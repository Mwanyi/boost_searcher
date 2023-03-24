#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <jsoncpp/json/json.h>
#include "index.hpp"
#include "util.hpp"
#include "log.hpp"

namespace ns_searcher {
    struct InvertedElemPrint{
        uint64_t doc_id;
        int weight;
        std::vector<std::string> words;
        InvertedElemPrint():doc_id(0), weight(0){}
    };

    class searcher {
        private:
            // 供系统进行查找的索引
            ns_index::Index* index;
        public:
            searcher() {}
            ~searcher() {}
            // 调用buildIndex时的输入，也就是去标签处理完之后保存文档的路径
            // 这里我使用的是data/untag_html/untag.txt
            void InitSearcher(const std::string &input) {
                // 1.获取或者创建索引对象
                index = ns_index::Index::GetInstance();
                LOG(NORMAL, "获取index单例成功...");
                // 2.根据index对象创建索引
                index->BulidIndex(input);
                LOG(NORMAL, "建立正排和倒排索引成功...");
            }

            // word:传入的关键字，json_string：返回给用户浏览器的搜索结果
            void search(const std::string &word, std::string *json_string) {
                // 1.分词，对word进行分词
                std::vector<std::string> words;
                ns_util::JiebaUtil::CutString(word, &words);
                // 2.触发：对每一个分词进行index查找
                // ns_index::InversedList inversedlist_all;
                std::vector<InvertedElemPrint> inversedlist_all;
                std::unordered_map<uint64_t, InvertedElemPrint> tokens_map;

                for (auto s : words) {
                    boost::to_lower(s);
                    // 根据倒排得到该字符的倒排拉链
                    ns_index::InversedList* inversedlist = index->GetInversedIndex(s);
                    if (inversedlist == nullptr) {
                        continue;
                    }
                    //inversedlist_all.insert(inversedlist_all.end(), inversedlist->begin(), inversedlist->end());
                    for(const auto &elem : *inversedlist){
                        auto &item = tokens_map[elem.index_id]; //[]:如果存在直接获取，如果不存在新建
                        //item一定是doc_id相同的print节点
                        item.doc_id = elem.index_id;
                        item.weight += elem.weight;
                        item.words.push_back(elem.word);
                    }
                }

                for(const auto &item : tokens_map){
                    inversedlist_all.push_back(std::move(item.second));
                }
                // 3.合并文档并按照weight进行排序(相关性)
                std::sort(inversedlist_all.begin(), inversedlist_all.end(), \
                        [](const InvertedElemPrint &e1, const InvertedElemPrint &e2) {
                        return e1.weight > e2.weight;
                        });
                // 4.根据结果构建json串—jsonp
                Json::Value root;
                for (auto & item : inversedlist_all) {
                    ns_index::DocInfo *doc = index->GetForwardIndex(item.doc_id);
                    if (nullptr == doc) {
                        continue;
                    }
                    // 通过jsoncpp完成序列化和反序列化
                    Json::Value elem;
                    elem["title"] = doc->title;
                    // 文档全部内容，但是需要的只是一部分
                    elem["dese"] = GetDese(doc->content, item.words[0]);
                    elem["url"] = doc->url;

                    root.append(elem);
                }
                Json::StyledWriter writer;
                *json_string = writer.write(root);
            }
            std::string GetDese(const std::string &content, const std::string &word) {
                // 找到word在content的首次出现，截取出前50个单词，后100个
                const int pre_range = 50;
                const int back_range = 100;
                // 1.找到首次出现
                auto iter = std::search(content.begin(), content.end(), word.begin(), word.end(), [](int x, int y){
                        return (std::tolower(x) == std::tolower(y));
                        });
                if (iter == content.end()) {
                    return "can't find";
                }
                std::size_t pos = std::distance(content.begin(), iter);
                // 2.找出begin(前50或者开头)，end(后100或者结尾)
                std::size_t begin = 0;
                std::size_t end = content.size()-1;
                // 如果前面和后面都不超过begin和end的范围，则更新begin和end
                // size_t是一个无符号整数，若pos较小减完就是一个负数，但是无符号数会变成一个很大的正数，因此不能用减法，用加法
                if (pos > begin+pre_range) {
                    begin = pos-pre_range;
                }
                if (pos+back_range < end) {
                    end = pos+back_range;
                }
                // 3.返回字串
                if (begin <= end) {
                    return content.substr(begin, end-begin+1);
                }
                else {
                    return "None2";
                }
            }
    };
}
