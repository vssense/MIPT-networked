#include "protocol.h"
#include "quantisation.h"
#include "bitstream.h"
#include <cstring>
#include <iostream>

void send_join(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);
  *packet->data = E_CLIENT_TO_SERVER_JOIN;

  enet_peer_send(peer, 0, packet);
}

void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(Entity),
                                                   ENET_PACKET_FLAG_RELIABLE);

  Bitstream bs(packet->data);
  bs.write(E_SERVER_TO_CLIENT_NEW_ENTITY);
  bs.write(ent);
  enet_peer_send(peer, 0, packet);
}

void send_set_controlled_entity(ENetPeer *peer, uint16_t eid)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t),
                                                   ENET_PACKET_FLAG_RELIABLE);

  Bitstream bs(packet->data);
  bs.write(E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY);
  bs.write_packed_uint32(eid);
  enet_peer_send(peer, 0, packet);
}

void send_entity_input(ENetPeer *peer, uint16_t eid, float thr, float ori)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   sizeof(uint8_t),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);

  float4bitsQuantized thrPacked(thr, -1.f, 1.f);
  float4bitsQuantized oriPacked(ori, -1.f, 1.f);
  uint8_t thrSteerPacked = (thrPacked.packedVal << 4) | oriPacked.packedVal;
  Bitstream bs(packet->data);
  bs.write(E_CLIENT_TO_SERVER_INPUT);
  bs.write_packed_uint32(eid);
  bs.write(thrSteerPacked);
  enet_peer_send(peer, 1, packet);
}

typedef PackedFloat<uint16_t, 11> PositionXQuantized;
typedef PackedFloat<uint16_t, 10> PositionYQuantized;

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float ori)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   sizeof(uint16_t) +
                                                   sizeof(uint16_t) +
                                                   sizeof(uint8_t),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);

  PositionXQuantized xPacked(x, -16, 16);
  PositionYQuantized yPacked(y, -8, 8);
  uint8_t oriPacked = pack_float<uint8_t>(ori, -PI, PI, 8);
  Bitstream bs(packet->data);
  bs.write(E_SERVER_TO_CLIENT_SNAPSHOT);
  bs.write_packed_uint32(eid);
  bs.write(xPacked.packedVal);
  bs.write(yPacked.packedVal);
  bs.write(oriPacked);
  enet_peer_send(peer, 1, packet);
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  Bitstream bs(packet->data);
  uint8_t head;
  bs.read(head);
  bs.read(ent);
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  uint8_t head;
  Bitstream bs(packet->data);
  bs.read(head);
  uint32_t tmp;
  bs.read_packed_uint32(tmp);
  eid = tmp;
}

void deserialize_entity_input(ENetPacket *packet, uint16_t &eid, float &thr, float &steer)
{
  uint8_t head;
  Bitstream bs(packet->data);
  bs.read(head);
  uint32_t tmp;
  bs.read_packed_uint32(tmp);
  eid = tmp;
  uint8_t thrSteerPacked;
  bs.read(thrSteerPacked);
  static uint8_t neutralPackedValue = pack_float<uint8_t>(0.f, -1.f, 1.f, 4);
  static uint8_t nominalPackedValue = pack_float<uint8_t>(1.f, 0.f, 1.2f, 4);
  float4bitsQuantized thrPacked(thrSteerPacked >> 4);
  float4bitsQuantized steerPacked(thrSteerPacked & 0x0f);
  thr = thrPacked.packedVal == neutralPackedValue ? 0.f : thrPacked.unpack(-1.f, 1.f);
  steer = steerPacked.packedVal == neutralPackedValue ? 0.f : steerPacked.unpack(-1.f, 1.f);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &ori)
{
  Bitstream bs(packet->data);
  uint8_t head;
  bs.read(head);
  uint32_t tmp;
  bs.read_packed_uint32(tmp);
  eid = tmp;
  uint16_t xPacked;
  uint16_t yPacked;
  uint8_t oriPacked;
  bs.read(xPacked);
  bs.read(yPacked);
  bs.read(oriPacked);
  PositionXQuantized xPackedVal(xPacked);
  PositionYQuantized yPackedVal(yPacked);
  x = xPackedVal.unpack(-16, 16);
  y = yPackedVal.unpack(-8, 8);
  ori = unpack_float<uint8_t>(oriPacked, -PI, PI, 8);
}

