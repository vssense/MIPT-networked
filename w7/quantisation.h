#pragma once
#include "mathUtils.h"
#include "raylib.h"
#include <limits>

template<typename T>
T pack_float(float v, float lo, float hi, int num_bits)
{
  T range = (1 << num_bits) - 1;//std::numeric_limits<T>::max();
  return range * ((clamp(v, lo, hi) - lo) / (hi - lo));
}

template<typename T>
float unpack_float(T c, float lo, float hi, int num_bits)
{
  T range = (1 << num_bits) - 1;//std::numeric_limits<T>::max();
  return float(c) / range * (hi - lo) + lo;
}

template<typename T, int num_bits>
struct PackedFloat
{
  T packedVal;

  PackedFloat(float v, float lo, float hi) { pack(v, lo, hi); }
  PackedFloat(T compressed_val) : packedVal(compressed_val) {}

  void pack(float v, float lo, float hi) { packedVal = pack_float<T>(v, lo, hi, num_bits); }
  float unpack(float lo, float hi) { return unpack_float<T>(packedVal, lo, hi, num_bits); }
};

typedef PackedFloat<uint8_t, 4> float4bitsQuantized;

template<typename T, int num_bits1, int num_bits2>
struct PackedVec2
{
  T packedVal;

  PackedVec2(float v1, float v2, float lo, float hi) { pack(v1, v2, lo, hi); }
  PackedVec2(T compressed_val) : packedVal(compressed_val) {}

  void pack(float v1, float v2, float lo, float hi) {
    T packedVal1 = pack_float<T>(v1, lo, hi, num_bits1);
    T packedVal2 = pack_float<T>(v2, lo, hi, num_bits2);
    packedVal = packedVal1 << num_bits2 | packedVal2;
  }

  Vector2 unpack(float lo, float hi) {
    Vector2 vec;
    vec.x = unpack_float<T>(packedVal >> num_bits2, lo, hi, num_bits1);
    vec.y = unpack_float<T>(packedVal & ((1 << num_bits2) - 1), lo, hi, num_bits2);
    return vec;
  }
};

template<typename T, int num_bits1, int num_bits2, int num_bits3>
struct PackedVec3
{
  T packedVal;
  
  PackedVec3(float v1, float v2, float v3, float lo, float hi) { pack(v1, v2, v3, lo, hi); }
  PackedVec3(T compressed_val) : packedVal(compressed_val) {}

  void pack(float v1, float v2, float v3, float lo, float hi) {
    T packedVal1 = pack_float<T>(v1, lo, hi, num_bits1);
    T packedVal2 = pack_float<T>(v2, lo, hi, num_bits2);
    T packedVal3 = pack_float<T>(v3, lo, hi, num_bits3);
    packedVal = (packedVal1 << (num_bits2 + num_bits3)) | (packedVal2 << num_bits3) | packedVal3;
  }

  Vector3 unpack(float lo, float hi) {
    Vector3 vec;
    vec.x = unpack_float<T>(packedVal >> (num_bits2 + num_bits3), lo, hi, num_bits1);
    vec.y = unpack_float<T>((packedVal >> num_bits3) & ((1 << num_bits2) - 1), lo, hi, num_bits2);
    vec.z = unpack_float<T>(packedVal & ((1 << num_bits3) - 1), lo, hi, num_bits3);
    return vec;
  }
};
