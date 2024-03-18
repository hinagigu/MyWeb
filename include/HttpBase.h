#include <array>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>
#pragma once
namespace HTTP {
template <typename T> struct EnumClassHash {
  std::size_t operator()(T t) const { return static_cast<std::size_t>(t); }
};
enum HTTP_VERSION { HTTP_10 = 0, HTTP_11, VERSION_NOT_SUPPORT };
enum HTTP_METHOD { GET = 0, POST, PUT, DELETE, METHOD_NOT_SUPPORT };
enum HTTP_HEADER {
  Host = 0,
  User_Agent,
  Connection,
  Accept_Encoding,
  Accept_Language,
  Accept,
  Cache_Control,
  Upgrade_Insecure_Requests,
  HEADER_NOT_SUPPORT
};
enum HTTP_STATUS {
  OK = 200,
  Moved_Permanently = 301,
  Bad_Request = 400,
  Unauthorized = 401,
  Forbidden = 403,
  Not_Found = 404,
  Internal_Server_Error = 500,
  Not_Implemented = 501,
  Status_NOT_SUPPORT
};

static const std::vector<std::pair<const char *, HTTP::HTTP_METHOD>>
    methodStrings = {{"GET", HTTP::GET},
                     {"POST", HTTP::POST},
                     {"PUT", HTTP::PUT},
                     {"DELETE", HTTP::DELETE}};
// HTTP_HEADER对应的字符串数组
static const std::vector<std::pair<const char *, HTTP_HEADER>> headerStrings =
    {{"Host", Host},
      {"User-Agent", User_Agent},
      {"Connection", Connection},
      {"Accept-Encoding", Accept_Encoding},
      {"Accept-Language", Accept_Language},
      {"Accept", Accept},
      {"Cache-Control", Cache_Control},
      {"Upgrade-Insecure-Requests", Upgrade_Insecure_Requests}};

inline auto method_string_to_enum(const std::string_view &method_str)
    -> HTTP_METHOD {
  for (const auto &[str, method] : methodStrings) {
    if (method_str == str) {
      return method;
    }
  }
  return METHOD_NOT_SUPPORT;
}

inline auto header_string_to_enum(const std::string_view &header_str)
    -> HTTP_HEADER {
  for (const auto &[str, header] : headerStrings) {
    if (header_str == str) {
      return header;
    }
  }
  return HEADER_NOT_SUPPORT;
}

} // namespace HTTP
