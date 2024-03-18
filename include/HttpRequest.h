#include "HttpBase.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
# pragma once
namespace HTTP {
class HttpRequest {
public:
  using header_map =
      std::unordered_map<HTTP_HEADER, std::string, EnumClassHash<HTTP_HEADER>>;
  HttpRequest(HttpRequest &&other) = default;
  HttpRequest &operator=(HttpRequest &&other) = default;
  HttpRequest(const HttpRequest &other) = default;
  HttpRequest &operator=(const HttpRequest &other) = default;
    HttpRequest(const HTTP_METHOD& method, const std::string& version, const std::string& path)
        : method_(method), version_(version), path_(path) {}

  HttpRequest() = delete;
  ~HttpRequest() = default;
  auto get_method() const -> HTTP_METHOD { return method_; }
  std::string get_version() const { return version_; }
  header_map get_headers() const { return headers_; }
  void add_header(const HTTP_HEADER &key, const std::string &value) {
    headers_.insert({key, value});
  }
  auto get_header(const HTTP_HEADER &key) const -> std::string {
    if (!has_header(key)) {
      throw std::runtime_error("header not found");
    }
    return headers_.at(key);
  }
  auto get_header(const HTTP_HEADER &key,
                  const std::string &default_value) const -> std::string {
    if (!has_header(key)) {
      return default_value;
    }
    return headers_.at(key);
  }
  auto has_header(const HTTP_HEADER key) const -> bool{return headers_.contains(key);}
  void set_body(const std::string &body) { body_ = body; }
  std::string get_body() const { return body_; }
  auto get_path() const -> std::string { return path_; }
private:
  HTTP::HTTP_METHOD method_;
  std::string version_;
  header_map headers_;
  std::string body_;
  std::string path_;
};

} // namespace HTTP