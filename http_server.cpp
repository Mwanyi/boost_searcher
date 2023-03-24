#include "searcher.hpp"
#include "cpp-httplib/httplib.h"

const std::string root_path = "./wwwroot";
const std::string input_path = "data/untag_html/untag.txt"; 

int main()
{
    ns_searcher::searcher s;
    s.InitSearcher(input_path);

    httplib::Server ser;
    ser.set_base_dir(root_path.c_str());
    ser.Get("/s", [&s](const httplib::Request & req, httplib::Response &res){
            if (!req.has_param("word")) {
                res.set_content("必须要有关键字!", "text/plain: chatset=utf=8");
                return ;
            }
            std::string word = req.get_param_value("word");
            LOG(NORMAL, "用户搜索的: " + word);
            std::string json_string;
            s.search(word, &json_string);
            res.set_content(json_string, "application/json");
            });
    LOG(NORMAL, "服务器启动成功...");
    ser.listen("0.0.0.0", 8081);
    return 0;
}
