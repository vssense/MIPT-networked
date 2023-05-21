#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <string>
#include <assert.h>

void read_input_async(ENetPeer* lobby) {
  while (true) {
    std::string input;
    std::getline(std::cin, input);

    if (strcmp(input.data(), "start") == 0) {
      std::cout << "Starting the game...\n";
      const char *msg = "start";
      ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
      enet_peer_send(lobby, 0, packet);
      break;
    }
  }
}

int main() {
  if (enet_initialize() != 0) {
    std::cout << "Can't initialize enet" << std::endl;
    return 1;
  }

  atexit(enet_deinitialize);

  ENetHost *client = enet_host_create(nullptr, 2, 2, 0, 0);
  if (!client) {
    std::cout << "Can't create enet client" << std::endl;
    return 1;
  }

  ENetAddress lobby_addr;
  enet_address_set_host(&lobby_addr, "localhost");
  lobby_addr.port = 10887;

  ENetPeer *lobbyPeer = enet_host_connect(client, &lobby_addr, 2, 0);
  if (!lobbyPeer) {
    std::cout << "Can't connect to lobby" << std::endl;
    return 1;
  }

  std::thread input_thread(read_input_async, lobbyPeer);

  uint32_t timeStart = enet_time_get();
  uint32_t lastTimeSend = timeStart;
  
  ENetPeer *serverPeer = nullptr;
  while (true) {
    ENetEvent event;
    while (enet_host_service(client, &event, 10) > 0) {
      switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE: {
        std::cout << "Message received: '" << event.packet->data << "'" << std::endl;
        char* data = (char*)event.packet->data; 
        if (strstr(data, "port=") == data) {
          uint32_t port = 0;
          sscanf(data, "port=%u", &port);

          ENetAddress address_server;
          enet_address_set_host(&address_server, "localhost");
          address_server.port = port;

          serverPeer = enet_host_connect(client, &address_server, 2, 0);
          if (!serverPeer) {
            printf("Cannot connect to a server");
            return 1;
          }
          printf("Connecting to game server on port %u\n", port);
        }
        enet_packet_destroy(event.packet);
        break;
      }
      default:
        break;
      };
    }

    if (serverPeer) {
      uint32_t curTime = enet_time_get();
      if (curTime - lastTimeSend > 3000) {
        std::string time = "Client time = " + std::to_string(curTime);
        ENetPacket *packet = enet_packet_create(time.data(), time.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(serverPeer, 0, packet);
        lastTimeSend = curTime;
      }
    }
  }
  enet_host_destroy(client);

  input_thread.join();
  return 0;
}