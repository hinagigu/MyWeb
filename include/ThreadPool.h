// 实现一个线程池
#include "SafeQueue.h"
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

class ThreadPool {
public:
  ThreadPool(int thread_num);
  ~ThreadPool();
  template <typename F, typename... Args>
  auto submit(F &&t, Args &&...args) -> std::future<
      decltype(t(args...))>; // 模板参数列表配合可变参数列表,实现参数转发
  // 写一个工作函数，从线程池中取出一个任务，然后开始运行
  void startwork();

private:
  bool isStop_{false};
  SafeQueue<std::function<void()>> tasks;
  std::mutex mutex;
  std::condition_variable cond;
  std::vector<std::thread> work_threads;
};

// 线程池的调度任务是绑定好的task，这样就不用在每个线程中都重新绑定一次
template <typename F, typename... Args>
auto ThreadPool::submit(F &&t, Args &&...args)
    -> std::future<decltype(t(args...))> {
  using ret_type = decltype(t(args...));
  auto task = std::make_shared<std::packaged_task<ret_type()>>(std::bind(
      std::forward<F>(t),
      std::forward<Args>(args)...)); // 注意 这里需要使用forward实现完美转发
  // 这里task是智能指针，当task被销毁时，任务也会被销毁
  std::function<void()> warpper_func = [task]() { (*task)(); }; // 这里进行一次再封装，若直接push *task，则类型是不匹配的
  tasks.push(warpper_func); 
  std::unique_lock<std::mutex> lock(mutex);
  cond.notify_one();
  return task->get_future();
}
inline void ThreadPool::startwork() {
  while (!isStop_) {
    // 获取任务
    // 获取任务的时候，是在安全队列的，所以没问题，但是要怎么样获取？
    // 任务的返回类型不确定? 这里经过封装，所有任务都表现为void(),无参无返回
    std::function<void()> task;
    // 获取任务的时候，是阻塞的，因为队列中没有任务，所以一直阻塞
    if (tasks.empty()) {
      std::unique_lock<std::mutex> lock(mutex); // 防止多个队列同时阻塞
      std::cout<<"before block"<<std::endl;
      cond.wait(lock, [this]() { return !tasks.empty() || isStop_; });
      std::cout<<"after block"<<std::endl;
    }
    if (isStop_) {
      break;
    } // 先检查是否是因为关闭唤醒了它
    bool popres = tasks.pop(task);
    task(); // 执行任务
  }
} // 调用一个线程工作
inline ThreadPool::ThreadPool(int thread_num) {
  for (int i = 0; i < thread_num; ++i) {
    work_threads.emplace_back([this]() { startwork(); });
  }
  // 启动线程池
}

inline ThreadPool::~ThreadPool() {
  std::cout<<"ThreadPool::~ThreadPool"<<std::endl;
  isStop_ = true;
  cond.notify_all(); // 唤起所有线程并等待它们结束
  for (auto &t : work_threads) {
    if (t.joinable()) {
      t.join();
    }
  }
  // 等待所有线程退出
}
