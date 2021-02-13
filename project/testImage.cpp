/************************************************************
 * iptool.cpp - a simple batch processing tool for 
 *              experimenting with Image Processing
 *
 ************************************************************/

#include "image/Image.h"
#include "image/NetpbmImage.h"
#include "image/Pixel.h"
#include "image/ImageAlgorithm.h"
#include "utility/Error.h"
#include <exception>
#include <iostream>
#include <sstream>

using namespace batchIP;
using namespace batchIP::io;
using namespace batchIP::types;
using namespace batchIP::utility;
using namespace batchIP::algorithm;

class ExpectedError : public std::exception {
private:
   std::string mMessage;
public:
   explicit ExpectedError(const std::string& msg) : mMessage(msg) {}

   virtual ~ExpectedError() throw() {}

   virtual const char* what() const throw() {
      return mMessage.c_str();
   }
};



void createGrayscaleImage() {
   
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;

   ImageT image(1000u,1200u);
   reportIfNotEqual("rows",image.rows(),1000u);
   reportIfNotEqual("cols",image.cols(),1200u);
}

void copyConstructGrayscaleImages() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;

   ImageT image1(1000u,1200u);
   PixelT& pixel1 = image1.pixel(500,600);
   pixel1.namedColor.gray = 125;
   pixel1.namedColor.alpha = 10;
   ImageT image2(image1);

   reportIfNotEqual("rows",image2.rows(),1000u);
   reportIfNotEqual("cols",image2.cols(),1200u);
   reportIfNotEqual("pixels",image1.pixel(500,600),image2.pixel(500,600));
}

void assignGrayscaleImages() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;

   ImageT image1(1000u,1200u);
   PixelT& pixel1 = image1.pixel(500,600);
   pixel1.namedColor.gray = 125;
   pixel1.namedColor.alpha = 10;
   
   ImageT image2(50,50);
   image2 = image1;

   reportIfNotEqual("rows",image2.rows(),1000u);
   reportIfNotEqual("cols",image2.cols(),1200u);
   reportIfNotEqual("pixels",image1.pixel(500,600),image2.pixel(500,600));
}

void makeProperGrayscaleView() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(500u,600u);
   view.resize(600,700);
}

void makeImproperGrayscaleView() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   try {
      ViewT view = image.view(1001u,1201u);
      // If correct, we should NOT get to the next two lines.
      std::cout << "Improper size shouldn't print " << view.size() << std::endl;
      throw ExpectedError("Expected view creation to be out of range");
   } catch(const std::out_of_range& oor) {}
}


void moveGrayscaleView() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(50u,50u);
   view.move(0u,1200u-50u);
   view.move(1000u-50u,0u);
   view.move(1000u-50u,1200u-50u);
   try {
      view.move(995u,1195u);
      throw ExpectedError("Expected view.move to be out of range");
   } catch(const std::out_of_range& oor) {}
}

void makeProperGrayscaleSubview() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(500u,600u);
   view.resize(600u,600u);

   ViewT subview = view.view(300u,300u,300u,300u);

   PixelT& subpixel = subview.pixel(0,0);
   subpixel.namedColor.gray = 125;
   subpixel.namedColor.alpha = 10;

   // Grab a copy so we compare to the copy below
   PixelT pixel = view.pixel(300,300);
   
   reportIfNotEqual("pixel and subpixel",subpixel,pixel);
}

void makeImproperGrayscaleSubview() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(500u,600u);
   view.resize(600u,600u);

   try {
      ViewT subview = view.view(301u,300u,300u,300u);
      throw ExpectedError("Expected view.subview to be out of range");
   } catch(const std::out_of_range& oor) {}
   try {
      ViewT subview = view.view(300u,301u,300u,300u);
      throw ExpectedError("Expected view.subview to be out of range");
   } catch(const std::out_of_range& oor) {}
}

void moveGrayscaleSubview() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(500u,600u);
   view.resize(600u,600u);

   ViewT subview = view.view(300u,300u,300u,300u);
   subview.move(0u,300u);
   subview.move(300u,0u);
   subview.move(0u,0u);
   try {
      subview.move(301u,0u);
      throw ExpectedError("Expected view.move rows to be out of range");
   } catch(const std::out_of_range& oor) {}
   try {
      subview.move(0u,301u);
      throw ExpectedError("Expected view.move cols to be out of range");
   } catch(const std::out_of_range& oor) {}
}

