#include "HttpMessageFactory.h"
#include "HttpBase.h"
#include "HttpRequest.h"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
namespace HTTP {
std::unique_ptr<HttpMessageFactory> HttpMessageFactory::instance_{nullptr};
// 必须显示初始化，否则编译错误

std::string_view ltrim(std::string_view sv) {
  sv.remove_prefix(sv.find_first_not_of(' '));
  return sv;
}

auto HttpMessageFactory::createHttpRequest(std::string &str)
    -> std::unique_ptr<HttpRequest> {
  std::istringstream iss(str);
  std::string first_line;
  std::getline(iss, first_line);

  // 使用std::string_view解析第一行
  std::string_view first_line_view(first_line);
  std::size_t space_pos1 = first_line_view.find(' ');
  if (space_pos1 == std::string_view::npos) {
    throw std::runtime_error("Invalid request format: Method not found");
  }
  std::size_t space_pos2 = first_line_view.find(' ', space_pos1 + 1);
  if (space_pos2 == std::string_view::npos) {
    throw std::runtime_error("Invalid request format: Version not found");
  }

  std::string_view method_str = first_line_view.substr(0, space_pos1);
  std::string_view version_str = first_line_view.substr(space_pos2 + 1);
  std::string_view path_str =
      first_line_view.substr(space_pos1 + 1, space_pos2 - space_pos1 - 1);

  HTTP_METHOD method = method_string_to_enum(method_str);
  if (method == HTTP::METHOD_NOT_SUPPORT) {
    throw std::runtime_error("Unsupported HTTP method");
  }
  std::string version(version_str);
  std::string path(path_str);

  std::unique_ptr<HttpRequest> req =
      std::make_unique<HttpRequest>(method, version, path);

  // 解析头部
  while (std::getline(iss, first_line)) {
    if (first_line.empty()) {
      break; // 遇到空行，结束头部解析，准备读取请求体
    }

    std::string_view line_view(first_line);
    std::size_t colon_pos = line_view.find(':');
    if (colon_pos == std::string_view::npos) {
      continue;
    }

    std::string_view header_key = line_view.substr(0, colon_pos);
    std::string_view header_value = ltrim(line_view.substr(
        colon_pos + 1)); // 使用自行实现的 ltrim 函数去除前面的空白

    HTTP_HEADER header_enum = header_string_to_enum(header_key);
    if (header_enum != HTTP::HEADER_NOT_SUPPORT) {
      req->add_header(header_enum, std::string(header_value));
    }
  }

  // 读取请求体
  std::stringstream requestBodyStream;
  requestBodyStream << iss.rdbuf();
  req->set_body(requestBodyStream.str());

  return req;
}

auto HttpMessageFactory::createHttpResponse(std::string &str)
    -> std::unique_ptr<HttpResponse> {
  std::istringstream iss(str);
  std::string http_version, status_code_str, reason_phrase;
  std::getline(iss, http_version, ' ');
  std::getline(iss, status_code_str, ' ');
  std::getline(iss, reason_phrase);

  int status_code = std::stoi(status_code_str);
  HTTP_STATUS http_status = static_cast<HTTP_STATUS>(status_code);

  std::unique_ptr<HttpResponse> response =
      std::make_unique<HttpResponse>(http_status, http_version);

  std::string line;
  while (std::getline(iss, line) && !line.empty()) {
    std::smatch match;
    std::regex re(R"(^(\S+):\s*(.*)$)");
    if (std::regex_match(line, match, re)) {
      auto header_key = header_string_to_enum(match[1].str());
      if (header_key != HEADER_NOT_SUPPORT) {
        response->add_header(header_key, match[2].str());
      }
    } else if (line == "\r\n") { // End of headers
      break;
    }
  }

  std::getline(iss, line);         // Discard the ending \r\n
  std::stringstream ss(iss.str()); // Rest is the optional response body
  std::string body((std::istreambuf_iterator<char>(ss)),
                   std::istreambuf_iterator<char>());
  response->set_body(body);

  return response;
}
}
} // namespace HTTP
