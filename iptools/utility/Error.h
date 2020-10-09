#pragma once

#ifdef FAIL_WITH_ASSERT
#include <cassert>
#endif
#include <sstream>
#include <stdexcept>

namespace batchIP {
namespace utility {

template<typename T>
void reportIfNotLessThan(const char* field,T val,T maxVal) {
#ifdef FAIL_WITH_ASSERT

   assert(val < maxVal);

#else

#  ifndef NDEBUG // Uses the NDEBUG flag to disable, that's also used to disable assert statements
   if(val >= maxVal) {
      std::stringstream ss;
      ss << field << " " << val << " out of range of " << maxVal;
      throw std::out_of_range(ss.str());
   }
#  endif

#endif
}

template<typename T>
void reportIfNotEqual(const char* field,T val1,T val2) {
#ifdef FAIL_WITH_ASSERT

   assert(val1 == val2);

#else

#  ifndef NDEBUG // Uses the NDEBUG flag to disable, that's also used to disable assert statements
   if(val1 != val2) {
      std::stringstream ss;
      ss << field << " " << val1 << " != " << val2;
      throw std::out_of_range(ss.str());
   }
#  endif

#endif
}


inline void fail(const char* reason) {
#ifdef FAIL_WITH_ASSERT
   assert(false);
#else
   throw std::out_of_range(reason);
#endif
}

} // namespace utility
} // namespace batchIP

