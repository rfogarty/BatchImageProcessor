#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "cppTools/Platform.h"
#include "Channel.h"
#include "PixelConversion.h"
#include <ostream>

template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct RGBAPixel {
   
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
   
   struct HSI {
      ChannelT hue;
      ChannelT saturation;
      ChannelT intensity;
      ChannelT alpha;
#if __cplusplus >= 201103L
      HSI() :
         hue(0),
         saturation(0),
         intensity(0),
         alpha(0)
      {}
#endif
   };
   
   union {
      Channels indexedColor;
      HSI      namedColor;
   };

   enum {
      HUE_CHANNEL         = 0,
      SATURATION_CHANNEL, // 1
      INTENSITY_CHANNEL,  // 2
      ALPHA_CHANNEL,      // 3
      MAX_CHANNELS        // 4
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
      << (ResoluteType)pixel.namedColor.intensity << ","
      << (ResoluteType)pixel.namedColor.alpha << ")";

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
      GRAY_CHANNEL    = 0,
      ALPHA_CHANNEL,  // 1
      MAX_CHANNELS    // 2
   };

   GrayAlphaPixel() :
      namedColor()
   {}

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
      MONO_CHANNEL = 0,
      MAX_CHANNELS // 1
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

   // implicit conversion from RGBAPixel
   template<unsigned channel,template<typename,typename> class PixelT,typename ChannelTT,typename ChannelTTTraits>
   explicit MonochromePixel(const PixelT<ChannelTT,ChannelTTTraits>& rgba);
};


template<typename ChannelT,typename ChannelTraitsT>
template<unsigned channel,template<typename,typename> class PixelT,typename ChannelTT,typename ChannelTTTraits>
MonochromePixel<ChannelT,ChannelTraitsT>::MonochromePixel(const PixelT<ChannelTT,ChannelTTTraits>& pixel) :
   namedColor() {
   channel2mono(pixel,*this,channel);
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


template<typename ChannelT,typename ChannelTraitsT = ChannelTraits<ChannelT> >
struct ParametricGrayAlphaPixel {
   
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

   unsigned row;
   unsigned col;

   enum {
      GRAY_CHANNEL   = 0,
      ALPHA_CHANNEL, // 1
      MAX_CHANNELS   // 2
   };

   ParametricGrayAlphaPixel() :
      namedColor()
   {}

   bool operator==(const ParametricGrayAlphaPixel& that) {
      return namedColor.gray == that.namedColor.gray &&
             namedColor.alpha == that.namedColor.alpha;
   }

   bool operator!=(const ParametricGrayAlphaPixel& that) {
      return !(*this == that);
   }
};


