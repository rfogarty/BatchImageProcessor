#pragma once

#if __cplusplus >= 201103L

#include <cstdint>
using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;
using std::int32_t;
using std::uint32_t;
using std::int64_t;
using std::uint64_t;

#include <utility> // std::swap (post C++17, this should be in string_view

#else // if Pre C++11

// include C99 int types (note not in std::)
#include <stdint.h>
#include <algorithm>

#endif

namespace stdesque {

template<typename T>
class Endian {
};

template<>
class Endian<uint8_t> {
   static uint8_t swap(uint8_t val) { return val; }
};

template<>
class Endian<uint16_t> {
   union TypePun {
      uint8_t bytes[2];
      uint16_t native;
   };
public:
   static uint16_t swap(uint16_t val) {
      TypePun pun;
      pun.native = val;
      std::swap(pun.bytes[0], pun.bytes[1]);
      return pun.native;
   }
};

template<>
class Endian<int16_t> {
   union TypePun {
      uint8_t bytes[2];
      int16_t native;
   };
public:
   static int16_t swap(int16_t val) {
      TypePun pun;
      pun.native = val;
      std::swap(pun.bytes[0], pun.bytes[1]);
      return pun.native;
   }
};

template<>
class Endian<uint32_t> {
   union TypePun {
      uint8_t bytes[4];
      uint32_t native;
   };
public:
   static uint32_t swap(uint32_t val) {
      TypePun pun;
      pun.native = val;
      std::swap(pun.bytes[0], pun.bytes[3]);
      std::swap(pun.bytes[1], pun.bytes[2]);
      return pun.native;
   }
};

template<>
class Endian<int32_t> {
   union TypePun {
      uint8_t bytes[4];
      int32_t native;
   };
public:
   static int32_t swap(int32_t val) {
      TypePun pun;
      pun.native = val;
      std::swap(pun.bytes[0], pun.bytes[3]);
      std::swap(pun.bytes[1], pun.bytes[2]);
      return pun.native;
   }
};

template<>
class Endian<uint64_t> {
   union TypePun {
      uint8_t bytes[8];
      uint64_t native;
   };
public:
   static uint64_t swap(uint64_t val) {
      TypePun pun;
      pun.native = val;
      std::swap(pun.bytes[0], pun.bytes[7]);
      std::swap(pun.bytes[1], pun.bytes[6]);
      std::swap(pun.bytes[2], pun.bytes[5]);
      std::swap(pun.bytes[3], pun.bytes[4]);
      return pun.native;
   }
};

template<>
class Endian<int64_t> {
   union TypePun {
      uint8_t bytes[8];
      int64_t native;
   };
public:
   static int64_t swap(int64_t val) {
      TypePun pun;
      pun.native = val;
      std::swap(pun.bytes[0], pun.bytes[7]);
      std::swap(pun.bytes[1], pun.bytes[6]);
      std::swap(pun.bytes[2], pun.bytes[5]);
      std::swap(pun.bytes[3], pun.bytes[4]);
      return pun.native;
   }
};

}
