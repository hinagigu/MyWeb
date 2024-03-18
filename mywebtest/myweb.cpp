#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

int setup_server(int port) {
  // 套接字 for_tcp ,domain 参数表示IP地址类型 ，type表示面向sock流
  int server_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (server_sock == -1) {
    perror("Failed to create socket");
    return -1;
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Failed to bind");
    close(server_sock);
    return -1;
  }

  if (listen(server_sock, SOMAXCONN) == -1) {
    perror("Failed to listen");
    close(server_sock);
    return -1;
  }

  return server_sock; // 这个函数输入端口号，并设置服务器端的socket
}

void handle_client(int client_sock) {
  char buffer[BUFFER_SIZE];
  while (true) {
    ssize_t nread = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (nread <= 0) {
      break;
    }
    buffer[nread] = '\0';
    std::cout << "Received from client: " << buffer << std::endl;

    // 向客户端发送数据（此处仅为演示，实际应用可能需要更复杂的处理逻辑）
    write(client_sock, buffer, nread);
  }

  close(client_sock);
  std::cout << "Client disconnected" << std::endl;
  // 从客户端接受消息并打出接受到的消息
  // 这里会直接关闭客户端吗？会触发TCP的四次挥手吗
  // 服务器关闭连接（调用close(client_sock)），发送FIN（Finish）报文给客户端。
  // 客户端接收到FIN后，会回复一个ACK（Acknowledgement）报文，此时客户端进入CLOSE_WAIT状态。
  // 客户端在完成数据发送或主动关闭连接时，也会发送FIN报文给服务器。
  // 服务器接收到客户端的FIN后，回复一个ACK报文，然后进入TIME_WAIT状态，等待足够长的时间确保客户端收到了最后一个ACK，然后彻底关闭连接。
}

int main() {
  std::cout << "log: debug start listen" << '\n';
  int server_sock = setup_server(8000);
  if (server_sock == -1) {
    return -1;
  }
  //   EPOLL_CLOEXEC：这是一个标志位，表示在创建epoll实例时同时设置close-on-exec标志。当一个进程执行execve()系统调用时，带有close-on-exec
  std::cout << "log: start epoll" << '\n';
  int epoll_fd = epoll_create1(EPOLL_CLOEXEC); // 这里是另外创建了一个epoll实例，用于监听epoll事件
  if (epoll_fd == -1) {
    perror("Failed to create epoll instance");
    return -1;
  }

  /*这部分代码主要负责设置epoll事件并将其添加到epoll实例中，以及定义一个用于存储epoll事件通知结果的缓冲区。*/
  epoll_event
      ev;  // 这行代码声明了一个epoll_event类型的变量ev，这个结构体用于描述你希望epoll监控的事件类型以及与之关联的文件描述符。
  ev.events =
      EPOLLIN | EPOLLET;  // 使用边缘触发模式  EPOLLIN 表示关注文件描述符的读事件，即当文件描述符可读时，epoll会通知你
  ev.data.fd =
      server_sock;  // EPOLLET 表示使用边缘触发（Edge
                    // Triggered，ET）模式。在边缘触发模式下，epoll只会通知你一次，即当数据首次到达且文件描述符状态从不可读变为可读时。在此之后，你需要一次性读取所有可用数据，直到没有更多数据为止。
  // 这里关联文件描述符，关联需要监听的socket
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock, &ev) == -1) {
    perror("Failed to add server socket to epoll");
    return -1;
  } // 在这里注册了epoll的控制/监听类型
  /*这行代码将ev.events成员变量设置为EPOLLIN | EPOLLET。其中：

  EPOLLIN 表示关注文件描述符的读事件，即当文件描述符可读时，epoll会通知你。
  EPOLLET 表示使用边缘触发（Edge
  Triggered，ET）模式。在边缘触发模式下，epoll只会通知你一次，即当数据首次到达且文件描述符状态从不可读变为可读时。
  在此之后，你需要一次性读取所有可用数据，直到没有更多数据为止。*/
  epoll_event events[MAX_EVENTS];

  while (true) {
    int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (num_events == -1) {
      perror("Error in epoll_wait");
      continue;
    }
    std::cout << "log debug wait end"<<"  "<<num_events<<std::endl; // 这里为什么会出现两次呢？
    // 打出一条nc localhost 8000  log debug wait endlog debug wait end 这里会被触发两次
    // 这里说明阻塞结束了，那么就遍历所有事件，处理服务端接受信息
    for (int i = 0; i < num_events; ++i) {
      if (events[i].data.fd == server_sock) {
        // 新的连接到来
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        // accept4用来接受客户端信息，相比于标准的accept函数，accept4提供了一些额外的选项，可以在创建新套接字的同时设置特定的属性。
        int client_sock = accept4(server_sock, (struct sockaddr*)&client_addr, &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (client_sock == -1) {
          perror("Failed to accept new connection");
          continue;
        }
        // 读事件EPOLLIN、写事件EPOLLOUT,也就是在这里设置epoll关注的事件
        // ev.events = EPOLLET;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client_sock;
        // 添加客户端进程
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &ev) == -1) {
          perror("Failed to add client socket to epoll");
          close(client_sock);
          continue;
        }
      } else {
        // 数据可读，处理客户端数据
        handle_client(events[i].data.fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
      }
    }
  }

  close(server_sock);
  close(epoll_fd);
  return 0;
}