/************************************************************
 * iptool.cpp - a simple batch processing tool for 
 *              experimenting with Image Processing
 *
 ************************************************************/

#include "image/Image.h"
#include "image/NetpbmImage.h"
#include "image/Pixel.h"
#include "utility/Error.h"
#include <exception>
#include <iostream>
#include <sstream>

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
   
   typedef GrayAlphaPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;

   ImageT image(1000u,1200u);
   reportIfNotEqual("rows",image.rows(),1000u);
   reportIfNotEqual("cols",image.cols(),1200u);
}

void copyConstructGrayscaleImages() {
   typedef GrayAlphaPixel<unsigned char> PixelT;
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
   typedef GrayAlphaPixel<unsigned char> PixelT;
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
   typedef GrayAlphaPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(500u,600u);
   view.resize(600,700);
}

void makeImproperGrayscaleView() {
   typedef GrayAlphaPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;

   ImageT image(1000u,1200u);

   try {
      ViewT view = image.view(1001u,1201u);
      // If correct, we should NOT get to the next two lines.
      std::cout << "Improper size shouldn't print" << view.size() << std::endl;
      throw ExpectedError("Expected view creation to be out of range");
   } catch(const std::out_of_range& oor) {}
}


void moveGrayscaleView() {
   typedef GrayAlphaPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;

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
   typedef GrayAlphaPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;

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
   typedef GrayAlphaPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;

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
   typedef GrayAlphaPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;

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
   typedef GrayAlphaPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;
   typedef typename ViewT::iterator IteratorT;

   ImageT image(1000u,1200u);

   ViewT view = image.view(300u,300u,100u,200u);

   IteratorT pos = view.begin();
   IteratorT end = view.end();
   unsigned iterations = 0;
   unsigned size = view.size();
   for(;pos != end;++pos,++iterations) {
      pos->namedColor.gray = (unsigned char)((double)iterations*255.0/size);
   }
   reportIfNotEqual("view.size() == iterations",iterations,view.size());

   writePGMFile("UnitTestGrayscale1.pgm",image);
}




void createColorImage() {

   typedef ColorPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;

   ImageT image(1000u,1200u);
   reportIfNotEqual("rows",image.rows(),1000u);
   reportIfNotEqual("cols",image.cols(),1200u);
}

void copyConstructColorImages() {
   typedef ColorPixel<unsigned char> PixelT;
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
   typedef ColorPixel<unsigned char> PixelT;
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
   typedef ColorPixel<unsigned char> PixelT;
   typedef Image<PixelT> ImageT;
   typedef typename ImageT::image_view ViewT;

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
   }
   catch(const std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
