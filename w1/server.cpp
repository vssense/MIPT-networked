#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include "socket_tools.h"
#include "message.h"

class Server {
 public:
  bool Init();
  void Run();

 private:
  void ProcessInitConnection(const char* message_data);
  void ProcessSendMessage(const char* message_data);
  void ProcessKeepAlive(const char* message_data);


  int server_fd_ = -1;
  int client_fd_ = -1;
  addrinfo client_addr_info_;
};

bool Server::Init() {
  server_fd_ = create_dgram_socket(nullptr, kServerPort, nullptr);
  if (server_fd_ == -1)
    return false;

  std::cout << "start listening" << std::endl;
  return true;
}

void Server::Run() {
  int client_fd = -1;

  while (true) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(server_fd_, &read_set);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(server_fd_ + 1, &read_set, NULL, NULL, &timeout);

    if (FD_ISSET(server_fd_, &read_set)) {
      static char buffer[kBufSize];
      memset(buffer, 0, kBufSize);

      ssize_t numBytes = recvfrom(server_fd_, buffer, kBufSize - 1, 0, nullptr, nullptr);
      
      if (numBytes >= sizeof(MessageType)) {
        MessageType message_type = *reinterpret_cast<MessageType*>(buffer);
        char* message_data = buffer + sizeof(MessageType);

        switch (message_type) {
          case kInitConnection:
            ProcessInitConnection(message_data);
            break;

          case kSendMessage:
            ProcessSendMessage(message_data);
            break;
          
          case kKeepAlive:
            ProcessKeepAlive(message_data);
            break;

          default:
            std::cout << "Unknown message_type\n";
            break;
        }
      }
    }
  }
}

void Server::ProcessInitConnection(const char* message_data) {
  std::cout << message_data << " connected to server\n";
  client_fd_ = create_dgram_socket("localhost", kClientPort, &client_addr_info_);
  
  if (client_fd_ == -1) {
    std::cout << "error: " << strerror(errno) << std::endl;
    return;
  }

  MessageType type = kInitConnection;
  if (sendto(client_fd_, &type, sizeof(type), 0, client_addr_info_.ai_addr, client_addr_info_.ai_addrlen) == -1) {
    std::cout << "error: " << strerror(errno) << std::endl;
  }
}


void Server::ProcessSendMessage(const char* message_data) {
  std::cout << "Message recieved: " << message_data << std::endl;
  static char buffer[kBufSize];
  memset(buffer, 0, kBufSize);

  MessageType type = kSendMessage;
  memcpy(buffer, &type, sizeof(type));
  const char* received_msg = "Message received: ";
  strcpy(buffer + sizeof(type), received_msg);
  strcpy(buffer + sizeof(type) + strlen(received_msg), message_data);

  if (sendto(client_fd_, buffer, sizeof(type) + strlen(received_msg) + strlen(message_data),
         0, client_addr_info_.ai_addr, client_addr_info_.ai_addrlen) == -1) {
    std::cout << "error: " << strerror(errno) << std::endl;    
  }
}

void Server::ProcessKeepAlive(const char* /*message_data*/) {}


int main() {
  Server server;
  if (!server.Init()) {
    std::cout << "Unable to init server" << std::endl;
    return 1;
  }

  server.Run();
  return 0;
}
