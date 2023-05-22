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

  private:
    uint8_t* data_;
    uint32_t read_ofs = 0;
    uint32_t write_ofs = 0;
};