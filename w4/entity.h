#pragma once
#include <cstdint>
#include "raylib.h"

constexpr uint16_t invalid_entity = -1;
struct Entity
{
  uint32_t color = 0xff00ffff;
  Vector2 pos = {0.f, 0.f};
  float size = 15.f;
  uint16_t eid = invalid_entity;
};

