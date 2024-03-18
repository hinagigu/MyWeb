// 先定义一个TCP_server
// include
// socket相关文件夹
#include <netinet/in.h>
#include <optional>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <vector>

// 第一个是socket 第二个是inet_addr
#pragma once
#define MAXEVENTS 1024
#define BUF_SIZE 1024
class ServerBase {
public:
  ServerBase(int port);
  ~ServerBase();
  void start();

private:
  epoll_event event_;
  epoll_event events_[MAXEVENTS];
  struct sockaddr_in server_addr_;
  int socket_fd_;
  int epoll_fd_;
  std::optional<std::thread> background_thread_;
  std::vector<std::thread> handle_threads_;
  std::vector<int> client_socket_fds_;
  bool stop_work_{false};
  socklen_t len_;

  socklen_t get_client_addr_(struct sockaddr *addr, socklen_t len);
  void bind_socket();
  void listen_socket();
  void accept_socket(struct sockaddr_in *client_addr);
  void create_epoll();
  void work_thread();
  void handle_client(int client_socket);

  // 获取客户端的地址信息

};