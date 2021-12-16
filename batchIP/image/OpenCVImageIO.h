#include "image/Image.h"
#include "image/Pixel.h"
#include "image/PixelReader.h"
#include "image/PixelWriter.h"
#include "cppTools/Platform.h"
#include <opencv2/imgcodecs.hpp>


namespace batchIP {
namespace io {

template<typename ImageT>
struct CVConverter {
   typedef typename ImageT::pixel_type PixelT;
private:
   CVConverter();

   template<typename ElementT,typename CVMatT> 
   static void convertRGB(ImageT& image,const CVMatT& cvImage) {

      typename ImageT::iterator tpos = image.begin();
      //typename ImageT::iterator tend = image.end();
      
      types::RGBAPixel<ElementT> tpixel;
      for (int i = 0; i < cvImage.rows; ++i)
      {
         const ElementT* pixel = cvImage.template ptr<ElementT>(i);  // point to first pixel in row
         for (int j = 0; j < cvImage.cols; ++j)
         {
            tpixel.namedColor.blue = *pixel++;
            tpixel.namedColor.green = *pixel++;
            tpixel.namedColor.red = *pixel++;
            *tpos = tpixel; // I believe this should do implicit conversion if necessary
            ++tpos;
         }
      }
   }

   template<typename ElementT,typename CVMatT> 
   static void convertGray(ImageT& image,const CVMatT& cvImage) {

      typename ImageT::iterator tpos = image.begin();
      //typename ImageT::iterator tend = image.end();
      
      types::GrayAlphaPixel<ElementT> tpixel;
      for (int i = 0; i < cvImage.rows; ++i)
      {
         const ElementT* pixel = cvImage.template ptr<ElementT>(i);  // point to first pixel in row
         for (int j = 0; j < cvImage.cols; ++j)
         {
            tpixel.namedColor.gray = *pixel++;
            *tpos = tpixel; // I believe this should do implicit conversion if necessary
            ++tpos;
         }
      }
   }


public:
   template<typename CVMatT> 
   static void copyRGB(ImageT& image,const CVMatT& cvImage) {

      if(cvImage.depth() == CV_8U) {
         convertRGB<uint8_t>(image,cvImage);
      }
      else if(cvImage.depth() == CV_16U) {
         convertRGB<uint16_t>(image,cvImage);
      }
      else if(cvImage.depth() == CV_32F) {
         convertRGB<float>(image,cvImage);
      }
      else if(cvImage.depth() == CV_64F) {
         convertRGB<double>(image,cvImage);
      }
      // TODO: don't believe I support signed types right now
      //else if(cvImage.depth() == CV_8S) {
      //}
      //else if(cvImage.depth() == CV_16S) {
      //}
      //else if(cvImage.depth() == CV_32S) {
      //}
      else {
         std::stringstream ss;
         ss << "OpenCV depth is unsupported: " << cvImage.depth();
         throw std::invalid_argument(ss.str().c_str());
      }
   }

   template<typename CVMatT> 
   static void copyGray(ImageT& image,const CVMatT& cvImage) {

      if(cvImage.depth() == CV_8U) {
         convertGray<uint8_t>(image,cvImage);
      }
      else if(cvImage.depth() == CV_16U) {
         convertGray<uint16_t>(image,cvImage);
      }
      else if(cvImage.depth() == CV_32F) {
         convertGray<float>(image,cvImage);
      }
      else if(cvImage.depth() == CV_64F) {
         convertGray<double>(image,cvImage);
      }
      // TODO: don't believe I support signed types right now
      //else if(cvImage.depth() == CV_8S) {
      //}
      //else if(cvImage.depth() == CV_16S) {
      //}
      //else if(cvImage.depth() == CV_32S) {
      //}
      else {
         std::stringstream ss;
         ss << "OpenCV depth is unsupported: " << cvImage.depth();
         throw std::invalid_argument(ss.str().c_str());
      }
   }
 
}; // struct CVConverter



bool isImageFile(const std::string& filename) {
   return (utility::endsWith(filename, ".ppm")  ||
           utility::endsWith(filename, ".pgm")  || 
           utility::endsWith(filename, ".bmp")  || 
           utility::endsWith(filename, ".png")  || 
           utility::endsWith(filename, ".webp") || 
           utility::endsWith(filename, ".jpg")  || 
           utility::endsWith(filename, ".jpeg") || 
           utility::endsWith(filename, ".jp2")  || 
           utility::endsWith(filename, ".tif")  || 
           utility::endsWith(filename, ".tiff"));
}

template<typename PixelT> 
types::Image<PixelT> readColorFile(const std::string& filename) {
   if (!isImageFile(filename)) {
      std::stringstream ss;
      ss << "Unsupported file type: " << filename;
      throw std::invalid_argument(ss.str().c_str());
   }

   cv::Mat cvImage = cv::imread(filename,cv::IMREAD_UNCHANGED);
   unsigned rows = cvImage.rows;
   unsigned cols = cvImage.cols;
   
   // Copy all the image buffer data into the Image<PixelT> object
   typedef types::Image<PixelT> ImageT;

   ImageT image(rows,cols);

   CVConverter<ImageT>::copyRGB(image,cvImage);
   
   return image;
}

template<typename PixelT> 
types::Image<PixelT> readGrayscaleFile(const std::string& filename) {
   if (!isImageFile(filename)) {
      std::stringstream ss;
      ss << "Unsupported file type: " << filename;
      throw std::invalid_argument(ss.str().c_str());
   }

   cv::Mat cvImage = cv::imread(filename,cv::IMREAD_GRAYSCALE);
   unsigned rows = cvImage.rows;
   unsigned cols = cvImage.cols;
   
   // Copy all the image buffer data into the Image<PixelT> object
   typedef types::Image<PixelT> ImageT;

   ImageT image(rows,cols);

   CVConverter<ImageT>::copyGray(image,cvImage);
   
   return image;
}



} // namespace io
} // namespace batchIP

