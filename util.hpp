#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <boost/algorithm/string.hpp>

#include "cppjieba/Jieba.hpp"
#include "log.hpp"

namespace ns_util {
    class FileUtil {
        public:
            static bool ReadFile(std::string filepath, std::string* out) {
                std::ifstream in(filepath, std::ifstream::in);
                if (!in.is_open()) {
                    std::cerr << "open file " << filepath << " fail" << std::endl;
                    return false;
                }
                std::string line;
                while (std::getline(in, line)) {
                    *out += line;
                }
                in.close();
                return true;
            }
    };
    class StringUtil {
        public:
            // 字符串分割，将in的字符串根据sep进行分割，最终结果存储在out数组中
            static void CutString(const std::string& in, std::vector<std::string> *out, const std::string &sep) {
                boost::split(*out, in, boost::is_any_of(sep), boost::token_compress_on);
            }
    };

    const char* const DICT_PATH = "./dict/jieba.dict.utf8";
    const char* const HMM_PATH = "./dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "./dict/user.dict.utf8";
    const char* const IDF_PATH = "./dict/idf.utf8";
    const char* const STOP_WORD_PATH = "./dict/stop_words.utf8";
    class JiebaUtil {
        private:
            // static cppjieba::Jieba jieba;
            cppjieba::Jieba jieba;
            std::unordered_map<std::string, bool> stop_words;
            JiebaUtil():jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH) {}
            JiebaUtil(const JiebaUtil&) = delete;
            static JiebaUtil *instance;
        public:
            static JiebaUtil* get_instance()
            {
                static std::mutex mtx;
                if(nullptr == instance){
                    mtx.lock();
                    if(nullptr == instance){
                        instance = new JiebaUtil();
                        instance->InitJiebaUtil();
                    }
                    mtx.unlock();
                }

                return instance;
            }
            void InitJiebaUtil()
            {
                std::ifstream in(STOP_WORD_PATH);
                if(!in.is_open()){
                    LOG(FATAL, "load stop words file error");
                    return;
                }

                std::string line;
                while(std::getline(in, line)){
                    stop_words.insert({line, true});
                }

                in.close();
            }
            void CutStringHelper(const std::string &in, std::vector<std::string> *out) {
                jieba.CutForSearch(in, *out);
                for(auto iter = out->begin(); iter != out->end(); ){
                    auto it = stop_words.find(*iter);
                    if(it != stop_words.end()){
                        //说明当前的string 是暂停词，需要去掉
                        iter = out->erase(iter);
                    }
                    else{
                        iter++;
                    }
                }
            }
            static void CutString(const std::string &in, std::vector<std::string> *out)
            {
                ns_util::JiebaUtil::get_instance()->CutStringHelper(in, out);
            }
    };
    JiebaUtil *JiebaUtil::instance = nullptr;
    // cppjieba::Jieba JiebaUtil::jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);
}
