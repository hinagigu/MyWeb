// response
#include "HttpBase.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>
namespace HTTP {
    class HttpRespons{
    public:
        HttpRespons();
        ~HttpRespons();
        void set_status(int status);
        void add_header(const std::string& key, const std::string& value);
        std::string to_string() override;
    private:
        int _status = 200;
        std::unordered_map<std::string, std::string> _headers;
    };
}