#include "HttpBase.h"
#include "HttpMessageFactory.h"
#include <memory>

#include "gtest/gtest.h"

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(HttpMessageFactory, DISABLED_createHttpRequest) {

  std::string req_str = "GET / HTTP/1.1\n"
                        "Host: www.example.com\n"
                        "User-Agent: curl/7.51.0\n"
                        "\n";
  auto &factory = HTTP::HttpMessageFactory::getInstance();
  auto req = factory->createHttpRequest(req_str);
  ASSERT_NO_THROW({
    auto req = factory->createHttpRequest(req_str);
    ASSERT_EQ(req->get_method(), HTTP::HTTP_METHOD::GET);
    ASSERT_EQ(req->get_version(), "HTTP/1.1");
    ASSERT_EQ(req->get_path(), "/");
    ASSERT_EQ(req->get_header(HTTP::HTTP_HEADER::Host), "www.example.com");
    ASSERT_EQ(req->get_header(HTTP::HTTP_HEADER::User_Agent), "curl/7.51.0");
    ASSERT_TRUE(req->get_body().empty());
  });
}

TEST(HttpMessageFactory, DISABLED_createHttpRequestWithBody) {
  std::string req_str = "POST / HTTP/1.1\n"
                        "Host: www.example.com\n"
                        "User-Agent: curl/7.51.0\n"
                        "\n"
                        "body content";
  auto &factory = HTTP::HttpMessageFactory::getInstance();

  ASSERT_NO_THROW({
    auto req = factory->createHttpRequest(req_str);
    ASSERT_EQ(req->get_method(), HTTP::POST);
    ASSERT_EQ(req->get_version(), "HTTP/1.1");
    ASSERT_EQ(req->get_path(), "/");
    ASSERT_EQ(req->get_header(HTTP::Host), "www.example.com");
    ASSERT_EQ(req->get_header(HTTP::User_Agent), "curl/7.51.0");
    ASSERT_EQ(req->get_body(), "body content");
  });
}

TEST(HttpMessageFactory, createHttpRequestWithBrokenBody) {
  std::string req_str = "POST / HTTP/1.1\n"
                        "Host: www.example.com\n"
                        "User-Agent: curl/7.51.0\n"
                        "\n"
                        "body content\n"
                        "\n"
                        "another content";
  auto &factory = HTTP::HttpMessageFactory::getInstance();

  ASSERT_NO_THROW({
    auto req = factory->createHttpRequest(req_str);
    ASSERT_EQ(req->get_method(), HTTP::POST);
    ASSERT_EQ(req->get_version(), "HTTP/1.1");
    ASSERT_EQ(req->get_path(), "/");
    ASSERT_EQ(req->get_header(HTTP::Host), "www.example.com");
    ASSERT_EQ(req->get_header(HTTP::User_Agent), "curl/7.51.0");
    std::cout<<"CONTENT:"<<req->get_body()<<std::endl;
  });
}
