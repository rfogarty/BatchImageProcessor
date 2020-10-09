#pragma once

#include "Pixel.h"
#include "cppTools/TemplateMetaprogramming.h"

namespace batchIP {
namespace io {

template<typename ImageT,
         bool = stdesque::is_uint8<typename ImageT::pixel_type::value_type>::value>
struct PixelWriter {
   typedef typename ImageT::pixel_type PixelT;
private:
   PixelWriter();
public:

   static void writeRGBPixels(std::ostream& ostr,const ImageT& image) {
   
      typename ImageT::const_iterator pos = image.begin();
      typename ImageT::const_iterator end = image.end();
   
      for(;pos != end;++pos) {
         ostr << pos->namedColor.red;
         ostr << pos->namedColor.green;
         ostr << pos->namedColor.blue;
      }
   }
   
   static void writeGrayPixels(std::ostream& ostr,const ImageT& image,unsigned channel) {
   
      typename ImageT::const_iterator pos = image.begin();
      typename ImageT::const_iterator end = image.end();
   
      for(;pos != end;++pos) ostr << pos->indexedColor[channel];
   }

}; // PixelWriter

template<typename ImageT>
struct PixelWriter<ImageT,false> {
   typedef typename ImageT::pixel_type PixelT;
private:
   PixelWriter();
public:

   static void writeRGBPixels(std::ostream& ostr,const ImageT& image) {
   
      typename ImageT::const_iterator pos = image.begin();
      typename ImageT::const_iterator end = image.end();
   
      for(;pos != end;++pos) {
         types::RGBAPixel<uint8_t> rgba(*pos);
         ostr << rgba.namedColor.red;
         ostr << rgba.namedColor.green;
         ostr << rgba.namedColor.blue;
      }
   }
   
   
   static void writeGrayPixels(std::ostream& ostr,const ImageT& image,unsigned channel) {
   
      typename ImageT::const_iterator pos = image.begin();
      typename ImageT::const_iterator end = image.end();
      typedef types::ChannelTraits<uint8_t> OutTraitsT;
   
      for(;pos != end;++pos) {
         uint8_t mono = ((double)pos->indexedColor[channel] - PixelT::traits::min())*(double)OutTraitsT::max()/PixelT::traits::max();
         ostr << mono;
      }
   }
}; // PixelWriter


} // namespace io
} // namespace batchIP

