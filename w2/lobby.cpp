#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include <unordered_set>

const std::string server_port_msg = "port=10888";

bool game_started = false;
std::unordered_set<ENetPeer*> players;


void ProcessConnect(const ENetEvent& event) {
  printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
  if (game_started) {
    ENetPacket *packet = enet_packet_create(server_port_msg.data(), server_port_msg.length() + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(event.peer, 0, packet);
  }
  players.insert(event.peer);
}

void ProcessReceive(const ENetEvent& event) {
  std::cout << "Message recieved: " << event.packet->data << std::endl;
  if (strcmp((char*)event.packet->data, "start") == 0) {
    if (!game_started) {
      for (const auto& peer: players) {
        ENetPacket *packet = enet_packet_create(server_port_msg.data(), server_port_msg.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
      }
      game_started = true;
    } else {
      std::cout << "Received start, but game already started" << std::endl;
    } 
  }

  enet_packet_destroy(event.packet);
}

void ProcessDisconnect(const ENetEvent& event) {
  printf("%x:%u disconnected\n", event.peer->address.host, event.peer->address.port);
  players.erase(event.peer);
}

int main(int argc, const char **argv) {
  if (enet_initialize() != 0) {
    printf("Can't initialize enet");
    return 1;
  }

  atexit(enet_deinitialize);

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = 10887;

  ENetHost *lobby = enet_host_create(&address, 32, 2, 0, 0);

  if (!lobby) {
    std::cout << "Can't create enet lobby host\n";
    return 1;
  }

  while (true) {
    ENetEvent event;
    while (enet_host_service(lobby, &event, 10) > 0) {
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
  }

  enet_host_destroy(lobby);

  return 0;
}