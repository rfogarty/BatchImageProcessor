#pragma once

#include "Image.h"
#include "ImageAlgorithmOpenCV.h"
#include "Pixel.h"
#include "utility/Error.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <complex>

namespace batchIP {

namespace io {

template<typename ImageT>
cv::Mat native2OCV(const ImageT& src,
                   unsigned channel = 0,
                   // This ugly bit is an unnamed argument with a default which means it neither
                   // contributes to the mangled declaration name nor requires an argument. So what is the
                   // point? It still participates in SFINAE to help select that this is an appropriate
                   // matching function given its arguments. Note, SFINAE techniques are incompatible with
                   // deduction so can't be applied to in parameter directly.
                   typename std::enable_if<types::is_grayscale<typename ImageT::pixel_type>::value ||
                                           types::is_monochrome<typename ImageT::pixel_type>::value ||
                                           types::is_hsi<typename ImageT::pixel_type>::value,int>::type* = 0,
                   typename std::enable_if<std::is_floating_point<typename ImageT::pixel_type::value_type>::value,int>::type* = 0) {

   cv::Mat ocv(src.rows(),src.cols(),CV_64FC1,cv::Scalar(0));

   for(unsigned i = 0; i < src.rows();++i) {
      for(unsigned j = 0; j < src.cols();++j) {
         ocv.at<double>(i,j) = src.pixel(i,j).indexedColor[channel];
      }
   }

   return ocv;
}

template<typename ImageT>
cv::Mat native2OCV(const ImageT& src,
      // This ugly bit is an unnamed argument with a default which means it neither
      // contributes to the mangled declaration name nor requires an argument. So what is the
      // point? It still participates in SFINAE to help select that this is an appropriate
      // matching function given its arguments. Note, SFINAE techniques are incompatible with
      // deduction so can't be applied to in parameter directly.
                   typename std::enable_if<types::is_grayscale<typename ImageT::pixel_type>::value ||
                                           types::is_monochrome<typename ImageT::pixel_type>::value,int>::type* = 0,
                   typename std::enable_if<std::is_integral<typename ImageT::pixel_type::value_type>::value,int>::type* = 0) {
   cv::Mat ocv(src.rows(),src.cols(),CV_8UC1,cv::Scalar(0));

   typedef types::MonochromePixel<uint8_t> MonoPixelT;

   for(unsigned i = 0; i < src.rows();++i) {
      for(unsigned j = 0; j < src.cols();++j) {
         MonoPixelT mono(src.pixel(i,j)); // if any scale conversion needs to happen then this will happen here.
         ocv.at<uint8_t>(i,j) = mono.tuple.value0;
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

template<typename ImageT>
ImageT scaleocv2native(const cv::Mat& ocv,double min, double max,
      // This ugly bit is an unnamed argument with a default which means it neither
      // contributes to the mangled declaration name nor requires an argument. So what is the
      // point? It still participates in SFINAE to help select that this is an appropriate
      // matching function given its arguments. Note, SFINAE techniques are incompatible with
      // deduction so can't be applied to in parameter directly.
                  typename std::enable_if<types::is_grayscale<typename ImageT::pixel_type>::value ||
                                          types::is_monochrome<typename ImageT::pixel_type>::value,int>::type* = 0) {
   ImageT nativeMat(ocv.rows,ocv.cols);

   typedef types::MonochromePixel<double> MonoPixelT;

   double rescale = 1.0/(max - min);
   //std::cout << "rescale=" << rescale << std::endl;

   double minval = 1E100;
   double maxval = -1E100;
   double acc = 0.0;
   unsigned num = 0;
   for(unsigned i = 0; i < nativeMat.rows();++i) {
      for(unsigned j = 0; j < nativeMat.cols();++j) {
         // First unit normalize
         double val = ocv.at<double>(i, j);
         if (minval > val) {
            minval = val;
            //std::cout << "minval=" << val << std::endl;
         }
         if (maxval < val) {
            maxval = val;
            //std::cout << "maxval=" << val << std::endl;
         }
         acc += val;
         ++num;
         MonoPixelT mono((val - min) * rescale);
         nativeMat.pixel(i, j) = mono; // if any additional rescale will happen here.
      }
   }
   //std::cout << "minval=" << minval << " maxval=" << maxval << " avg=" << acc/num << std::endl;
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
            if(mask.at<uint8_t>(i,j) > 128) ++numSelect;
         }
      }
      // Now make a one dimensional image the length of the pixels that are thresholded
      // There is probably a better way to do this than iterating the mask data twice.
      cv::Mat oneD(1,numSelect,CV_8UC1,cv::Scalar(0));
      unsigned pos = 0;
      for (unsigned i = 0; i < src.rows(); ++i) {
         for (unsigned j = 0; j < src.cols(); ++j) {
            if (mask.at<uint8_t>(i, j) > 128) {
               oneD.at<uint8_t>(0, pos) = ocvSrc.at<uint8_t>(i, j);
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
            if (mask.at<uint8_t>(i, j) > 128) {
               ocvSrc.at<uint8_t>(i, j) = oneD.at<uint8_t>(0, pos);
               ++pos;
            }
         }
      }
   }
   if(fgOrBg == 1 || fgOrBg == 2) {
      unsigned numSelect = 0;
      for(unsigned i = 0; i < src.rows(); ++i) {
         for(unsigned j = 0; j < src.cols(); ++j) {
            if(mask.at<uint8_t>(i,j) < 128) ++numSelect;
         }
      }
      // Now make a one dimensional image the length of the pixels that are thresholded
      // There is probably a better way to do this than iterating the mask data twice.
      cv::Mat oneD(1,numSelect,CV_8UC1,cv::Scalar(0));
      unsigned pos = 0;
      for (unsigned i = 0; i < src.rows(); ++i) {
         for (unsigned j = 0; j < src.cols(); ++j) {
            if (mask.at<uint8_t>(i, j) < 128) {
               oneD.at<uint8_t>(0, pos) = ocvSrc.at<uint8_t>(i, j);
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
            if (mask.at<uint8_t>(i, j) < 128) {
               ocvSrc.at<uint8_t>(i, j) = oneD.at<uint8_t>(0, pos);
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

//
// This centers an DFT output exactly like its Matlab equivalent.
// this code was based on example of displaying power spectral density at opencv.org
void fftshift(cv::Mat& src)
{
   int cx = src.cols/2;
   int cy = src.rows/2;

   cv::Mat q0(src, cv::Rect(0, 0, cx, cy));
   cv::Mat q1(src, cv::Rect(cx, 0, cx, cy));
   cv::Mat q2(src, cv::Rect(0, cy, cx, cy));
   cv::Mat q3(src, cv::Rect(cx, cy, cx, cy));

   cv::Mat tmp;
   q0.copyTo(tmp);
   q3.copyTo(q0);
   tmp.copyTo(q3);

   q1.copyTo(tmp);
   q2.copyTo(q1);
   tmp.copyTo(q2);
}


double unitSigmoid(double unit) {
   static const double scale = 6.0;
   return 1.0/(1 + std::exp(scale - unit*scale*2.0));
}


// Computes the log magnitude of the power spectrum or log power of Fourier Transform
template<typename SrcImageT,typename TgtImageT>
void powerSpectrum(const SrcImageT& src, TgtImageT& tgt,
      // This ugly bit is an unnamed argument with a default which means it neither
      // contributes to the mangled declaration name nor requires an argument. So what is the
      // point? It still participates in SFINAE to help select that this is an appropriate
      // matching function given its arguments. Note, SFINAE techniques are incompatible with
      // deduction so can't be applied to in parameter directly.
            typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value ||
                                    types::is_monochrome<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::Image<types::MonochromePixel<double> > NativeImageT;

   unsigned paddedSizeR = (unsigned)cv::getOptimalDFTSize(src.rows());
   unsigned paddedSizeC = (unsigned)cv::getOptimalDFTSize(src.cols());

   unsigned paddedSizeROffset = (paddedSizeR-src.rows()) >> 1;
   unsigned paddedSizeCOffset = (paddedSizeC-src.cols()) >> 1;

   NativeImageT paddedSrc(paddedSizeR,paddedSizeC);
   paddedSrc.view(src.rows(),src.cols(),paddedSizeROffset,paddedSizeCOffset) = src;

   cv::Mat ocvSrc(io::native2OCV(paddedSrc));

   cv::Mat ocvDst(cv::getOptimalDFTSize(src.rows()),cv::getOptimalDFTSize(src.cols()),CV_64FC2);
   cv::dft(ocvSrc,ocvDst,cv::DFT_COMPLEX_OUTPUT,src.rows());

   fftshift(ocvDst);
   // 1) find min and max
   double min = 1E100;
   double max = -1E100;
   for(int i = 0; i < ocvDst.rows;++i) {
      for(int j = 0; j < ocvDst.cols;++j) {
         std::complex<double>& c = ocvDst.at<std::complex<double> >(i,j);
         // Note: no need to take the square root as for the most part that factor will
         // be mostly negated anyway when we normalize the log.
         ocvDst.at<double>(i,j) = std::log(1.0 + c.real()*c.real() + c.imag()*c.imag());
         double v = ocvDst.at<double>(i,j);
         if(min > v) min = v;
         if(max < v) max = v;
      }
   }

   // We will need to unit normalize the data before returning.
   tgt = io::scaleocv2native<NativeImageT>(ocvDst,min,max).view(src.rows(),src.cols(),paddedSizeROffset,paddedSizeCOffset);
}

template<typename SrcImageT,typename TgtImageT>
void powerSpectrum(const SrcImageT& src, TgtImageT& tgt,
                   // This ugly bit is an unnamed argument with a default which means it neither
                   // contributes to the mangled declaration name nor requires an argument. So what is the
                   // point? It still participates in SFINAE to help select that this is an appropriate
                   // matching function given its arguments. Note, SFINAE techniques are incompatible with
                   // deduction so can't be applied to in parameter directly.
                   typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::HSIPixel<double> HSIPixelT;
   typedef types::Image<HSIPixelT> HSIImage;
   typedef types::GrayAlphaPixel<double> GrayscalePixelT;
   typedef types::Image<GrayscalePixelT> GrayscaleImage;

   HSIImage hsiImage(src);
   GrayscaleImage tgtMono(src.rows(),src.cols());
   selectChannel(hsiImage,tgtMono,(unsigned)HSIPixelT::INTENSITY_CHANNEL);

   powerSpectrum(tgtMono,tgtMono);

   tgt = tgtMono.defaultView();
}

void applyFilter(cv::Mat& ocvDst,unsigned rows,unsigned cols,double lowCutoff,double highCutoff) {

   // Set up filter
   // First compute the diagonal
   unsigned ry = ((unsigned) ocvDst.rows >> 1);
   unsigned rx = ((unsigned) ocvDst.cols >> 1);
   double diag = std::sqrt(double(ry)*double(ry) + double(rx)*double(rx));

   // Maintain the following as the square to avoid sqrt function in loop below.
   double lowcutPixelRangeSq =  diag*lowCutoff*diag*lowCutoff;
   double highcutPixelRangeSq = diag*highCutoff*diag*highCutoff;

   if(lowCutoff < highCutoff) { // if these are the same value (like 0.0), we can ignore
      // Bandpass filter

      // Now let's zero out values that are outside of the bandpass area
      double middleRow = (unsigned) ocvDst.rows >> 1;
      double middleCol = (unsigned) ocvDst.cols >> 1;
      for (unsigned i = 0; i < rows; ++i) {
         double rowoff = (double) i - middleRow;
         rowoff *= rowoff;
         for (unsigned j = 0; j < cols; ++j) {
            double coloff = (double) j - middleCol;
            coloff *= coloff;

            double range = coloff + rowoff;
            if (lowcutPixelRangeSq > range || range > highcutPixelRangeSq) {
               std::complex<double> &c = ocvDst.at<std::complex<double>>(i, j);
               c.real(0.0);
               c.imag(0.0);
            }
         }
      }
   }
   else if(lowCutoff > highCutoff) { // if these are the same value (like 0.0), we can ignore
      // Bandstop filter

      // Now let's zero out values that are inside of the bandstop area
      double middleRow = (unsigned) ocvDst.rows >> 1;
      double middleCol = (unsigned) ocvDst.cols >> 1;
      for (unsigned i = 0; i < rows; ++i) {
         double rowoff = (double) i - middleRow;
         rowoff *= rowoff;
         for (unsigned j = 0; j < cols; ++j) {
            double coloff = (double) j - middleCol;
            coloff *= coloff;

            double range = coloff + rowoff;
            if (highcutPixelRangeSq < range && range < lowcutPixelRangeSq) {
               std::complex<double> &c = ocvDst.at<std::complex<double>>(i, j);
               c.real(0.0);
               c.imag(0.0);
            }
         }
      }
   }
   // else if equal do nothing
}

#if 0
void applyTaperedFilter(cv::Mat& ocvDst,unsigned rows,unsigned cols,double lowCutoff,double highCutoff) {
// Set up filter, this should be its own routine.
   // Now let's zero out values at some unit-level of "pi" distance, where pi is the distance from the center to the
   // edge of the image in both rows and cols.
//   double range1 = 0.05; // some fraction of 1.0 (which would be a fraction of pi)
   unsigned lowcutPixelRangeR = (unsigned)(lowCutoff * ((unsigned)ocvDst.rows >> 1));
   unsigned lowcutPixelRangeC = (unsigned)(lowCutoff * ((unsigned)ocvDst.cols >> 1));
   // If rows and cols are not equal, instead of doing an elliptical boundary, simply shrinken to the smaller value
   unsigned lowcutPixelRange = std::min(lowcutPixelRangeR,lowcutPixelRangeC);
   // Maintain as the square to avoid sqrt function below.
   double lowcutPixelRangeSq = lowcutPixelRange*lowcutPixelRange;

   //std::cout << "pixelRange=" << pixelRange << " pixelRangeSq=" << pixelRangeSq << std::endl;

   unsigned highcutPixelRangeR = (unsigned)(highCutoff * ((unsigned)ocvDst.rows >> 1));
   unsigned highcutPixelRangeC = (unsigned)(highCutoff * ((unsigned)ocvDst.cols >> 1));
   // If rows and cols are not equal, instead of doing an elliptical boundary, simply shrinken to the smaller value
   unsigned highcutPixelRange = std::min(highcutPixelRangeR,highcutPixelRangeC);
   // Maintain as the square to avoid sqrt function below.
   double highcutPixelRangeSq = highcutPixelRange*highcutPixelRange;


//   double range2 = 0.15; // some fraction of 1.0 (which would be a fraction of pi)
//   pixelRange1 = (unsigned)(range2 * ((unsigned)ocvDst.rows >> 1));
//   pixelRange2 = (unsigned)(range2 * ((unsigned)ocvDst.cols >> 1));
//   pixelRange = std::min(pixelRange1,pixelRange2);
//   double pixelRangeSq2 = pixelRange2*pixelRange2;
//   std::cout << "pixelRange=" << pixelRange << " pixelRangeSq=" << pixelRangeSq << std::endl;

   // Now we need to iterate the set of pixels and find the distance from the middle. If it is within the window
   // we do nothing, if it is outside the window we zero out the real part.

   double middleRow = (unsigned)ocvDst.rows >> 1;
   double middleCol = (unsigned)ocvDst.cols >> 1;

//   double taperRange = 1.0/(pixelRangeSq2-pixelRangeSq);

   for(unsigned i = 0; i < rows;++i) {
      double rowoff = (double)i-middleRow;
      rowoff *= rowoff;
      for(unsigned j = 0; j < cols;++j) {
         double coloff = (double)j-middleCol;
         coloff *= coloff;

         double range = coloff+rowoff;
         if(range > pixelRangeSq2) {
            std::complex<double>& c = ocvDst.at<std::complex<double>>(i,j);
            c.real(0.0);
            c.imag(0.0);
         }
         else if(range > pixelRangeSq) {
            // outside of range so, zero out fft
            ocvDst.at<std::complex<double>>(i,j) *= unitSigmoid((pixelRangeSq2 - range)*taperRange);
         }
      }
   }
}

#endif

// Our input parameter space sets up two bandpass/bandstop areas
// However, applying a "double" bandpass filter will need to rearrange parameters into a bandpass and bandstop...
void remapFilterParams(double filter1Low,double& filter1High,double filter2Low,double& filter2High) {

   // This slightly sneaky remapping of the input space, allows us to easily compute
   // the composed filter by passing through applyFilter 2x. But in order to allow for
   // a "double" band pass, we modify the parameters to operate as a single wider bandpass
   // with a bandstop filter in the middle.

   // Parameter Options:
   // Lowpass: 0 H1 0 0   -> Bandpass(0,H1),(0,0)  ... no change
   // Lowpass: 0 0 0 H2   -> (0,0),Bandpass(0,H2)  ... no change
   // Highpass: L1 1 0 0  -> Bandpass(L1,1),(0,0)  ... no change
   // Bandpass: L1 H1 0 0 -> Bandpass(L1,H1),(0,0) ... no change
   // Bandstop: H1 L1 0 0 -> Bandstop(H1,L1),(0,0) ... no change
   // Highpass: 0 0 L2 1  -> (0,0),Bandpass(L2,1)  ... no change
   // Bandpass: 0 0 L2 H2 -> (0,0),Bandpass(L2,H2) ... no change
   // Bandstop: 0 0 H2 L2 -> (0,0),Bandstop(H2,L2) ... no change
   // Double Bandpass L1 H1 L2 H2 -> Bandpass(L1,H2),Bandstop(L2,H1) (L1->L1, H1->H2, L2->L2, H2->H1)
   // Double Bandstop H1 L1 H2 L2 -> Bandstop(H1,L1),Bandstop(H2,L2) ... no change

   if(filter1Low < filter1High && filter1High < filter2Low) std::swap(filter1High,filter2High);
   else if(filter2Low < filter2High && filter2High < filter1Low) std::swap(filter1High,filter2High);
   // else do nothing
}


// Computes the log magnitude of the power spectrum or log magnitude of Fourier Transform
template<typename SrcImageT,typename TgtImageT>
void filterResponse(const SrcImageT& src, TgtImageT& tgt,double low1,double high1, double low2,double high2,
                    // This ugly bit is an unnamed argument with a default which means it neither
                    // contributes to the mangled declaration name nor requires an argument. So what is the
                    // point? It still participates in SFINAE to help select that this is an appropriate
                    // matching function given its arguments. Note, SFINAE techniques are incompatible with
                    // deduction so can't be applied to in parameter directly.
                    typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value ||
                                            types::is_monochrome<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::Image<types::MonochromePixel<double> > NativeImageT;

   unsigned paddedSizeR = (unsigned)cv::getOptimalDFTSize(src.rows());
   unsigned paddedSizeC = (unsigned)cv::getOptimalDFTSize(src.cols());

   unsigned paddedSizeROffset = (paddedSizeR-src.rows()) >> 1;
   unsigned paddedSizeCOffset = (paddedSizeC-src.cols()) >> 1;

   NativeImageT paddedSrc(paddedSizeR,paddedSizeC);
   paddedSrc.view(src.rows(),src.cols(),paddedSizeROffset,paddedSizeCOffset) = src;

   cv::Mat ocvSrc(io::native2OCV(paddedSrc));

   cv::Mat ocvDst(cv::getOptimalDFTSize(src.rows()),cv::getOptimalDFTSize(src.cols()),CV_64FC2);
   cv::dft(ocvSrc,ocvDst,cv::DFT_COMPLEX_OUTPUT,src.rows());

   fftshift(ocvDst);

//   std::cout << "Filter Params: " << low1 << "->" << high1 << ", " << low2 << "->" << high2 << std::endl;
   remapFilterParams(low1,high1,low2,high2);
//   std::cout << "Filter Params: " << low1 << "->" << high1 << ", " << low2 << "->" << high2 << std::endl;

   applyFilter(ocvDst,paddedSrc.rows(),paddedSrc.cols(),low1,high1); // ...
   applyFilter(ocvDst,paddedSrc.rows(),paddedSrc.cols(),low2,high2); // ...

   // Rewrite real element as log magnitude - no need to square root magnitude since we are normalizing below
   double min = 1E100;
   double max = -1E100;
   for(int i = 0; i < ocvDst.rows;++i) {
      for(int j = 0; j < ocvDst.cols;++j) {
         std::complex<double>& c = ocvDst.at<std::complex<double> >(i,j);
         ocvDst.at<double>(i,j) = std::log(1.0 + c.real()*c.real() + c.imag()*c.imag());
         double v = ocvDst.at<double>(i,j);
         if(min > v) min = v;
         if(max < v) max = v;
      }
   }

   tgt = io::scaleocv2native<NativeImageT>(ocvDst,min,max).view(src.rows(),src.cols(),paddedSizeROffset,paddedSizeCOffset);
}

template<typename SrcImageT,typename TgtImageT>
void filterResponse(const SrcImageT& src, TgtImageT& tgt,double low1,double high1, double low2,double high2,
                    // This ugly bit is an unnamed argument with a default which means it neither
                    // contributes to the mangled declaration name nor requires an argument. So what is the
                    // point? It still participates in SFINAE to help select that this is an appropriate
                    // matching function given its arguments. Note, SFINAE techniques are incompatible with
                    // deduction so can't be applied to in parameter directly.
                    typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::HSIPixel<double> HSIPixelT;
   typedef types::Image<HSIPixelT> HSIImage;
   typedef types::GrayAlphaPixel<double> GrayscalePixelT;
   typedef types::Image<GrayscalePixelT> GrayscaleImage;

   HSIImage hsiImage(src);
   GrayscaleImage tgtMono(src.rows(),src.cols());
   selectChannel(hsiImage,tgtMono,(unsigned)HSIPixelT::INTENSITY_CHANNEL);

   filterResponse(tgtMono,tgtMono,low1,high1,low2,high2);

   tgt = tgtMono.defaultView();
}

template<typename SrcImageT,typename TgtImageT>
void lpResponse(const SrcImageT& src, TgtImageT& tgt,double cutoff) {
   filterResponse(src,tgt,0.0,cutoff,0.0,0.0);
}

template<typename SrcImageT,typename TgtImageT>
void hpResponse(const SrcImageT& src, TgtImageT& tgt,double cutoff) {
   filterResponse(src,tgt,cutoff,1.0,0.0,0.0);
}

template<typename SrcImageT,typename TgtImageT>
void bpResponse(const SrcImageT& src, TgtImageT& tgt,double low,double high) {
   filterResponse(src,tgt,low,high,0.0,0.0);
}

template<typename SrcImageT,typename TgtImageT>
void filter(const SrcImageT& src, TgtImageT& tgt,double low1,double high1, double low2,double high2,
            // This ugly bit is an unnamed argument with a default which means it neither
            // contributes to the mangled declaration name nor requires an argument. So what is the
            // point? It still participates in SFINAE to help select that this is an appropriate
            // matching function given its arguments. Note, SFINAE techniques are incompatible with
            // deduction so can't be applied to in parameter directly.
            typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value ||
                                    types::is_monochrome<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::Image <types::MonochromePixel<double>> NativeImageT;

   unsigned paddedSizeR = (unsigned)cv::getOptimalDFTSize(src.rows());
   unsigned paddedSizeC = (unsigned)cv::getOptimalDFTSize(src.cols());

   unsigned paddedSizeROffset = (paddedSizeR-src.rows()) >> 1;
   unsigned paddedSizeCOffset = (paddedSizeC-src.cols()) >> 1;

   NativeImageT paddedSrc(paddedSizeR,paddedSizeC);
   paddedSrc.view(src.rows(),src.cols(),paddedSizeROffset,paddedSizeCOffset) = src;

   cv::Mat ocvSrc(io::native2OCV(paddedSrc));

   cv::Mat ocvDst(cv::getOptimalDFTSize(src.rows()), cv::getOptimalDFTSize(src.cols()), CV_64FC2);
   cv::dft(ocvSrc, ocvDst, cv::DFT_COMPLEX_OUTPUT, src.rows());

   fftshift(ocvDst);

   remapFilterParams(low1, high1, low2, high2);
   applyFilter(ocvDst, paddedSrc.rows(), paddedSrc.cols(), low1, high1); // ...
   applyFilter(ocvDst, paddedSrc.rows(), paddedSrc.cols(), low2, high2); // ...

   // Now reshift fft
   fftshift(ocvDst);

   // Now we have to take the inverse DFT
   cv::Mat ocvDst2(cv::getOptimalDFTSize(src.rows()), cv::getOptimalDFTSize(src.cols()), CV_64FC1);
   cv::dft(ocvDst, ocvDst2, cv::DFT_REAL_OUTPUT + cv::DFT_INVERSE, src.rows());

   double min = 1E100;
   double max = -1E100;
   for (unsigned i = 0; i < src.rows(); ++i) {
      for (unsigned j = 0; j < src.cols(); ++j) {
         double v = ocvDst2.at<double>(i, j);
         if (min > v) min = v;
         if (max < v) max = v;
      }
   }

   tgt = io::scaleocv2native<NativeImageT>(ocvDst2, min, max).view(src.rows(), src.cols(),paddedSizeROffset,paddedSizeCOffset);
}

template<typename SrcImageT,typename TgtImageT>
void filter(const SrcImageT& src, TgtImageT& tgt,double low1,double high1, double low2,double high2,
            // This ugly bit is an unnamed argument with a default which means it neither
            // contributes to the mangled declaration name nor requires an argument. So what is the
            // point? It still participates in SFINAE to help select that this is an appropriate
            // matching function given its arguments. Note, SFINAE techniques are incompatible with
            // deduction so can't be applied to in parameter directly.
            typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::HSIPixel<double> HSIPixelT;
   typedef types::Image<HSIPixelT> HSIImage;
   typedef types::GrayAlphaPixel<double> GrayscalePixelT;
   typedef types::Image<GrayscalePixelT> GrayscaleImage;

   HSIImage hsiImage(src);
   GrayscaleImage tgtMono(src.rows(),src.cols());
   selectChannel(hsiImage,tgtMono,(unsigned)HSIPixelT::INTENSITY_CHANNEL);

   filter(tgtMono,tgtMono,low1,high1,low2,high2);

   // TODO: Surprisingly I don't yet have a way to assign an image to a channel,
   // for now I'll just iterate the image and do it manually
   auto hsipos = hsiImage.begin();
   auto hsiend = hsiImage.end();
   auto processedPos = tgtMono.begin();
   for(;hsipos != hsiend;++hsipos,++processedPos) {
      hsipos->indexedColor[HSIPixelT::INTENSITY_CHANNEL] = unitClamp(processedPos->tuple.value0);
   }

   // TODO: there still appears to be some sort of color conversion artifact (hue is swinging incorrectly)
   // need to track this down!!
   tgt = hsiImage.defaultView();
}


template<typename SrcImageT,typename TgtImageT>
void lpFilter(const SrcImageT& src, TgtImageT& tgt,double cutoff) {
   filter(src,tgt,0.0,cutoff,0.0,0.0);
}

template<typename SrcImageT,typename TgtImageT>
void hpFilter(const SrcImageT& src, TgtImageT& tgt,double cutoff) {
   filter(src,tgt,cutoff,1.0,0.0,0.0);
}

template<typename SrcImageT,typename TgtImageT>
void bpFilter(const SrcImageT& src, TgtImageT& tgt,double low,double high) {
   filter(src,tgt,low,high,0.0,0.0);
}

} // namespace algorithm
} // namespace batchIP
