#pragma once

#include "Pixel.h"
#include "cppTools/TemplateMetaprogramming.h"

template<typename ImageT,
         bool = is_uint8<typename ImageT::pixel_type::value_type>::value>
struct PixelReader {
   typedef typename ImageT::pixel_type PixelT;
private:
   PixelReader();
public:

   template<typename BufferT> 
   static void readRGBPixels(ImageT& image,const BufferT& buffer) {
   
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
   static void readGrayPixels(ImageT& image,const BufferT& buffer) {
   
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
   static void readRGBPixels(ImageT& image,const BufferT& buffer) {
   
      typename ImageT::iterator tpos = image.begin();
      typename ImageT::iterator tend = image.end();
      typename BufferT::const_iterator spos = buffer.begin();
   
      for(;tpos != tend;++tpos) {
         RGBAPixel<uint8_t> pixel;
         pixel.namedColor.red = *spos; ++spos;
         pixel.namedColor.green = *spos; ++spos;
         pixel.namedColor.blue = *spos; ++spos;
         *tpos = pixel;
      }
   }
   
   template<typename BufferT> 
   static void readGrayPixels(ImageT& image,const BufferT& buffer) {
   
      typename ImageT::iterator tpos = image.begin();
      typename ImageT::iterator tend = image.end();
      typename BufferT::const_iterator spos = buffer.begin();
   
      for(;tpos != tend;++tpos) {
         GrayAlphaPixel<uint8_t> pixel;
         pixel.namedColor.gray = *spos; ++spos;
         *tpos = pixel;
      }
   }
}; // struct PixelReader


