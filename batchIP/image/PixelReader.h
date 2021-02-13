#pragma once

#include "Pixel.h"
#include "cppTools/TemplateMetaprogramming.h"

namespace batchIP {
namespace io {

template<typename ImageT,
         bool = stdesque::is_uint8<typename ImageT::pixel_type::value_type>::value>
struct PixelReader {};

template<typename ImageT>
struct PixelReader<ImageT,true> {
   typedef typename ImageT::pixel_type PixelT;
private:
   PixelReader();
public:

   template<typename BufferT> 
   static void readRGBPixels(ImageT& image,const BufferT& buffer,bool=false) {
   
      typename ImageT::iterator tpos = image.begin();
      typename ImageT::iterator tend = image.end();
      typename BufferT::const_iterator spos = buffer.begin();
   
      for(;tpos != tend;++tpos) {
         PixelT& pixel = *tpos;
         pixel.namedColor.red = *spos; ++spos;
         pixel.namedColor.green = *spos; ++spos;
         pixel.namedColor.blue = *spos; ++spos;
      }
   }
   
   template<typename BufferT> 
   static void readGrayPixels(ImageT& image,const BufferT& buffer,bool=false) {
   
      typename ImageT::iterator tpos = image.begin();
      typename ImageT::iterator tend = image.end();
      typename BufferT::const_iterator spos = buffer.begin();
   
      for(;tpos != tend;++tpos) {
         PixelT& pixel = *tpos;
         pixel.namedColor.gray = *spos; ++spos;
      }
   }
}; // PixerReader


template<typename ImageT>
struct PixelReader<ImageT,false> {
   typedef typename ImageT::pixel_type PixelT;
private:
   PixelReader();
public:

   template<typename BufferT> 
   static void readRGBPixels(ImageT& image,const BufferT& buffer,bool endianSwap=false) {
   
      typename ImageT::iterator tpos = image.begin();
      typename ImageT::iterator tend = image.end();
      typename BufferT::const_iterator spos = buffer.begin();

      if(endianSwap) {
         for(;tpos != tend;++tpos) {
            types::RGBAPixel<typename BufferT::value_type> pixel;
            pixel.namedColor.red = stdesque::Endian<typename BufferT::value_type>::swap(*spos); ++spos;
            pixel.namedColor.green = stdesque::Endian<typename BufferT::value_type>::swap(*spos); ++spos;
            pixel.namedColor.blue = stdesque::Endian<typename BufferT::value_type>::swap(*spos); ++spos;
            *tpos = pixel;
         }
      }
      else {
         for(;tpos != tend;++tpos) {
            types::RGBAPixel<typename BufferT::value_type> pixel;
            pixel.namedColor.red = *spos; ++spos;
            pixel.namedColor.green = *spos; ++spos;
            pixel.namedColor.blue = *spos; ++spos;
            *tpos = pixel;
         }
      }
   }
   
   template<typename BufferT> 
   static void readGrayPixels(ImageT& image,const BufferT& buffer,bool endianSwap=false) {
   
      typename ImageT::iterator tpos = image.begin();
      typename ImageT::iterator tend = image.end();
      typename BufferT::const_iterator spos = buffer.begin();

      if(endianSwap) {
         for (; tpos != tend; ++tpos) {
            types::GrayAlphaPixel<typename BufferT::value_type> pixel;
            pixel.namedColor.gray = stdesque::Endian<typename BufferT::value_type>::swap(*spos);
            ++spos;
            *tpos = pixel;
         }
      }
      else {
         for (; tpos != tend; ++tpos) {
            types::GrayAlphaPixel<typename BufferT::value_type> pixel;
            pixel.namedColor.gray = *spos;
            ++spos;
            *tpos = pixel;
         }
      }
   }
}; // struct PixelReader


} // namespace io
} // namespace batchIP

