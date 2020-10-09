#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "cppTools/Platform.h"
#include <limits>

namespace batchIP {
namespace types {

// The following types may be used to scale up the primitive Channel type
// so that an accumulated value can be calculated. Also, this is used to
// print values to avoid printing char types, e.g.
// TODO: since this variable type has more responsibility than choosing an
// accumulator type, this name should be changed.
template<typename PixelT,typename ChannelT = typename PixelT::value_type>
struct AccumulatorVariableSelect { typedef ChannelT type; };
template<typename PixelT> struct AccumulatorVariableSelect<PixelT,uint8_t>  { typedef uint32_t type; };
template<typename PixelT> struct AccumulatorVariableSelect<PixelT,uint16_t> { typedef uint32_t type; };
template<typename PixelT> struct AccumulatorVariableSelect<PixelT,uint32_t> { typedef uint64_t type; };
template<typename PixelT> struct AccumulatorVariableSelect<PixelT,int8_t>   { typedef int32_t  type; };
template<typename PixelT> struct AccumulatorVariableSelect<PixelT,int16_t>  { typedef int32_t  type; };
template<typename PixelT> struct AccumulatorVariableSelect<PixelT,int32_t>  { typedef int64_t  type; };
template<typename PixelT> struct AccumulatorVariableSelect<PixelT,float>    { typedef double   type; };
template<typename PixelT> struct AccumulatorVariableSelect<PixelT,double>   { typedef double   type; };

// The below are not ideal, and in some cases, we would want 128-bit numbers, however, those
// aren't standard yet across platforms.
template<typename PixelT,typename ChannelT = typename PixelT::value_type>
struct BigAccumulatorVariableSelect { typedef ChannelT type; };
template<typename PixelT> struct BigAccumulatorVariableSelect<PixelT,uint8_t>  { typedef uint64_t type; };
template<typename PixelT> struct BigAccumulatorVariableSelect<PixelT,uint16_t> { typedef uint64_t type; };
template<typename PixelT> struct BigAccumulatorVariableSelect<PixelT,uint32_t> { typedef uint64_t type; };
template<typename PixelT> struct BigAccumulatorVariableSelect<PixelT,int8_t>   { typedef int64_t  type; };
template<typename PixelT> struct BigAccumulatorVariableSelect<PixelT,int16_t>  { typedef int64_t  type; };
template<typename PixelT> struct BigAccumulatorVariableSelect<PixelT,int32_t>  { typedef int64_t  type; };
template<typename PixelT> struct BigAccumulatorVariableSelect<PixelT,float>    { typedef double   type; };
template<typename PixelT> struct BigAccumulatorVariableSelect<PixelT,double>   { typedef double   type; };


template<typename ChannelT>
struct ChannelTraits {
   typedef ChannelT value_type;

#if __cplusplus >= 201103L
   static constexpr
#else
   static
#endif
   value_type min() { return 0; }

#if __cplusplus >= 201103L
   static constexpr
#else
   static
#endif
   value_type max() { return std::numeric_limits<value_type>::max(); }
};

template<>
struct ChannelTraits<float> {
   typedef float value_type;

#if __cplusplus >= 201103L
   static constexpr
#else
   static
#endif
   value_type min() { return 0.0f; }

#if __cplusplus >= 201103L
   static constexpr
#else
   static
#endif
   value_type max() { return 1.0f; }
};

template<>
struct ChannelTraits<double> {
   typedef double value_type;

#if __cplusplus >= 201103L
   static constexpr
#else
   static
#endif
   value_type min() { return 0.0; }

#if __cplusplus >= 201103L
   static constexpr
#else
   static
#endif
   value_type max() { return 1.0; }
};


} // namespace types
} // namespace batchIP

