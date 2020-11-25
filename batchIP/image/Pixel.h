#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "cppTools/Platform.h"
#include "Channel.h"
#include "PixelConversion.h"
#include <ostream>

namespace batchIP {
namespace types {

template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct RGBAPixel {
   
   typedef ChannelT value_type;
   typedef ChannelTraitsT traits;

   typedef ChannelT Channels[4];
   
   struct Tuple {
      ChannelT value0;
      ChannelT value1;
      ChannelT value2;
      ChannelT value3;
   };

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
   
   // The concept of this union is to have many aliases
   // for the data values of this Pixel. Different
   // algorithms may want to access the data with one
   // type of name. In particular, indexedColor and
   // tuple coordinates have some overlapping names for 
   // all Pixel types, which could allow certain algorithms
   // to operate generically.
   union {
      Channels indexedColor;
      RGBA     namedColor;
      Tuple    tuple;
   };

   enum {
      RED_CHANNEL    = 0,
      GREEN_CHANNEL, // 1
      BLUE_CHANNEL,  // 2
      ALPHA_CHANNEL, // 3
      MAX_CHANNELS   // 4
   };

   RGBAPixel() :
      namedColor()
   {}

   bool operator==(const RGBAPixel& that) {
      // TODO: this type of comparison isn't appropriate
      // for floating point types...
      // Probably need to add a comparison traits for ChannelT
      return namedColor.red == that.namedColor.red &&
             namedColor.green == that.namedColor.green &&
             namedColor.blue == that.namedColor.blue &&
             namedColor.alpha == that.namedColor.alpha;
   }

   bool operator!=(const RGBAPixel& that) {
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
template <class T> struct is_rgba<RGBAPixel<T> >            : public std::true_type{};


template<typename ChannelT,typename ChannelTraitsT>
std::ostream& operator<<(std::ostream& os,const RGBAPixel<ChannelT,ChannelTraitsT>& pixel) {
   typedef typename AccumulatorVariableSelect<RGBAPixel<ChannelT,ChannelTraitsT> >::type ResoluteType;
   os << "Color(" 
      << (ResoluteType)pixel.namedColor.red << ","
      << (ResoluteType)pixel.namedColor.green << ","
      << (ResoluteType)pixel.namedColor.blue << ","
      << (ResoluteType)pixel.namedColor.alpha << ")";

   return os;
}

// Note, values of Saturation are assumed to be "normalized" or (0,1)
// over the entire space. The unnormalized value which is computed
// from the rgba2hsi, needs to be compensated before storage into
// this class. Also, hsi2rgba similarly needs to be compensated
// before being applied.
template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct HSIPixel {
   
   typedef ChannelT value_type;
   typedef ChannelTraitsT traits;

   typedef ChannelT Channels[4];
   
   struct Tuple {
      ChannelT value0;
      ChannelT value1;
      ChannelT value2;
   };

   struct HSI {
      ChannelT hue;
      ChannelT saturation;
      ChannelT intensity;
#if __cplusplus >= 201103L
      HSI() :
         hue(0),
         saturation(0),
         intensity(0)
      {}
#endif
   };
   
   // The concept of this union is to have many aliases
   // for the data values of this Pixel. Different
   // algorithms may want to access the data with one
   // type of name. In particular, indexedColor and
   // tuple coordinates have some overlapping names for 
   // all Pixel types, which could allow certain algorithms
   // to operate generically.
   union {
      Channels indexedColor;
      HSI      namedColor;
      Tuple    tuple;
   };

   enum {
      HUE_CHANNEL         = 0,
      SATURATION_CHANNEL, // 1
      INTENSITY_CHANNEL,  // 2
      MAX_CHANNELS        // 3
   };

   HSIPixel() :
      namedColor()
   {}

   // implicit conversion from RGBAPixel
   template<typename ChannelTT,typename ChannelTTTraits>
   HSIPixel(const RGBAPixel<ChannelTT,ChannelTTTraits>& rgba);

   // also provide implicit conversion to RGBAPixel via conversion cast
   template<typename ChannelTT,typename ChannelTTTraits>
   operator RGBAPixel<ChannelTT,ChannelTTTraits>() const;

   bool operator==(const HSIPixel& that) {
      return namedColor.hue == that.namedColor.hue &&
             namedColor.saturation == that.namedColor.saturation &&
             namedColor.intensity == that.namedColor.intensity &&
             namedColor.alpha == that.namedColor.alpha;
   }

   bool operator!=(const HSIPixel& that) {
      return !(*this == that);
   }
};

template<typename ChannelT,typename ChannelTraitsT>
template<typename ChannelTT,typename ChannelTTTraits>
HSIPixel<ChannelT,ChannelTraitsT>::HSIPixel(const RGBAPixel<ChannelTT,ChannelTTTraits>& rgba) :
   namedColor() {
   rgba2hsi(rgba,*this);
}

template<typename ChannelT,typename ChannelTraitsT>
template<typename ChannelTT,typename ChannelTTTraits>
HSIPixel<ChannelT,ChannelTraitsT>::operator RGBAPixel<ChannelTT,ChannelTTTraits>() const {
   RGBAPixel<ChannelTT,ChannelTTTraits> rgba;
   hsi2rgba(*this,rgba);
   return rgba;
}


template <class T> struct is_hsi                            : public std::false_type{};
template <class T> struct is_hsi<const T >                  : public is_hsi<T>{};
template <class T> struct is_hsi<volatile const T >         : public is_hsi<T>{};
template <class T> struct is_hsi<volatile T >               : public is_hsi<T>{};
template <class T> struct is_hsi<T& >                       : public is_hsi<T>{};
template <class T> struct is_hsi<const T& >                 : public is_hsi<T>{};
template <class T> struct is_hsi<volatile const T& >        : public is_hsi<T>{};
template <class T> struct is_hsi<volatile T& >              : public is_hsi<T>{};
// TODO: does this need to consider the traits class? O.w. my have inadvertant false types, if overloaded traits?
template <class T> struct is_hsi<HSIPixel<T> >              : public std::true_type{};



template<typename ChannelT,typename ChannelTraitsT>
std::ostream& operator<<(std::ostream& os,const HSIPixel<ChannelT,ChannelTraitsT>& pixel) {
   typedef typename AccumulatorVariableSelect<RGBAPixel<ChannelT,ChannelTraitsT> >::type ResoluteType;
   os << "HSI_Color(" 
      << (ResoluteType)pixel.namedColor.hue << ","
      << (ResoluteType)pixel.namedColor.saturation << ","
      << (ResoluteType)pixel.namedColor.intensity << ")";

   return os;
}


template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct GrayAlphaPixel {
   
   typedef ChannelT value_type;
   typedef ChannelTraitsT traits;

   typedef ChannelT Channels[2];
   
   struct Tuple {
      ChannelT value0;
      ChannelT value1;
   };

   struct GrayAlpha {
      ChannelT gray;
      ChannelT alpha;
#if __cplusplus >= 201103L
      GrayAlpha() :
         gray(0),
         alpha(0)
      {}
#endif
   };
   
   // The concept of this union is to have many aliases
   // for the data values of this Pixel. Different
   // algorithms may want to access the data with one
   // type of name. In particular, indexedColor and
   // tuple coordinates have some overlapping names for 
   // all Pixel types, which could allow certain algorithms
   // to operate generically.
   union {
      Channels  indexedColor;
      GrayAlpha namedColor;
      Tuple     tuple;
   };

   enum {
      GRAY_CHANNEL    = 0,
      ALPHA_CHANNEL,  // 1
      MAX_CHANNELS    // 2
   };

   GrayAlphaPixel() :
      namedColor()
   {}

   GrayAlphaPixel(ChannelT gray,ChannelT alpha = 0) {
      namedColor.gray = gray;
      namedColor.alpha = alpha;
   }

   // implicit conversion from RGBAPixel
   template<typename ChannelTT,typename ChannelTTTraits>
   GrayAlphaPixel(const RGBAPixel<ChannelTT,ChannelTTTraits>& rgba);

   // also provide implicit conversion to RGBAPixel via conversion cast
   template<typename ChannelTT,typename ChannelTTTraits>
   operator RGBAPixel<ChannelTT,ChannelTTTraits>() const;


   bool operator==(const GrayAlphaPixel& that) {
      return namedColor.gray == that.namedColor.gray &&
             namedColor.alpha == that.namedColor.alpha;
   }

   bool operator!=(const GrayAlphaPixel& that) {
      return !(*this == that);
   }
};


template<typename ChannelT,typename ChannelTraitsT>
template<typename ChannelTT,typename ChannelTTTraits>
GrayAlphaPixel<ChannelT,ChannelTraitsT>::GrayAlphaPixel(const RGBAPixel<ChannelTT,ChannelTTTraits>& rgba) :
   namedColor() {
   rgba2gray(rgba,*this);
}

template<typename ChannelT,typename ChannelTraitsT>
template<typename ChannelTT,typename ChannelTTTraits>
GrayAlphaPixel<ChannelT,ChannelTraitsT>::operator RGBAPixel<ChannelTT,ChannelTTTraits>() const {
   RGBAPixel<ChannelTT,ChannelTTTraits> rgba;
   gray2rgba(*this,rgba);
   return rgba;
}



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
   typedef typename AccumulatorVariableSelect<RGBAPixel<ChannelT,ChannelTraitsT> >::type ResoluteType;
   os << "Grayscale(" 
      << (ResoluteType)pixel.namedColor.gray << ","
      << (ResoluteType)pixel.namedColor.alpha << ")";

   return os;
}



template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct MonochromePixel {

   typedef ChannelT value_type;
   typedef ChannelTraitsT traits;

   typedef ChannelT Channels[1];
   
   struct Tuple {
      ChannelT value0;
   };

   struct Mono {
      ChannelT mono;
#if __cplusplus >= 201103L
      Mono() :
         mono(0)
      {}
#endif
   };
   
   // The concept of this union is to have many aliases
   // for the data values of this Pixel. Different
   // algorithms may want to access the data with one
   // type of name. In particular, indexedColor and
   // tuple coordinates have some overlapping names for 
   // all Pixel types, which could allow certain algorithms
   // to operate generically.
   union {
      Channels indexedColor;
      Mono     namedColor;
      Tuple    tuple;
   };

   enum {
      GRAY_CHANNEL = 0,
      MONO_CHANNEL = GRAY_CHANNEL,
      MAX_CHANNELS // 1
   };

   MonochromePixel() :
      namedColor()
   {}

   MonochromePixel(ChannelT mono) {
      namedColor.mono = mono;
   }

   // implicit conversion from RGBAPixel
   template<typename ChannelTT,typename ChannelTTTraits>
   MonochromePixel(const GrayAlphaPixel<ChannelTT,ChannelTTTraits>& gray);

   // also provide implicit conversion to GrayAlphaPixel via conversion cast
   template<typename ChannelTT,typename ChannelTTTraits>
   operator GrayAlphaPixel<ChannelTT,ChannelTTTraits>() const;

   bool operator==(const MonochromePixel& that) {
      return namedColor.mono == that.namedColor.mono;
   }

   bool operator!=(const MonochromePixel& that) {
      return !(*this == that);
   }

   // explicit conversion from RGBAPixel channel
   template<template<typename,typename> class PixelT,typename ChannelTT,typename ChannelTTTraits>
   MonochromePixel(const PixelT<ChannelTT,ChannelTTTraits>& rgba,unsigned channel);

   // explicit conversion from RGBAPixel channel
   template<template<typename,typename> class PixelT,typename ChannelTT,typename ChannelTTTraits>
   MonochromePixel(const PixelT<ChannelTT,ChannelTTTraits>& mono);
};

template<typename ChannelT,typename ChannelTraitsT>
template<typename ChannelTT,typename ChannelTTTraits>
MonochromePixel<ChannelT,ChannelTraitsT>::MonochromePixel(const GrayAlphaPixel<ChannelTT,ChannelTTTraits>& gray) :
   namedColor() {
   channel2mono(gray,*this,GRAY_CHANNEL);
}

template<typename ChannelT,typename ChannelTraitsT>
template<typename ChannelTT,typename ChannelTTTraits>
MonochromePixel<ChannelT,ChannelTraitsT>::operator GrayAlphaPixel<ChannelTT,ChannelTTTraits>() const {
   GrayAlphaPixel<ChannelTT,ChannelTTTraits> gray;
   channel2mono(*this,gray,MONO_CHANNEL);
   return gray;
}


template<typename ChannelT,typename ChannelTraitsT>
template<template<typename,typename> class PixelT,typename ChannelTT,typename ChannelTTTraits>
MonochromePixel<ChannelT,ChannelTraitsT>::MonochromePixel(const PixelT<ChannelTT,ChannelTTTraits>& pixel,unsigned channel) :
   namedColor() {
   channel2mono(pixel,*this,channel);
}

template<typename ChannelT,typename ChannelTraitsT>
template<template<typename,typename> class PixelT,typename ChannelTT,typename ChannelTTTraits>
MonochromePixel<ChannelT,ChannelTraitsT>::MonochromePixel(const PixelT<ChannelTT,ChannelTTTraits>& pixel) :
   namedColor() {
   channel2mono(pixel, *this, GRAY_CHANNEL);
}

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
   typedef typename AccumulatorVariableSelect<RGBAPixel<ChannelT,ChannelTraitsT> >::type ResoluteType;
   os << "Monochrome(" 
      << (ResoluteType)pixel.namedColor.mono << ")";

   return os;
}


//template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
//struct ParametricGrayAlphaPixel {
//   
//   typedef ChannelT value_type;
//   typedef ChannelTraitsT traits;
//
//   typedef ChannelT Channels[2];
//   
//   struct GA {
//      ChannelT gray;
//      ChannelT alpha;
//#if __cplusplus >= 201103L
//      GA() :
//         gray(0),
//         alpha(0)
//      {}
//#endif
//   };
//
//   union {
//      Channels indexedColor;
//      GA namedColor;
//   };
//
//   unsigned row;
//   unsigned col;
//
//   enum {
//      GRAY_CHANNEL   = 0,
//      ALPHA_CHANNEL, // 1
//      MAX_CHANNELS   // 2
//   };
//
//   ParametricGrayAlphaPixel() :
//      namedColor()
//   {}
//
//   bool operator==(const ParametricGrayAlphaPixel& that) {
//      return namedColor.gray == that.namedColor.gray &&
//             namedColor.alpha == that.namedColor.alpha;
//   }
//
//   bool operator!=(const ParametricGrayAlphaPixel& that) {
//      return !(*this == that);
//   }
//};

template<template<typename,typename> class PixelT,
         typename ChannelT,
         typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct ParametricPixel : public PixelT<ChannelT,ChannelTraitsT> {
   typedef PixelT<ChannelT,ChannelTraitsT> SuperT;
   typedef ChannelT value_type;
   typedef ChannelTraitsT traits;

   unsigned mRow;
   unsigned mCol;

   ParametricPixel() :
      mRow(0), mCol(0)
   {}

   bool operator==(const ParametricPixel& that) {
      return SuperT::operator==(that) &&
             mRow == that.mRow &&
             mCol == that.mCol;
   }

   bool operator!=(const ParametricPixel& that) {
      return !(*this == that);
   }
};




} // namespace types
} // namespace batchIP

