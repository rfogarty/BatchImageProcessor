#pragma once

#include <sstream>
#include <stdexcept>

template<typename T>
void reportIfNotLessThan(const char* field,T val,T maxVal) {
#ifndef NDEBUG // Uses the NDEBUG flag to disable, that's also used to disable assert statements
   if(val >= maxVal) {
      std::stringstream ss;
      ss << field << " " << val << " out of range";
      throw std::out_of_range(ss.str());
   }
#endif
}

template<typename T>
void reportIfNotEqual(const char* field,T val1,T val2) {
#ifndef NDEBUG // Uses the NDEBUG flag to disable, that's also used to disable assert statements
   if(val1 != val2) {
      std::stringstream ss;
      ss << field << " " << val1 << " != " << val2;
      throw std::out_of_range(ss.str());
   }
#endif
}



