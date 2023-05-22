#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>

class Bitstream {
  public:
    Bitstream(uint8_t* data) : data_(data)
    {}

    template<typename Type>
    void write(const Type& val) {
      memcpy(data_ + write_ofs, reinterpret_cast<const uint8_t*>(&val), sizeof(Type));
      write_ofs += sizeof(Type);
    }

    template<typename Type>
    void read(Type& val) {
      memcpy(reinterpret_cast<uint8_t*>(&val), data_ + read_ofs, sizeof(Type));
      read_ofs += sizeof(Type);
    }

    void read_packed_uint32(uint32_t& value);
    void write_packed_uint32(uint32_t value);

  private:
    uint8_t* data_;
    uint32_t read_ofs = 0;
    uint32_t write_ofs = 0;
};

void Bitstream::read_packed_uint32(uint32_t& value) {
  uint32_t number = *reinterpret_cast<uint32_t*>(data_);
  if ((number & 0x1) == 0) {
    read_ofs += sizeof(uint8_t);
    value = (number & 0xFF) >> 1;
  } else if ((number & 0x3) == 1) {
    read_ofs += sizeof(uint16_t);
    value = (number & 0xFFFF) >> 2;
  } else {
    read_ofs += sizeof(uint32_t);
    value = (number) >> 2;
  }
}

void Bitstream::write_packed_uint32(uint32_t value) {
  if (value <= 0x7F) {
    write_ofs += sizeof(uint8_t);
    uint8_t* data = reinterpret_cast<uint8_t*>(data_);
    *data = (value << 1) & 0xFE;
  } else if (value <= 0x3FFF) {
    write_ofs += sizeof(uint16_t);
    uint16_t* data = reinterpret_cast<uint16_t*>(data_);
    *data = (value << 2) & 0xFFFC | 0x1;
  } else if (value <= 0x3FFF'FFFF) {
    write_ofs += sizeof(uint32_t);
    uint32_t* data = reinterpret_cast<uint32_t*>(data_);
    *data = (value << 2) & 0xFFFF'FFFC | 0x3;
  }
}