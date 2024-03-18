#include <iostream>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
template <typename T> class SafeQueue {
private:
  std::queue<T> m_queue;

  //   std::mutex m_mutex; // 访问互斥信号量
  std::shared_mutex m_mutex; // 访问互斥信号量

public:
  SafeQueue() {}
  SafeQueue(SafeQueue &&other) {}
  ~SafeQueue() {}

  bool empty() // 返回队列是否为空
  {
    // std::unique_lock<std::mutex> lock(m_mutex);
    // std::lock_guard<std::mutex> lock(m_mutex);
    std::cout<<"querying  empty" << std::endl;
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_queue.empty();
  }

  int size() {
    // std::unique_lock<std::mutex> lock(m_mutex);
    std::cout<<"querying  size" << std::endl;
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_queue.size();
  }

  // 队列添加元素
  void push(T &t) {
    // std::unique_lock<std::mutex> lock(m_mutex);
    std::cout<<"pushing" << std::endl;
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    m_queue.emplace(t);
  }
  void push(T &&t) {
    // std::unique_lock<std::mutex> lock(m_mutex);
    std::cout<<"pushing rl" << std::endl;
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    m_queue.push(std::move(t));
  } // 接受一个右值引用
  // 队列取出元素
  bool pop(T &t) {
    // std::unique_lock<std::mutex> lock(m_mutex); // 队列加锁
    std::unique_lock<std::shared_mutex> lock(
        m_mutex); // lockguard为普通互斥锁使用的
    std::cout<<"popping" << std::endl;
    if (m_queue.empty()) {
      std::cout << "queue is empty" << std::endl;
      return false;
    }
    t = std::move(
        m_queue.front()); // 取出队首元素，返回队首元素值，并进行右值引用
    m_queue.pop(); // 弹出入队的第一个元素​
    return true;
  }

  T front() {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::cout<<"front" << std::endl;
    if (m_queue.empty()) {
      throw std::runtime_error("queue is empty");
    }
    return m_queue.front();
  }
};