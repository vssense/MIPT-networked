#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <thread>
#include <unistd.h>
#include "socket_tools.h"
#include "message.h"

class Client {
public:
  bool Init(const std::string& name);
  void Run();

  void InputAndSend();
  void Listen();
  void KeepAlive();

private:
  int server_fd_ = -1;
  int client_fd_ = -1;

  addrinfo addr_info_{};
};

bool Client::Init(const std::string& name) {

  server_fd_ = create_dgram_socket("localhost", kServerPort, &addr_info_);

  if (server_fd_ == -1) {
    return false;
  }

  static char buffer[kBufSize];
  memset(buffer, 0, kBufSize);

  MessageType type = kInitConnection;
  memcpy(buffer, &type, sizeof(type));
  strcpy(buffer + sizeof(type), name.c_str());

  client_fd_ = create_dgram_socket(nullptr, kClientPort, nullptr);
  if (client_fd_ == -1) {
    return false;
  }

  if (sendto(server_fd_, buffer, sizeof(type) + name.length(), 0, addr_info_.ai_addr, addr_info_.ai_addrlen) == -1) {
    return false;
  }

  return true;
}

void Client::Run() {
  std::cout << "Type your message to send it to the server" << std::endl;
  std::thread input_thread(&Client::InputAndSend, this);
  std::thread listen_thread(&Client::Listen, this);
  std::thread keep_alive_thread(&Client::KeepAlive, this);

  input_thread.join();
  listen_thread.join();
  keep_alive_thread.join();

}

void Client::InputAndSend() {
  while (true) {
    std::string input;
    std::getline(std::cin, input);
    static char buffer[kBufSize];
    memset(buffer, 0, kBufSize);
    
    MessageType type = kSendMessage;
    memcpy(buffer, &type, sizeof(type));
    memcpy(buffer + sizeof(type), input.c_str(), input.length());

    if (sendto(server_fd_, buffer, sizeof(type) + input.length(), 0, addr_info_.ai_addr, addr_info_.ai_addrlen) == -1) {
      std::cout << strerror(errno) << std::endl;
      return;
    }
  }
}

void Client::Listen() {
  while (true) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(client_fd_, &read_set);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(client_fd_ + 1, &read_set, NULL, NULL, &timeout);

    if (FD_ISSET(client_fd_, &read_set)) {
      static char buffer[kBufSize];
      memset(buffer, 0, kBufSize);

      ssize_t numBytes = recvfrom(client_fd_, buffer, kBufSize - 1, 0, nullptr, nullptr);

      if (numBytes >= sizeof(MessageType)) {
        MessageType message_type = *reinterpret_cast<MessageType*>(buffer);
        char* message_data = buffer + sizeof(MessageType);

        switch (message_type) {
          case kSendMessage:
            std::cout << "Server replied: " << message_data << std::endl;
            break;

          case kInitConnection:
            std::cout << "Init connection successfully" << std::endl;
            break;
          default:
            std::cout << "Unknown message type\n";
        }
      }
    }
  }
}

void Client::KeepAlive() {
  MessageType type = kKeepAlive;
  while (true) {
    sleep(5);
    if (sendto(server_fd_, &type, sizeof(type), 0, addr_info_.ai_addr, addr_info_.ai_addrlen) == -1)
      std::cout << strerror(errno) << std::endl;
      return;
  }
}

int main(int argc, const char **argv) {
  std::string name;
  if (argc == 2) {
   name = argv[1];
  } else if (argc == 1) {
    name = "Anonymous";
  }

  Client client;
  if (!client.Init(name)) {
    std::cout << "Can't init client" << std::endl;
    return 1;
  }

  client.Run();
  return 0;
}
