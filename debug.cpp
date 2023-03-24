#include <iostream>
#include <string>
#include <stdio.h>
#include "searcher.hpp"

const std::string input_path = "data/untag_html/untag.txt";

int main() {
    ns_searcher::searcher *sear = new ns_searcher::searcher();
    sear->InitSearcher(input_path);
    
    std::string query;
    std::string json_string;
    char buffer[1024];
    while (true) {
        std::cout << "please input query:";
        // std::cin >> query;
        fgets(buffer, sizeof(buffer)-1, stdin);
        buffer[sizeof(buffer)-1] = 0;
        query = buffer;
        sear->search(query, &json_string);
        
        std::cout << json_string << std::endl;
    }

    return 0;
}
