#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "cppTools/Platform.h"
#include <limits>
#include <ostream>

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

template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct ColorPixel {
   
   typedef ChannelT value_type;
   typedef ChannelTraitsT traits;

   typedef ChannelT Channels[4];
   
   struct RGBA {
      ChannelT red;
      ChannelT green;
      ChannelT blue;
      ChannelT alpha;
#if __cplusplus >= 201103L
      RGBA() :
         red(0),
         green(0),
         blue(0),
         alpha(0)
      {}
#endif
   };
   
   union {
      Channels indexedColor;
      RGBA namedColor;
   };

   enum {
      MAX_CHANNELS = 4
   };

   ColorPixel() :
      namedColor()
   {}

   bool operator==(const ColorPixel& that) {
      return namedColor.red == that.namedColor.red &&
             namedColor.green == that.namedColor.green &&
             namedColor.blue == that.namedColor.blue &&
             namedColor.alpha == that.namedColor.alpha;
   }

   bool operator!=(const ColorPixel& that) {
      return !(*this == that);
   }
};

template <class T> struct is_rgba                            : public std::false_type{};
template <class T> struct is_rgba<const T >                  : public is_rgba<T>{};
template <class T> struct is_rgba<volatile const T >         : public is_rgba<T>{};
template <class T> struct is_rgba<volatile T >               : public is_rgba<T>{};
template <class T> struct is_rgba<T& >                       : public is_rgba<T>{};
template <class T> struct is_rgba<const T& >                 : public is_rgba<T>{};
template <class T> struct is_rgba<volatile const T& >        : public is_rgba<T>{};
template <class T> struct is_rgba<volatile T& >              : public is_rgba<T>{};
// TODO: does this need to consider the traits class? O.w. my have inadvertant false types, if overloaded traits?
template <class T> struct is_rgba<ColorPixel<T> >            : public std::true_type{};



template<typename ChannelT,typename ChannelTraitsT>
std::ostream& operator<<(std::ostream& os,const ColorPixel<ChannelT,ChannelTraitsT>& pixel) {

   os << "Color(" 
      << pixel.namedColor.red << ","
      << pixel.namedColor.green << ","
      << pixel.namedColor.blue << ","
      << pixel.namedColor.alpha << ")";

   return os;
}


template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct GrayAlphaPixel {
   
   typedef ChannelT value_type;
   typedef ChannelTraitsT traits;

   typedef ChannelT Channels[2];
   
   struct GA {
      ChannelT gray;
      ChannelT alpha;
#if __cplusplus >= 201103L
      GA() :
         gray(0),
         alpha(0)
      {}
#endif
   };
   
   union {
      Channels indexedColor;
      GA namedColor;
   };

   enum {
      MAX_CHANNELS = 2
   };

   GrayAlphaPixel() :
      namedColor()
   {}

   bool operator==(const GrayAlphaPixel& that) {
      return namedColor.gray == that.namedColor.gray &&
             namedColor.alpha == that.namedColor.alpha;
   }

   bool operator!=(const GrayAlphaPixel& that) {
      return !(*this == that);
   }
};

// Template types for SFINAE deduction - used to identify proper function overloads
template <class T> struct is_grayscale                       : public std::false_type{};
template <class T> struct is_grayscale<const T >             : public is_grayscale<T>{};
template <class T> struct is_grayscale<volatile const T >    : public is_grayscale<T>{};
template <class T> struct is_grayscale<volatile T >          : public is_grayscale<T>{};
template <class T> struct is_grayscale<T& >                  : public is_grayscale<T>{};
template <class T> struct is_grayscale<const T& >            : public is_grayscale<T>{};
template <class T> struct is_grayscale<volatile const T& >   : public is_grayscale<T>{};
template <class T> struct is_grayscale<volatile T& >         : public is_grayscale<T>{};
// TODO: does this need to consider the traits class? O.w. my have inadvertant false types, if overloaded traits?
template <class T> struct is_grayscale<GrayAlphaPixel<T> >   : public std::true_type{};


template<typename ChannelT,typename ChannelTraitsT>
std::ostream& operator<<(std::ostream& os,const GrayAlphaPixel<ChannelT,ChannelTraitsT>& pixel) {

   os << "Grayscale(" 
      << pixel.namedColor.gray << ","
      << pixel.namedColor.alpha << ")";

   return os;
}



template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct MonochromePixel {

   typedef ChannelT value_type;
   typedef ChannelTraitsT traits;

   typedef ChannelT Channels[1];
   
   struct M {
      ChannelT mono;
#if __cplusplus >= 201103L
      M() :
         mono(0)
      {}
#endif
   };
   
   union {
      Channels indexedColor;
      M        namedColor;
   };

   enum {
      MAX_CHANNELS = 1
   };

   MonochromePixel() :
      namedColor()
   {}

   bool operator==(const MonochromePixel& that) {
      return namedColor.mono == that.namedColor.mono;
   }

   bool operator!=(const MonochromePixel& that) {
      return !(*this == that);
   }
};


template <class T> struct is_monochrome                      : public std::false_type{};
template <class T> struct is_monochrome<const T >            : public is_monochrome<T>{};
template <class T> struct is_monochrome<volatile const T >   : public is_monochrome<T>{};
template <class T> struct is_monochrome<volatile T >         : public is_monochrome<T>{};
template <class T> struct is_monochrome<T& >                 : public is_monochrome<T>{};
template <class T> struct is_monochrome<const T& >           : public is_monochrome<T>{};
template <class T> struct is_monochrome<volatile const T& >  : public is_monochrome<T>{};
template <class T> struct is_monochrome<volatile T& >        : public is_monochrome<T>{};
// TODO: does this need to consider the traits class? O.w. my have inadvertant false types, if overloaded traits?
template <class T> struct is_monochrome<MonochromePixel<T> > : public std::true_type{};


template<typename ChannelT,typename ChannelTraitsT>
std::ostream& operator<<(std::ostream& os,const MonochromePixel<ChannelT,ChannelTraitsT>& pixel) {

   os << "Monochrome(" 
      << pixel.namedColor.mono << ")";

   return os;
}



template<typename PixelT,typename ChannelT = typename PixelT::value_type>
struct AccumulatorVariableSelect { typedef ChannelT type; };

template<typename PixelT> struct AccumulatorVariableSelect<PixelT,unsigned char> { typedef unsigned type; };

template<typename PixelT> struct AccumulatorVariableSelect<PixelT,uint16_t> { typedef unsigned type; };

template<typename PixelT> struct AccumulatorVariableSelect<PixelT,uint32_t> { typedef uint64_t type; };

template<typename PixelT> struct AccumulatorVariableSelect<PixelT,signed char> { typedef int type; };

template<typename PixelT> struct AccumulatorVariableSelect<PixelT,int16_t> { typedef int type; };

template<typename PixelT> struct AccumulatorVariableSelect<PixelT,int32_t> { typedef int64_t type; };

template<typename PixelT> struct AccumulatorVariableSelect<PixelT,float> { typedef double type; };

template<typename PixelT> struct AccumulatorVariableSelect<PixelT,double> { typedef double type; };