void testViewIteratorGrayscale() {
   typedef GrayAlphaPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef ImageT::image_view ViewT;
   typedef ViewT::iterator IteratorT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(300u,300u,100u,200u);

   IteratorT pos = view.begin();
   IteratorT end = view.end();
   unsigned iterations = 0;
   unsigned size = view.size();
   for(;pos != end;++pos,++iterations) {
      pos->namedColor.gray = (uint8_t)((double)iterations*255.0/size);
   }
   reportIfNotEqual("view.size() == iterations",iterations,view.size());

   writePGMFile<PixelT::GRAY_CHANNEL>("UnitTestGrayscale1.pgm",image);
}


#if 0
void testElasticViewGrayscale() {
   typedef GrayAlphaPixel<uint16_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;
   typedef typename ViewT::iterator IteratorT;

   ImageT image(15u,15u);

   IteratorT pos = image.begin();
   IteratorT end = image.end();
   for(unsigned i = 0;pos != end;++pos,++i) {
   }

   ViewT view = image.view(300u,300u,100u,200u);

   IteratorT pos = view.begin();
   IteratorT end = view.end();
   unsigned iterations = 0;
   unsigned size = view.size();
   for(;pos != end;++pos,++iterations) {
      pos->namedColor.gray = (uint8_t)((double)iterations*255.0/size);
   }
   reportIfNotEqual("view.size() == iterations",iterations,view.size());


}
#endif

void createColorImage() {

   typedef RGBAPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;

   ImageT image(1000u,1200u);
   reportIfNotEqual("rows",image.rows(),1000u);
   reportIfNotEqual("cols",image.cols(),1200u);
}

void copyConstructColorImages() {
   typedef RGBAPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;

   ImageT image1(1000u,1200u);
   PixelT& pixel1 = image1.pixel(500,600);
   pixel1.namedColor.red = 125;
   pixel1.namedColor.green = 225;
   pixel1.namedColor.blue = 150;
   pixel1.namedColor.alpha = 10;
   ImageT image2(image1);

   reportIfNotEqual("rows",image2.rows(),1000u);
   reportIfNotEqual("cols",image2.cols(),1200u);
   reportIfNotEqual("pixels",image1.pixel(500,600),image2.pixel(500,600));
}

void assignColorImages() {
   typedef RGBAPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;

   ImageT image1(1000u,1200u);
   PixelT& pixel1 = image1.pixel(500,600);
   pixel1.namedColor.red = 125;
   pixel1.namedColor.green = 225;
   pixel1.namedColor.blue = 150;
   pixel1.namedColor.alpha = 10;
   
   ImageT image2(50,50);
   image2 = image1;

   reportIfNotEqual("rows",image2.rows(),1000u);
   reportIfNotEqual("cols",image2.cols(),1200u);
   reportIfNotEqual("pixels",image1.pixel(500,600),image2.pixel(500,600));
}

void moveColorView() {
   typedef RGBAPixel<uint8_t> PixelT;
   typedef Image<PixelT> ImageT;
   typedef ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(50u,50u);
   view.move(0u,1200u-50u);
   view.move(1000u-50u,0u);
   view.move(1000u-50u,1200u-50u);
   try {
      view.move(995,1195);
      throw ExpectedError("Expected view.move to be out of range");
   } catch(const std::out_of_range& oor) {}
}


void testRGBA2HSI(uint8_t r,uint8_t g, uint8_t b) {

   typedef RGBAPixel<uint8_t> RGBAT;
   typedef HSIPixel<float> HSIT;

   RGBAT rgb1;
   rgb1.namedColor.red = r;
   rgb1.namedColor.green = g;
   rgb1.namedColor.blue = b;
   HSIT hsi(rgb1);
   RGBAT rgb2(hsi);
   std::cout << "hsi: " << hsi << std::endl;
   std::cout << "rgb1: " << rgb1 << std::endl;
   std::cout << "rgb2: " << rgb2 << std::endl;
   reportIfNotEqual("rgb1 != rgb2",rgb1,rgb2);
}

