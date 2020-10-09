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

#else // if Pre C++11

// include C99 int types (note not in std::)
#include <stdint.h>

#endif

