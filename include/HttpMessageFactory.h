// 实现一个工厂类来管理Request和Response
#include "HttpBase.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <memory>
#include <mutex>
namespace HTTP {
class HttpMessageFactory {
public:
  // 给出一个创建factory的方法
  static auto getInstance() -> std::unique_ptr<HttpMessageFactory> & {
    static std::once_flag s_flag;
    std::call_once(s_flag,
                   [&]() { instance_.reset(new HttpMessageFactory()); });
    return instance_;
  }
  // 可以添加创建HttpRequest的方法
  auto createHttpRequest(std::string &str) -> std::unique_ptr<HttpRequest>;
  // 可以添加创建HttpResponse的方法
  auto createHttpResponse(std::string &str)
      -> std::unique_ptr<HttpResponse>;
  ~HttpMessageFactory() = default;

private:
  // 防止实例化
  HttpMessageFactory() = default;
  static std::unique_ptr<HttpMessageFactory> instance_;
  HttpMessageFactory(const HttpMessageFactory &) = delete;
  HttpMessageFactory &operator=(const HttpMessageFactory &) = delete;
  HttpMessageFactory(const HttpMessageFactory &&) = delete;
  HttpMessageFactory &operator=(const HttpMessageFactory &&) = delete;
};
// 实现工厂类

} // namespace HTTP