void testSobel() {
   typedef float PrecisionT;
   typedef types::Image<types::MonochromePixel<PrecisionT> > KernelT;

   {
      KernelT kernelX;
      KernelT kernelX_new;
      KernelT kernelY;
      KernelT kernelY_new;
      unsigned int windowSize = 5;
      edge::sobel5(kernelX, kernelY);
      edge::sobelX(windowSize,kernelX_new, kernelY_new);

      std::cout << "Sobel" << windowSize << "-X: \n";
      for(unsigned int i = 0; i < windowSize;++i) {
         std::string space = "";
         for(unsigned int j = 0; j < windowSize;++j) {
            std::cout  << space << kernelX.pixel(i, j).tuple.value0;
            space = " ";
         }
         std::cout << "\n";
      }
      std::cout << "\n";

      std::cout << "Sobel" << windowSize << "-X-New: \n";
      for(unsigned int i = 0; i < windowSize;++i) {
         std::string space = "";
         for(unsigned int j = 0; j < windowSize;++j) {
            std::cout  << space << kernelX_new.pixel(i, j).tuple.value0;
            space = " ";
         }
         std::cout << "\n";
      }
      std::cout << "\n";

      std::cout << "Sobel" << windowSize << "-Y: \n";
      for(unsigned int i = 0; i < windowSize;++i) {
         std::string space = "";
         for(unsigned int j = 0; j < windowSize;++j) {
            std::cout  << space << kernelY.pixel(i, j).tuple.value0;
            space = " ";
         }
         std::cout << "\n";
      }
      std::cout << std::endl;

      std::cout << "Sobel" << windowSize << "-Y-New: \n";
      for(unsigned int i = 0; i < windowSize;++i) {
         std::string space = "";
         for(unsigned int j = 0; j < windowSize;++j) {
            std::cout  << space << kernelY_new.pixel(i, j).tuple.value0;
            space = " ";
         }
         std::cout << "\n";
      }
      std::cout << std::endl;

   }

   //          -2/8 -1/5  0  1/5  2/8           -5  -4  0   4   5
   //          -2/5 -1/2  0  1/2  2/5           -8 -10  0  10   8
   //G_x (5x5) -2/4 -1/1  0  1/1  2/4  (*20) = -10 -20  0  20  10
   //          -2/5 -1/2  0  1/2  2/5           -8 -10  0  10   8
   //          -2/8 -1/5  0  1/5  2/8           -5  -4  0   4   5


   //-3/18 -2/13 -1/10 0  1/10 2/13 3/18
   //-3/13 -2/8  -1/5  0  1/5  2/8  3/13
   //-3/10 -2/5  -1/2  0  1/2  2/5  3/10
   //-3/9  -2/4  -1/1  0  1/1  2/4  3/9
   //-3/10 -2/5  -1/2  0  1/2  2/5  3/10
   //-3/13 -2/8  -1/5  0  1/5  2/8  3/13
   //-3/18 -2/13 -1/10 0  1/10 2/13 3/18

   //-3/18 -2/13 -1/10   0/9  1/10  2/13  3/18             -130 -120  -78    0   78  120  130
   //-3/13  -2/8  -1/5   0/4   1/5   2/8  3/13             -180 -195 -156    0  156  195  180
   //-3/10  -2/5  -1/2   0/1   1/2   2/5  3/10             -234 -312 -390    0  390  312  234
   //-3/9  -2/4  -1/1     0   1/1   2/4   3/9   * 780 =   -260 -390 -780    0  780  390  260
   //-3/10  -2/5  -1/2   0/1   1/2   2/5  3/10             -234 -312 -390    0  390  312  234
   //-3/13  -2/8  -1/5   0/4   1/5   2/8  3/13             -180 -195 -156    0  156  195  180
   //-3/18 -2/13 -1/10   0/9  1/10  2/13  3/18             -130 -120  -78    0   78  120  130

   //-4/32 -3/25 -2/20 -1/17  0/16  1/17  2/20  3/25  4/32                -16575  -15912  -13260   -7800       0    7800   13260   15912   16575
   //-4/25 -3/18 -2/13 -1/10   0/9  1/10  2/13  3/18  4/25                -21216  -22100  -20400  -13260       0   13260   20400   22100   21216
   //-4/20 -3/13  -2/8  -1/5   0/4   1/5   2/8  3/13  4/20                -26520  -30600  -33150  -26520       0   26520   33150   30600   26520
   //-4/17 -3/10  -2/5  -1/2   0/1   1/2   2/5  3/10  4/17                -31200  -39780  -53040  -66300       0   66300   53040   39780   31200
   //-4/16  -3/9  -2/4  -1/1     0   1/1   2/4   3/9  4/16   * 132600 =   -33150  -44200  -66300 -132600       0  132600   66300   44200   33150
   //-4/17 -3/10  -2/5  -1/2   0/1   1/2   2/5  3/10  4/17                -31200  -39780  -53040  -66300       0   66300   53040   39780   31200
   //-4/20 -3/13  -2/8  -1/5   0/4   1/5   2/8  3/13  4/20                -26520  -30600  -33150  -26520       0   26520   33150   30600   26520
   //-4/25 -3/18 -2/13 -1/10   0/9  1/10  2/13  3/18  4/25                -21216  -22100  -20400  -13260       0   13260   20400   22100   21216
   //-4/32 -3/25 -2/20 -1/17  0/16  1/17  2/20  3/25  4/32                -16575  -15912  -13260   -7800       0    7800   13260   15912   16575



   {
      KernelT kernelX_new;
      KernelT kernelY_new;
      unsigned int windowSize = 7;
      edge::sobelX(windowSize,kernelX_new, kernelY_new);

      std::cout << "Sobel" << windowSize << "-X-New: \n";
      for(unsigned int i = 0; i < windowSize;++i) {
         std::string space = "";
         for(unsigned int j = 0; j < windowSize;++j) {
            std::cout  << space << kernelX_new.pixel(i, j).tuple.value0;
            space = " ";
         }
         std::cout << "\n";
      }
      std::cout << "\n";

      std::cout << "Sobel" << windowSize << "-Y-New: \n";
      for(unsigned int i = 0; i < windowSize;++i) {
         std::string space = "";
         for(unsigned int j = 0; j < windowSize;++j) {
            std::cout  << space << kernelY_new.pixel(i, j).tuple.value0;
            space = " ";
         }
         std::cout << "\n";
      }
      std::cout << std::endl;
   }

   {
      KernelT kernelX_new;
      KernelT kernelY_new;
      unsigned int windowSize = 9;
      edge::sobelX(windowSize,kernelX_new, kernelY_new);

      std::cout << "Sobel" << windowSize << "-X-New: \n";
      for(unsigned int i = 0; i < windowSize;++i) {
         std::string space = "";
         for(unsigned int j = 0; j < windowSize;++j) {
            std::cout  << space << kernelX_new.pixel(i, j).tuple.value0;
            space = " ";
         }
         std::cout << "\n";
      }
      std::cout << "\n";

      std::cout << "Sobel" << windowSize << "-Y-New: \n";
      for(unsigned int i = 0; i < windowSize;++i) {
         std::string space = "";
         for(unsigned int j = 0; j < windowSize;++j) {
            std::cout  << space << kernelY_new.pixel(i, j).tuple.value0;
            space = " ";
         }
         std::cout << "\n";
      }
      std::cout << std::endl;
   }

}

int main() {

   try {
      createGrayscaleImage();
      copyConstructGrayscaleImages();
      assignGrayscaleImages();
      makeProperGrayscaleView();
      makeImproperGrayscaleView();
      moveGrayscaleView();
      makeProperGrayscaleSubview();
      makeImproperGrayscaleSubview();
      moveGrayscaleSubview();
      testViewIteratorGrayscale();

      createColorImage();
      copyConstructColorImages();
      assignColorImages();
      moveColorView();
      
      testRGBA2HSI(0,0,0);
      
      testRGBA2HSI(255,0,0);
      testRGBA2HSI(0,255,0);
      testRGBA2HSI(0,0,255);
      
      testRGBA2HSI(255,255,0);
      testRGBA2HSI(0,255,255);
      testRGBA2HSI(255,0,255);

      testRGBA2HSI(128,0,0);
      testRGBA2HSI(0,128,0);
      testRGBA2HSI(0,0,128);
      

      testRGBA2HSI(100,100,100);
      testRGBA2HSI(255,255,255);

      testRGBA2HSI(128,100,50);

      testSobel();
   }
   catch(const std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
