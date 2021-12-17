#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <boost/gil/io/read_image_info.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/extension/io/pnm.hpp>
#include <boost/gil/extension/io/tiff.hpp>
#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/extension/io/bmp.hpp>
#include <string>
#include <sstream>
#include <stdexcept>

namespace batchIP {
namespace io {

   template<typename InfoType>
   bool isImageGrayscale1(const std::string& filename,const InfoType& typetag) {
      auto reader = boost::gil::read_image_info(filename,typetag);
      return reader._info._num_channels == 1;
   }

   template<typename InfoType>
   bool isImageGrayscale2(const std::string& filename,const InfoType& typetag) {
      auto reader = boost::gil::read_image_info(filename,typetag);
      return reader._info._num_components == 1;
   }

   template<typename InfoType>
   bool isImageGrayscale3(const std::string& filename,const InfoType& typetag) {
      auto reader = boost::gil::read_image_info(filename,typetag);
      return reader._info._samples_per_pixel == 1;
   }


   bool isImageGrayscale(const std::string& filename) {
      if(utility::endsWith(filename, ".ppm")) return false;
      if(utility::endsWith(filename, ".pgm"))  return true;
      //if(utility::endsWith(filename, ".bmp"))  return isImageGrayscale1(filename,boost::gil::bmp_tag());
      if(utility::endsWith(filename, ".png"))  return isImageGrayscale1(filename,boost::gil::png_tag());  
      //if(utility::endsWith(filename, ".webp")) return isImageColor1(filename,boost::gil::webp_tag()); 
      if(utility::endsWith(filename, ".jpg") ||  
         utility::endsWith(filename, ".jpeg")|| 
         utility::endsWith(filename, ".jp2"))  return isImageGrayscale2(filename,boost::gil::jpeg_tag());  
      if(utility::endsWith(filename, ".tif") || 
         utility::endsWith(filename, ".tiff")) return isImageGrayscale3(filename,boost::gil::tiff_tag());
      // o.w.
      std::stringstream ss;
      ss << "Unsupported file type: " << filename;
      throw std::invalid_argument(ss.str().c_str());
   }

} // namespace io
} // namespace batchIP

#pragma GCC diagnostic pop

