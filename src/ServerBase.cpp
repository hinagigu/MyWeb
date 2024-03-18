#include "ServerBase.h"
#include <cstddef>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
// 初始化端口号
ServerBase::ServerBase(int port) {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  // Address Family 第一个参数 AF_INET 指定的是地址家族（Address
  // Family），这属于传输层以下的网络层内容，表示使用的是 IPv4 地址格式。
  // 第二个参数 SOCK_STREAM
  // 表示使用的是面向连接的、提供可靠数据传输服务的传输层协议，具体就是指
  // TCP（Transmission Control Protocol）。 第三个参数 0
  // 表示不指定协议，系统会自动选择一个合适的协议。
  if (socket_fd_ == -1) {
    std::cout << "socket error" << std::endl;
  }
  // 这是在设置IP层的内容
  this->server_addr_.sin_family =
      AF_INET; // 这一行设置的是地址族（address family），这里的 AF_INET 表示
               // Internet 地址簇，即 IPv4 协议。
  this->server_addr_.sin_port = htons(port); // 设置端口号
  this->server_addr_.sin_addr.s_addr =
      INADDR_ANY; // 表示服务器将在本机的所有网络接口上监听连接请求，即它可以接收来自任意IP地址的连接请求。
  bind_socket();  //绑定窗口
  listen_socket(); // 这里要开始监听窗口
  create_epoll();  // 创建epoll
  // set epoll
  event_.events =
      EPOLLIN | EPOLLET; // 边沿触发：变化时触发 水平触发：缓冲区非空就触发
  event_.data.fd = socket_fd_;
  int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd_, &event_);
  if (ret == -1) {
    std::cout << "epoll_ctl error" << std::endl;
  }
  background_thread_.emplace([&] { work_thread(); }); // 开启工作线程
}
void ServerBase::bind_socket() {
  int ret = bind(socket_fd_, (struct sockaddr *)&this->server_addr_,
                 sizeof(this->server_addr_));
  if (ret == -1) {
    std::cerr << "bind error: " << strerror(errno) << std::endl;
  }
}
void ServerBase::listen_socket() {
  int ret = listen(socket_fd_, 10);
  if (ret == -1) {
    std::cout << "listen error" << std::endl;
  } // listen start
}
void ServerBase::accept_socket(struct sockaddr_in *client_addr) {
  socklen_t len = sizeof(*client_addr);
  int connfd = accept(socket_fd_, (struct sockaddr *)&client_addr, &len);
  if (connfd == -1) {
    std::cout << "accept error" << std::endl;
  }
}
//
void ServerBase::create_epoll() {
  epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd_ == -1) {
    std::cout << "create epoll error" << std::endl;
  }
}
/*尽管你将client_sock传递给了handle_client函数处理，但在实际的服务器开发中，handle_client函数可能并不是一次性处理完与客户端的所有交互，
而是需要在一段时间内持续接收客户端的数据或者发送数据给客户端。将客户端套接字添加到epoll中，可以让服务器在接收到客户端的读写事件时及时作出反应，
而无需不断轮询或阻塞等待。*/
void ServerBase::work_thread() {
  while (!stop_work_) {
    int ret = epoll_wait(epoll_fd_, events_, 10, -1);
    if (ret == -1) {
      std::cout << "epoll wait error" << std::endl;
    } // 推测，这里不能else if
    for (int i = 0; i < ret; ++i) {
      // 判断事件类型
      if (events_[i].data.fd == socket_fd_) {
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        // accept4用来接受客户端信息，相比于标准的accept函数，accept4提供了一些额外的选项，可以在创建新套接字的同时设置特定的属性。
        int client_sock = accept4(socket_fd_, (struct sockaddr *)&client_addr,
                                  &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (client_sock == -1) {
          perror("Failed to accept new connection");
          std::cerr << "Error code: " << errno
                    << ", Error message: " << strerror(errno) << std::endl;
          continue;
        } else {
          // 可以获得client的socket
          client_socket_fds_.push_back(client_sock);
          epoll_event client_event;
          client_event.events =
              EPOLLIN | EPOLLET; // 设置关注读事件，并假设使用边缘触发模式
          client_event.data.fd = client_sock;
          epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_sock,
                    &client_event); // 添加对客户端socket的监听
          handle_client(client_sock);
        }
      }
    }
  }
}

void ServerBase::handle_client(int client_sock) {
  // 这里可以留一些外部标志来终止线程
  handle_threads_.emplace_back([&] {
    // 创建线程,这里可以处理客户端的请求
    while (!stop_work_) {
      char buf[BUF_SIZE];
      int ret = recv(client_sock, buf, BUF_SIZE - 1, 0);
      if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          // 在非阻塞模式下，资源暂时不可用是正常的，可以继续循环等待
          continue;
        } else {
          perror("Failed to receive data");
          break; // 出现其他错误则退出循环
        }
      }
      if (ret == 0) {
        // 客户端关闭连接
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_sock, NULL);
        close(client_sock);
        break;
      } else {
        buf[ret] = '\0';
        std::cout << "receive data: " << buf << std::endl;
        break;
      }
    }
  });
}
ServerBase::~ServerBase() {
  stop_work_ = true;
  std::cout << "ServerBase::~ServerBase()" << std::endl;
  for (auto &x : handle_threads_) {
    if (x.joinable()) {
      x.join();
    }
  }
  for (int &cl_socket : client_socket_fds_) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, cl_socket, NULL);
    close(cl_socket);
  }
  background_thread_->join();
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket_fd_, NULL);

  close(epoll_fd_);
  close(socket_fd_);
}
