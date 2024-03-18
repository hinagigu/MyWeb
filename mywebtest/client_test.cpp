#include <iostream>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

const int PORT = 8000;
const int NUM_THREADS = 10; // 模拟并发访问的客户端数量

void send_message_to_server(const std::string& message) {
    int client_sock = socket(AF_INET, SOCK_STREAM, 0); // 开启一个套接字
    if (client_sock == -1) {
        perror("Failed to create socket");
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 本地主机IP
    // 设置基本属性
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to server");
        close(client_sock);
        return;
    }

    send(client_sock, message.c_str(), message.size(), 0);

    close(client_sock);
}

int main() {
    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; ++i) {
        std::string message = "Message from thread " + std::to_string(i);
        threads.push_back(std::thread(send_message_to_server, message));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "All clients finished sending messages." << std::endl;

    return 0;
}