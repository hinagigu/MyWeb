#include <gtest/gtest.h>

#include <future>

#include <iostream>
#include <vector>

#include "ThreadPool.h"

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

int multiply(const int a, const int b)
{
  return a * b;
}

TEST(ThreadPool, testSubmit) {
  ThreadPool pool(3);

  // 初始化线程池
  std::vector<std::future<int>> outputs;
  std::vector<int> assume_outputs;
  // 提交乘法操作，总共30个
  outputs.reserve(300);
  assume_outputs.reserve(300);
  for (int i = 1; i <= 3; ++i)
    for (int j = 1; j <= 100; ++j) {
      outputs.push_back(pool.submit(multiply, i, j));
      assume_outputs.push_back(i * j);
    }
  for(int i = 0; i < outputs.size(); ++i)
    EXPECT_EQ(outputs[i].get(), assume_outputs[i]);
}

TEST(ThreadPool, testSubmit2) {
  int ret = 0;
  std::promise<int> prom;
  auto future = prom.get_future();
  ThreadPool pool(1);
  pool.submit([&prom](int x) { prom.set_value(x); }, ret);
  future.wait();
  EXPECT_EQ(future.get(), ret);
}

TEST(ThreadPool, testSubmitQueueFull) {
  ThreadPool pool(1);

  std::vector<std::future<int>> futures;
  for (int i = 0; i < 100; ++i) {
    futures.push_back(pool.submit([i]() { return i; }));
  }
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(futures[i].get(), i);
  }
}