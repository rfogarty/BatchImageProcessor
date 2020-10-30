#pragma once

#include "Image.h"
#include "ImageAlgorithmOpenCV.h"
#include "Pixel.h"
#include "utility/Error.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>

namespace batchIP {

namespace io {

template<typename ImageT>
cv::Mat native2OCV(const ImageT& src,
                   // This ugly bit is an unnamed argument with a default which means it neither
                   // contributes to the mangled declaration name nor requires an argument. So what is the
                   // point? It still participates in SFINAE to help select that this is an appropriate
                   // matching function given its arguments. Note, SFINAE techniques are incompatible with
                   // deduction so can't be applied to in parameter directly.
                   typename std::enable_if<types::is_grayscale<typename ImageT::pixel_type>::value ||
                                           types::is_monochrome<typename ImageT::pixel_type>::value,int>::type* = 0) {
   cv::Mat ocv(src.rows(),src.cols(),CV_8UC1,cv::Scalar(0));

   typedef types::MonochromePixel<uint8_t> MonoPixelT;

   for(unsigned i = 0; i < src.rows();++i) {
      for(unsigned j = 0; j < src.cols();++j) {
         MonoPixelT mono(src.pixel(i,j)); // if any scale conversion needs to happen then this will happen here.
         ocv.at<uint8_t>(i,j,0) = mono.tuple.value0;
      }
   }

   return ocv;
}

template<typename ImageT>
ImageT ocv2native(const cv::Mat& ocv,
                  // This ugly bit is an unnamed argument with a default which means it neither
                  // contributes to the mangled declaration name nor requires an argument. So what is the
                  // point? It still participates in SFINAE to help select that this is an appropriate
                  // matching function given its arguments. Note, SFINAE techniques are incompatible with
                  // deduction so can't be applied to in parameter directly.
                  typename std::enable_if<types::is_grayscale<typename ImageT::pixel_type>::value ||
                                          types::is_monochrome<typename ImageT::pixel_type>::value,int>::type* = 0) {
   ImageT nativeMat(ocv.rows,ocv.cols);

   typedef types::MonochromePixel<uint8_t> MonoPixelT;

   for(unsigned i = 0; i < nativeMat.rows();++i) {
      for(unsigned j = 0; j < nativeMat.cols();++j) {
         MonoPixelT mono(ocv.at<uint8_t>(i,j,0));
         nativeMat.pixel(i,j) = mono; // if any scale conversion needs to happen then this will happen here.
      }
   }

   return nativeMat;
}


} // namespace io

namespace algorithm {

//#define PRESMOOTH 1
#define HIGHRES_SOBEL 1

template<typename SrcImageT,typename TgtImageT>
void edgeSobelOCV(const SrcImageT src,TgtImageT& tgt,unsigned windowSize) {

   typedef types::Image<types::MonochromePixel<uint8_t> > NativeImageT;
   cv::Mat ocvSrc(io::native2OCV(src));
#ifdef PRESMOOTH
   cv::Mat smoothOcvSrc;
   GaussianBlur(ocvSrc, smoothOcvSrc, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT);
   ocvSrc = smoothOcvSrc;
#endif
   cv::Mat gradientX,gradientY,gradientXScaled,gradientYScaled,gradient;
   //std::cout << "Computing Sobel gradient in OpenCV using windowSize=" << windowSize << std::endl;
#ifdef HIGHRES_SOBEL
   Sobel(ocvSrc, gradientX, CV_64FC1, 1, 0, windowSize, 1, 0, cv::BORDER_DEFAULT);
   Sobel(ocvSrc, gradientY, CV_64FC1, 0, 1, windowSize, 1, 0, cv::BORDER_DEFAULT);
#else
   Sobel(ocvSrc, gradientX, CV_16S, 1, 0, windowSize, 1, 0, cv::BORDER_DEFAULT);
   Sobel(ocvSrc, gradientY, CV_16S, 0, 1, windowSize, 1, 0, cv::BORDER_DEFAULT);
#endif
   convertScaleAbs(gradientX, gradientXScaled);
   convertScaleAbs(gradientY, gradientYScaled);

#ifdef NATIVEMAGNITUDE
   GradientT natGradientX(io::ocv2native<GradientT>(gradientXScaled));
   GradientT natGradientY(io::ocv2native<GradientT>(gradientYScaled));
   tgt = gradientMagnitude(natGradientX,natGradientY);
#else
   addWeighted(gradientXScaled, 0.5, gradientYScaled, 0.5, 0, gradient);
   tgt = io::ocv2native<NativeImageT>(gradient);
#endif
}

template<typename SrcImageT,typename TgtImageT>
void edgeCannyOCV(const SrcImageT src,TgtImageT& tgt,unsigned windowSize,double tlow,double thigh) {

   typedef types::Image<types::MonochromePixel<uint8_t> > NativeImageT;
   cv::Mat ocvSrc(io::native2OCV(src));
   cv::Mat edge;
   cv::Canny(ocvSrc,edge,tlow,thigh,windowSize,true);
   tgt = io::ocv2native<NativeImageT>(edge);
}

template<typename SrcImageT,typename TgtImageT>
void histogramEqualizeOCV(const SrcImageT src,TgtImageT& tgt) {

   typedef types::Image<types::MonochromePixel<uint8_t> > NativeImageT;
   cv::Mat ocvSrc(io::native2OCV(src));
   cv::Mat ocvDst;
   cv::equalizeHist(ocvSrc, ocvDst);
   tgt = io::ocv2native<NativeImageT>(ocvDst);
}

template<typename SrcImageT,typename TgtImageT>
void otsuBinarizeOCV(const SrcImageT& src, TgtImageT& tgt,
      // This ugly bit is an unnamed argument with a default which means it neither
      // contributes to the mangled declaration name nor requires an argument. So what is the
      // point? It still participates in SFINAE to help select that this is an appropriate
      // matching function given its arguments. Note, SFINAE techniques are incompatible with
      // deduction so can't be applied to in parameter directly.
                  typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value ||
                                          types::is_monochrome<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::Image<types::MonochromePixel<uint8_t> > NativeImageT;
   cv::Mat ocvSrc(io::native2OCV(src));
   cv::Mat ocvDst;
   cv::threshold(ocvSrc, ocvDst, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
   tgt = io::ocv2native<NativeImageT>(ocvDst);
}

template<typename SrcImageT,typename TgtImageT>
void thresholdEqualizeOCV(const SrcImageT& src, TgtImageT& tgt,unsigned fgOrBg,
      // This ugly bit is an unnamed argument with a default which means it neither
      // contributes to the mangled declaration name nor requires an argument. So what is the
      // point? It still participates in SFINAE to help select that this is an appropriate
      // matching function given its arguments. Note, SFINAE techniques are incompatible with
      // deduction so can't be applied to in parameter directly.
                     typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value ||
                                             types::is_monochrome<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::Image<types::MonochromePixel<uint8_t> > NativeImageT;
   cv::Mat ocvSrc(io::native2OCV(src));
   // First compute the mask to determine foreground and background regions
   cv::Mat mask;
   cv::threshold(ocvSrc, mask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

   if(fgOrBg == 0 || fgOrBg == 2) {
      unsigned numSelect = 0;
      for(unsigned i = 0; i < src.rows(); ++i) {
         for(unsigned j = 0; j < src.cols(); ++j) {
            if(mask.at<uint8_t>(i,j,0) > 128) ++numSelect;
         }
      }
      // Now make a one dimensional image the length of the pixels that are thresholded
      // There is probably a better way to do this than iterating the mask data twice.
      cv::Mat oneD(1,numSelect,CV_8UC1,cv::Scalar(0));
      unsigned pos = 0;
      for (unsigned i = 0; i < src.rows(); ++i) {
         for (unsigned j = 0; j < src.cols(); ++j) {
            if (mask.at<uint8_t>(i, j, 0) > 128) {
               oneD.at<uint8_t>(0, pos, 0) = ocvSrc.at<uint8_t>(i, j, 0);
               ++pos;
            }
         }
      }
      // Now we can histogramEqualize the 1D image.
      cv::equalizeHist(oneD,oneD);
      // And write equalized values back into original matrix
      pos = 0;
      for (unsigned i = 0; i < src.rows(); ++i) {
         for (unsigned j = 0; j < src.cols(); ++j) {
            if (mask.at<uint8_t>(i, j, 0) > 128) {
               ocvSrc.at<uint8_t>(i, j, 0) = oneD.at<uint8_t>(0, pos, 0);
               ++pos;
            }
         }
      }
   }
   if(fgOrBg == 1 || fgOrBg == 2) {
      unsigned numSelect = 0;
      for(unsigned i = 0; i < src.rows(); ++i) {
         for(unsigned j = 0; j < src.cols(); ++j) {
            if(mask.at<uint8_t>(i,j,0) < 128) ++numSelect;
         }
      }
      // Now make a one dimensional image the length of the pixels that are thresholded
      // There is probably a better way to do this than iterating the mask data twice.
      cv::Mat oneD(1,numSelect,CV_8UC1,cv::Scalar(0));
      unsigned pos = 0;
      for (unsigned i = 0; i < src.rows(); ++i) {
         for (unsigned j = 0; j < src.cols(); ++j) {
            if (mask.at<uint8_t>(i, j, 0) < 128) {
               oneD.at<uint8_t>(0, pos, 0) = ocvSrc.at<uint8_t>(i, j, 0);
               ++pos;
            }
         }
      }
      // Now we can histogramEqualize the 1D image.
      cv::equalizeHist(oneD,oneD);
      // And write equalized values back into original matrix
      pos = 0;
      for (unsigned i = 0; i < src.rows(); ++i) {
         for (unsigned j = 0; j < src.cols(); ++j) {
            if (mask.at<uint8_t>(i, j, 0) < 128) {
               ocvSrc.at<uint8_t>(i, j, 0) = oneD.at<uint8_t>(0, pos, 0);
               ++pos;
            }
         }
      }
   }

   tgt = io::ocv2native<NativeImageT>(ocvSrc);
}

#ifdef SUPPORT_QRCODE_DETECT
template<typename SrcImageT,typename TgtImageT>
void qrDecodeOCV(const SrcImageT& src,TgtImageT& tgt,bool equalize,
      // This ugly bit is an unnamed argument with a default which means it neither
      // contributes to the mangled declaration name nor requires an argument. So what is the
      // point? It still participates in SFINAE to help select that this is an appropriate
      // matching function given its arguments. Note, SFINAE techniques are incompatible with
      // deduction so can't be applied to in parameter directly.
                     typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value ||
                                             types::is_monochrome<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::Image<types::MonochromePixel<uint8_t> > NativeImageT;

   // First equalize image to present image with best contrast
   NativeImageT srcEQ(src.rows(),src.cols());
   if(equalize) histogramEqualizeOCV(src,srcEQ);
   else srcEQ = src;

   // Now use OpenCV QRDetector
   cv::Mat ocvSrc(io::native2OCV(srcEQ));
   cv::Mat points, binarizedImage;
   cv::QRCodeDetector qrcodeReader;
   std::string code = qrcodeReader.detectAndDecode(ocvSrc, points, binarizedImage);
   if(code.size() > 0u) {
      std::cout << "QRCodeDetector: ";
      if(code.size() > 256u) std::cout << code.substr(0,255) << "..." << std::endl;
      else std::cout << code << std::endl;
      //tgt.view(binarizedImage.rows,binarizedImage.cols) = io::ocv2native<NativeImageT>(binarizedImage);
   }
   else {
      //utility::fail("QRCodeDetector: failed to decode QRCode");
      std::cout << "QRCodeDetector: failed to decode QRCode" << std::endl;
      //std::cout << "Sizeof binarizeImage: " << binarizedImage.rows << "," << binarizedImage.cols << std::endl;
   }
   tgt = srcEQ;
}
#endif

} // namespace algorithm
} // namespace batchIP
