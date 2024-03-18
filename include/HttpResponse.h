// response
#include "HttpBase.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
namespace HTTP {
class HttpResponse {
public:
  using header_map = std::unordered_map<HTTP_HEADER, std::string, EnumClassHash<HTTP_HEADER>>;

  HttpResponse(const HTTP_STATUS &status, const std::string &version)
      : status_(status), version_(version) {}

  HttpResponse() = delete;
  ~HttpResponse() = default;

  auto get_status() const -> HTTP_STATUS { return status_; }
  std::string get_status_string() const {
    return std::to_string(static_cast<int>(status_));
  }

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
  auto has_header(const HTTP_HEADER key) const -> bool {
    return headers_.contains(key);
  }

  void set_body(const std::string &body) { body_ = body; }
  std::string get_body() const { return body_; }

private:
  HTTP_STATUS status_;
  std::string version_;
  header_map headers_;
  std::string body_;
};
} // namespace HTTP