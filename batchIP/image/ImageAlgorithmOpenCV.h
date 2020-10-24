#pragma once

#include "Image.h"
#include "ImageAlgorithmOpenCV.h"
#include "Pixel.h"
#include "utility/Error.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>

#define PRESMOOTH 1

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
   std::cout << "Computing Sobel gradient in OpenCV using windowSize=" << windowSize << std::endl;
#ifdef HIGHRES_SOBEL
   Sobel(ocvSrc, gradientX, CV_32FC1, 1, 0, windowSize, 1, 0, cv::BORDER_DEFAULT);
   Sobel(ocvSrc, gradientY, CV_32FC1, 0, 1, windowSize, 1, 0, cv::BORDER_DEFAULT);
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

} // namespace algorithm
} // namespace batchIP
