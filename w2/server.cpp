#include <enet/enet.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

static size_t currentID = 0;

std::string getName() { 
  return std::string("Bob_") + std::to_string(currentID);
}

struct Client {
  std::string name;
  size_t id;
};

using Clients = std::unordered_map<ENetPeer*, Client>;
Clients clients;

void SendPings(ENetPeer *peerToSend) {
    std::string ping_info = "Client pings:";
    for (const auto &client : clients)
          ping_info += '\n' + client.second.name + ": " + std::to_string(client.first->roundTripTime);

    if (clients.size() > 1) {
        ENetPacket *packet = enet_packet_create(ping_info.c_str(), ping_info.length() + 1, ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(peerToSend, 1, packet);
    }
}

void player_joined(ENetPeer* new_peer)
{
    Client new_player{getName(), currentID++};
    std::string msg = "New client joined:\n" + new_player.name + ", id: " + std::to_string(new_player.id) + '\n';

    std::string players_table = "Current clients:";
    for (const auto& client : clients) {
        players_table += '\n' + client.second.name + ", id: " + std::to_string(client.second.id) ;
        ENetPacket *packet = enet_packet_create(msg.c_str(), msg.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(client.first, 0, packet);
    }

    ENetPacket *packet = nullptr;
    if (!clients.empty()) {
        packet = enet_packet_create(players_table.c_str(), players_table.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(new_peer, 0, packet);
    }
    clients[new_peer] = new_player;
    printf("Assigned name: %s\nAssigned id: %zu\n\n", new_player.name.c_str(), new_player.id);
}

void ProcessConnect(const ENetEvent& event) {
  printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
  player_joined(event.peer);
}

void ProcessReceive(const ENetEvent& event) {
  printf("%s#%zu sent a message:'%s'\n", clients[event.peer].name.c_str(), clients[event.peer].id, event.packet->data);
  enet_packet_destroy(event.packet);
}

void ProcessDisconnect(const ENetEvent& event) {
  printf("%x:%u disconnected\n", event.peer->address.host, event.peer->address.port);
  clients.erase(event.peer);
}

int main(int argc, const char **argv) {
  if (enet_initialize() != 0) {
    std::cout << "Can't initialize enet";
    return 1;
  }

  atexit(enet_deinitialize);
  
  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = 10888;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);
  if (!server) {
    std::cout << "Can't create enet server host" << std::endl;
    return 1;
  }

  std::cout << "Server started..." << std::endl;

  uint32_t lastTimeSend = enet_time_get();

  while (true) {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
          ProcessConnect(event);
          break;
        case ENET_EVENT_TYPE_RECEIVE:
          ProcessReceive(event);
          break;
        case ENET_EVENT_TYPE_DISCONNECT:
          ProcessDisconnect(event);
          break;
        default:
          break;
      };
    }

    uint32_t curTime = enet_time_get();
    if (curTime - lastTimeSend > 3000) {
      for (const auto& client : clients) {
        std::string time =  "Server time: " + std::to_string(curTime);
        ENetPacket *packet = enet_packet_create(time.c_str(), time.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(client.first, 0, packet);
        lastTimeSend = curTime;
        SendPings(client.first);
      }
    }
    
  }

  enet_host_destroy(server);
  return 0;
}