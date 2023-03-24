#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <utility>

#include "util.hpp"

typedef struct DocInfo {
    std::string title;  // 标题
    std::string content;// 内容
    std::string url;    // url
}DocInfo;

bool SaveFileName(const std::string &SourcePath, std::vector<std::string> *FileNameList);
bool ParseHtml(std::vector<std::string> &FileNameList, std::vector<DocInfo> *results);
bool SaveResult(const std::string &SavePath, const std::vector<DocInfo> &results);

// 数据源存储路径
const std::string SourcePath = "data/input";
// 最终去标签之后存放的文件路径
const std::string SavePath = "data/untag_html/untag.txt";

int main() {
    std::vector<std::string> FileNameList;
    std::vector<DocInfo> results; 

    // 第一步：通过对应的数据源路径取出所有的文件名存储在FileNameList中
    if (!SaveFileName(SourcePath, &FileNameList)) {
        std::cerr << "read file name fail!!" << std::endl;
        return 1;
    }

    // 第二步：通过得到的文件名将所有的文件内容进行解析
    if (!ParseHtml(FileNameList, &results)) {
        std::cerr << "Parser html fail!!" << std::endl;
        return 2;
    }

    // 第三步：将结果写入到结果文件中
    if (!SaveResult(SavePath, results)) {
        std::cerr << "save html fail!!" << std::endl;
        return 3;
    }

    return 0;
}
bool SaveFileName(const std::string &SourcePath, std::vector<std::string> *FileNameList) {
    namespace fs = boost::filesystem;
    fs::path root_path(SourcePath);
    // 如果该路径不存在
    if (!exists(root_path)) {
        std::cerr << SourcePath << "not exists" << std::endl;
        return false;
    }
    // 迭代器end表示一个空，也代表走到结尾
    fs::recursive_directory_iterator end;
    fs::recursive_directory_iterator it = fs::recursive_directory_iterator(root_path);
    for (; it != end; it++) {
        // 如果不是一个常规文件，跳过
        if (!fs::is_regular_file(*it)) {
            continue;
        }
        // 如果该文件的后缀不是html也跳过
        if (it->path().extension() != ".html") {
            continue;
        }

        // std::cout << "debug:" << it->path().string() << std::endl;
        // 如果该迭代器迭代到一个合法的且后缀为html的普通文件，就取出路径加入FileNameList
        FileNameList->push_back(it->path().string());
    }

    return true;
}

static bool ParseTitle(const std::string& file, std::string *title) {
    std::size_t begin = file.find("<title>");
    if (begin == std::string::npos) {
        return false;
    }
    std::size_t end = file.find("</title>");
    if (end == std::string::npos) {
        return false;
    }

    begin += std::string("<title>").size();
    if (begin > end) {
        return false;
    }
    *title = file.substr(begin, end-begin);
    return true;
}

static bool ParseContent(const std::string &file, std::string *content) {
    // 以一个小型状态机实现
    enum state {
        LABLE,
        CONTENT
    };
    enum state s = LABLE;
    for (char c : file) {
        switch(s) {
            case LABLE:
                if (c == '>') {
                    s = CONTENT;
                }
                break;
            case CONTENT:
                if (c == '<') {
                    s = LABLE;
                }
                // 不保留原本的\n，因为在整合所有文档到一个文档时需要使用\n作为文档的分隔符
                else if (c == '\n') {
                    content->push_back(' ');
                }
                else {
                    content->push_back(c);
                }
                break;
            default:
                break;
        }
    }
    return true;
}

static bool ParseURL(const std::string &file, std::string *url) {
    std::string url_head = "https://www.boost.org/doc/libs/1_81_0/doc/html";
    std::string url_tail = file.substr(SourcePath.size());
    *url = url_head + url_tail;
    return true;
}

// for debug
static void ShowDoc(const DocInfo &doc) {
    std::cout << "title:" << doc.title << std::endl;
    std::cout << "content:" << doc.content << std::endl;
    std::cout << "URL:" << doc.url << std::endl;
}

bool ParseHtml(std::vector<std::string> &FileNameList, std::vector<DocInfo> *results) {
    for (const std::string &file : FileNameList) {
        // 1.读取文件全部内容
        std::string result;
        if (!ns_util::FileUtil::ReadFile(file, &result)) {
            continue;        
        }
        DocInfo doc;
        // 2.读取标题
        if (!ParseTitle(result, &doc.title)) {
            continue;
        }
        // 3.读取内容
        if (!ParseContent(result, &doc.content)) {
            continue;
        }
        // 4.读取URL
        if (!ParseURL(file, &doc.url)) {
            continue;
        }

        //results->push_back(doc);
        //优化方案：使用move直接移动可以节省空间，不需要写时拷贝
        results->push_back(std::move(doc));
        // debug
        //ShowDoc(doc);
    }
    
    return true;
}

bool SaveResult(const std::string &SavePath, const std::vector<DocInfo> &results) {
    // 以二进制的方法写入
    std::ofstream out(SavePath, std::ios::out | std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "open SavePath " << SavePath << "fail!!" << std::endl;
        return false;
    }
    for (auto &result : results) {
        std::string output;
        output = result.title;
        output += '\3';
        output += result.content;
        output += '\3';
        output += result.url;
        output += '\n';

        out.write(output.c_str(), output.size());
    }

    out.close();
    return true;
}
