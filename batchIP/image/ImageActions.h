#pragma once

#include "ImageAction.h"
#include "RegionOfInterest.h"
#include "ImageAlgorithm.h"
#include "ImageAlgorithmOpenCV.h"
#include "utility/StringParse.h"
#include "utility/Error.h"

namespace batchIP {
namespace operation {

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// Abstract Actions that call above funtions follow

template<typename ImageT>
class Scale : public Action<ImageT> {
public:
   typedef Scale<ImageT> ThisT;

private:
   float mAmount;

   enum { NUM_PARAMETERS = 1 };

public:
   explicit Scale(int amount) : mAmount(amount) {}

   virtual ~Scale() {}

   virtual ActionType type() const { return SCALE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      algorithm::scale(src,tgt,mAmount);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::fail("scale function does not support regions");
   }

   static Scale* make(std::istream& ins) {
      int amount = utility::parseWord<float>(ins);
      return new Scale<ImageT>(amount);
   }
};


template<typename ImageT>
class Crop : public Action<ImageT> {
public:
   typedef Crop<ImageT> ThisT;

private:
   types::RegionOfInterest mROI;

   enum { NUM_PARAMETERS = 4 };

public:
   explicit Crop(const types::RegionOfInterest& roi) : mROI(roi) {}

   virtual ~Crop() {}

   virtual ActionType type() const { return CROP; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      tgt = roi2view(src,mROI);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::fail("scale function does not support regions");
   }

   static Crop* make(std::istream& ins) {
      types::RegionOfInterest roi;
      roi.mRowBegin = utility::parseWord<unsigned>(ins);
      roi.mColBegin = utility::parseWord<unsigned>(ins);
      roi.mRows = utility::parseWord<unsigned>(ins);
      roi.mCols = utility::parseWord<unsigned>(ins);
      return new Crop<ImageT>(roi);
   }
};

template<typename ImageT>
class Histogram : public Action<ImageT> {
public:
   typedef Histogram<ImageT> ThisT;

private:

   unsigned mLogBase;
   bool mPrintHistogram;
   mutable bool mRunOnce;

   void runHist(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,unsigned logBase,bool printHistogram) const {
      algorithm::histogram(types::roi2view(src,roi),tgt,logBase,printHistogram);
   }

   enum { NUM_PARAMETERS = 2 };

public:
   explicit Histogram(unsigned logBase,bool printHistogram) : mLogBase(logBase), mPrintHistogram(printHistogram),mRunOnce(false) {}

   virtual ~Histogram() {}

   virtual ActionType type() const { return HISTOGRAM; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      runHist(src,tgt,view2roi(src.defaultView()),mLogBase,mPrintHistogram);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      if(!mRunOnce) {
         mRunOnce = true;
         utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
         unsigned logBase = utility::parseWord<unsigned>(parameters[0]);
         bool printHistogram = utility::parseWord<bool>(parameters[1]);
         runHist(src,tgt,roi,logBase,printHistogram);
      }
      else {
         utility::fail("Histogram (hist) supports only one ROI (call multiple times or use crop first)!");
      }
   }

   static Histogram* make(std::istream& ins) {
      unsigned logBase = utility::parseWord<unsigned>(ins);
      bool printHistogram = utility::parseWord<bool>(ins);
      return new Histogram<ImageT>(logBase,printHistogram);
   }
};




template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >
class HistogramChannel : public Action<ImageSrc,ImageTgt> {
public:
   typedef HistogramChannel<ImageSrc,ImageTgt> ThisT;

private:

   unsigned mLogBase;
   unsigned mChannel;
   bool mPrintHistogram;
   mutable bool mRunOnce;

   void runHist(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,unsigned logBase,unsigned channel,bool printHistogram) const {
      algorithm::histogram(types::roi2view(src,roi),tgt,logBase,channel,printHistogram);
   }

   enum { NUM_PARAMETERS = 3 };

public:
   HistogramChannel(unsigned logBase,unsigned channel,bool printHistogram) : mLogBase(logBase),mChannel(channel), mPrintHistogram(printHistogram), mRunOnce(false) {}

   virtual ~HistogramChannel() {}

   virtual ActionType type() const { return HISTOGRAM; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {
      runHist(src,tgt,view2roi(src.defaultView()),mLogBase,mChannel,mPrintHistogram);
   }

   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      if(!mRunOnce) {
         mRunOnce = true;
         utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
         unsigned logBase = utility::parseWord<unsigned>(parameters[0]);
         unsigned channel = utility::parseWord<unsigned>(parameters[1]);
         bool printHistogram = utility::parseWord<bool>(parameters[2]);
         runHist(src,tgt,roi,logBase,channel,printHistogram);
      }
      else {
         utility::fail("Histogram (histChan) supports only one ROI (call multiple times or use crop first)!");
      }
   }

   static HistogramChannel* make(std::istream& ins) {
      unsigned logBase = utility::parseWord<unsigned>(ins);
      unsigned channel = utility::parseWord<unsigned>(ins);
      bool printHistogram = utility::parseWord<bool>(ins);
      return new HistogramChannel<ImageSrc,ImageTgt>(logBase,channel,printHistogram);
   }
};




#define ZERO_ARG_ACTION(NAME,CALL,TYPE)                                                                                                   \
template<typename ImageT>                                                                                                                 \
class NAME : public Action<ImageT> {                                                                                                      \
public:                                                                                                                                   \
   typedef NAME<ImageT> ThisT;                                                                                                            \
                                                                                                                                          \
private:                                                                                                                                  \
   void runPr(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi) const {                                                   \
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);                                                                     \
      algorithm::CALL(types::roi2view(src,roi),tgtview);                                                                                  \
   }                                                                                                                                      \
   enum { NUM_PARAMETERS = 0 };                                                                                                           \
public:                                                                                                                                   \
   virtual ~NAME() {}                                                                                                                     \
                                                                                                                                          \
   virtual ActionType type() const { return TYPE; }                                                                                       \
                                                                                                                                          \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                      \
                                                                                                                                          \
   virtual void run(const ImageT& src,ImageT& tgt) const {                                                                                \
      runPr(src,tgt,view2roi(src.defaultView()));                                                                                         \
   }                                                                                                                                      \
                                                                                                                                          \
   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {      \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                \
      runPr(src,tgt,roi);                                                                                                                 \
   }                                                                                                                                      \
                                                                                                                                          \
   static NAME* make(std::istream& ins) {                                                                                                 \
      return new NAME<ImageT>();                                                                                                          \
   }                                                                                                                                      \
};                                                                                                                                        \
/* End of ZERO_ARG_ACTION */

ZERO_ARG_ACTION(HistogramEqualizeOCV,histogramEqualizeOCV,HISTOGRAM_EQ)
ZERO_ARG_ACTION(OptimalBinarize,optimalBinarize,BINARIZE)
ZERO_ARG_ACTION(OtsuBinarize,otsuBinarize,BINARIZE)
ZERO_ARG_ACTION(OtsuBinarizeOCV,otsuBinarizeOCV,BINARIZE)
ZERO_ARG_ACTION(DFTFilter,filter,BINARIZE)
ZERO_ARG_ACTION(PowerSpectrum,powerSpectrum,POWER_SPECTUM)



#define ZERO_ARG_GRAY_OUT_ACTION(NAME,CALL,TYPE)                                                                                                      \
template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >                      \
class NAME : public Action<ImageSrc,ImageTgt> {                                                                                                       \
public:                                                                                                                                               \
   typedef Action<ImageSrc,ImageTgt> SuperT;                                                                                                          \
   typedef NAME<ImageSrc,ImageTgt> ThisT;                                                                                                             \
   typedef typename ImageSrc::pixel_type pixel_type;                                                                                                  \
                                                                                                                                                      \
private:                                                                                                                                              \
                                                                                                                                                      \
   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi) const {                                                             \
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);                                                                               \
      algorithm::CALL(types::roi2view(src,roi),tgtview);                                                                                              \
   }                                                                                                                                                  \
                                                                                                                                                      \
   enum { NUM_PARAMETERS = 0 };                                                                                                                       \
                                                                                                                                                      \
public:                                                                                                                                               \
   explicit NAME() {}                                                                                                                                 \
                                                                                                                                                      \
   virtual ~NAME() {}                                                                                                                                 \
                                                                                                                                                      \
   virtual ActionType type() const { return TYPE; }                                                                                                   \
                                                                                                                                                      \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {                                                                                        \
      run(src,tgt,view2roi(src.defaultView()));                                                                                                       \
   }                                                                                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {              \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                            \
      run(src,tgt,roi);                                                                                                                               \
   }                                                                                                                                                  \
                                                                                                                                                      \
   static NAME* make(std::istream& ins) {                                                                                                             \
      return new NAME<ImageSrc,ImageTgt>();                                                                                                           \
   }                                                                                                                                                  \
};                                                                                                                                                    \
/* End of ZERO_ARG_GRAY_OUT_ACTION */



#define ONE_ARG_ACTION(NAME,CALL,TYPE,VAL0T)                                                                                              \
template<typename ImageT>                                                                                                                 \
class NAME : public Action<ImageT> {                                                                                                      \
public:                                                                                                                                   \
   typedef NAME<ImageT> ThisT;                                                                                                            \
                                                                                                                                          \
private:                                                                                                                                  \
   VAL0T mVal0;                                                                                                                           \
                                                                                                                                          \
   void runPr(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,VAL0T val0) const {                                        \
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);                                                                     \
      algorithm::CALL(types::roi2view(src,roi),tgtview,val0);                                                                             \
   }                                                                                                                                      \
   enum { NUM_PARAMETERS = 1 };                                                                                                           \
public:                                                                                                                                   \
   NAME(VAL0T val0) : mVal0(val0) {}                                                                                                      \
                                                                                                                                          \
   virtual ~NAME() {}                                                                                                                     \
                                                                                                                                          \
   virtual ActionType type() const { return TYPE; }                                                                                       \
                                                                                                                                          \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                      \
                                                                                                                                          \
   virtual void run(const ImageT& src,ImageT& tgt) const {                                                                                \
      runPr(src,tgt,view2roi(src.defaultView()),mVal0);                                                                                   \
   }                                                                                                                                      \
                                                                                                                                          \
   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {      \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                \
      VAL0T val0 = utility::parseWord<VAL0T>(parameters[0]);                                                                              \
      runPr(src,tgt,roi,val0);                                                                                                            \
   }                                                                                                                                      \
                                                                                                                                          \
   static NAME* make(std::istream& ins) {                                                                                                 \
      VAL0T val0 = utility::parseWord<VAL0T>(ins);                                                                                        \
      return new NAME<ImageT>(val0);                                                                                                      \
   }                                                                                                                                      \
};                                                                                                                                        \
/* End of ZERO_ARG_ACTION */

ONE_ARG_ACTION(Intensity,add,INTENSITY,int)
ONE_ARG_ACTION(ThresholdEqualizeOCV,thresholdEqualizeOCV,BINARIZE,unsigned)
ONE_ARG_ACTION(Binarize,binarize,BINARIZE,unsigned)
#ifdef SUPPORT_QRCODE_DETECT
ONE_ARG_ACTION(QRDecodeOCV,qrDecodeOCV,QR_DECODE,unsigned)
#endif
ONE_ARG_ACTION(UniformSmooth,uniformSmooth,UNIFORM_SMOOTH,unsigned)
ONE_ARG_ACTION(LPFilterResponse,lpResponse,FILTER_RESP,double)
ONE_ARG_ACTION(HPFilterResponse,hpResponse,FILTER_RESP,double)
ONE_ARG_ACTION(LPFilter,lpFilter,FILTER,double)
ONE_ARG_ACTION(HPFilter,hpFilter,FILTER,double)


#define TWO_ARG_ACTION(NAME,CALL,TYPE,VAL0T,VAL1T)                                                                 \
template<typename ImageT>                                                                                          \
class NAME : public Action<ImageT> {                                                                               \
public:                                                                                                            \
   typedef NAME<ImageT> ThisT;                                                                                     \
private:                                                                                                           \
   VAL0T mVal0;                                                                                                    \
   VAL1T mVal1;                                                                                                    \
                                                                                                                   \
   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,VAL0T val0, VAL1T val1) const {       \
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);                                              \
      algorithm::CALL(types::roi2view(src,roi),tgtview,val0,val1);                                                 \
   }                                                                                                               \
                                                                                                                   \
   enum { NUM_PARAMETERS = 2 };                                                                                    \
                                                                                                                   \
public:                                                                                                            \
   explicit NAME(VAL0T val0,VAL1T val1) : mVal0(val0), mVal1(val1) {}                                              \
                                                                                                                   \
   virtual ~NAME() {}                                                                                              \
                                                                                                                   \
   virtual ActionType type() const { return TYPE; }                                                                \
                                                                                                                   \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                               \
                                                                                                                   \
   virtual void run(const ImageT& src,ImageT& tgt) const {                                                         \
      run(src,tgt,view2roi(src.defaultView()),mVal0,mVal1);                                                        \
   }                                                                                                               \
                                                                                                                   \
   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,                              \
                                                  const types::ParameterPack& parameters) const {                  \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());         \
      VAL0T val0 = utility::parseWord<VAL0T>(parameters[0]);                                                       \
      VAL1T val1 = utility::parseWord<VAL1T>(parameters[1]);                                                       \
      run(src,tgt,roi,val0,val1);                                                                                  \
   }                                                                                                               \
                                                                                                                   \
   static NAME* make(std::istream& ins) {                                                                          \
      VAL0T val0 = utility::parseWord<VAL0T>(ins);                                                                 \
      VAL1T val1 = utility::parseWord<VAL1T>(ins);                                                                 \
      return new NAME<ImageT>(val0,val1);                                                                          \
   }                                                                                                               \
};                                                                                                                 \
/* End of Macro HISTACTION */

TWO_ARG_ACTION(HistogramModify,histogramModify,HISTOGRAM_MOD,unsigned,unsigned)
TWO_ARG_ACTION(HistogramModifyRGB,histogramModifyRGB,HISTOGRAM_MOD,unsigned,unsigned)
TWO_ARG_ACTION(HistogramModifyIntensity,histogramModifyIntensity,HISTOGRAM_MOD,unsigned,unsigned)
TWO_ARG_ACTION(BinarizeDT,binarizeDouble,BINARIZE,unsigned,unsigned)
TWO_ARG_ACTION(AfixAnyHSI,afixAnyHSI,AFIX_HSI,uint8_t,unsigned)
TWO_ARG_ACTION(BPFilterResponse,bpResponse,FILTER_RESP,double,double)
TWO_ARG_ACTION(BPFilter,bpFilter,FILTER,double,double)



#define ONE_ARG_GRAY_OUT_ACTION(NAME,CALL,TYPE,VAL0T)                                                                                                 \
template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >                      \
class NAME : public Action<ImageSrc,ImageTgt> {                                                                                                       \
public:                                                                                                                                               \
   typedef Action<ImageSrc,ImageTgt> SuperT;                                                                                                          \
   typedef NAME<ImageSrc,ImageTgt> ThisT;                                                                                                             \
   typedef typename ImageSrc::pixel_type pixel_type;                                                                                                  \
                                                                                                                                                      \
private:                                                                                                                                              \
   VAL0T mVal0;                                                                                                                                       \
                                                                                                                                                      \
   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,VAL0T val0) const {                                                  \
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);                                                                               \
      algorithm::CALL(types::roi2view(src,roi),tgtview,val0);                                                                                         \
   }                                                                                                                                                  \
                                                                                                                                                      \
   enum { NUM_PARAMETERS = 1 };                                                                                                                       \
                                                                                                                                                      \
public:                                                                                                                                               \
   explicit NAME(VAL0T val0) : mVal0(val0) {}                                                                                                         \
                                                                                                                                                      \
   virtual ~NAME() {}                                                                                                                                 \
                                                                                                                                                      \
   virtual ActionType type() const { return TYPE; }                                                                                                   \
                                                                                                                                                      \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {                                                                                        \
      run(src,tgt,view2roi(src.defaultView()),mVal0);                                                                                                 \
   }                                                                                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {              \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                            \
      VAL0T val0 = utility::parseWord<VAL0T>(parameters[0]);                                                                                          \
      run(src,tgt,roi,val0);                                                                                                                          \
   }                                                                                                                                                  \
                                                                                                                                                      \
   static NAME* make(std::istream& ins) {                                                                                                             \
      VAL0T val0 = utility::parseWord<VAL0T>(ins);                                                                                                    \
      return new NAME<ImageSrc,ImageTgt>(val0);                                                                                                       \
   }                                                                                                                                                  \
};                                                                                                                                                    \
/* End of ONE_ARG_GRAY_OUT_ACTION */

ONE_ARG_GRAY_OUT_ACTION(EdgeSobelOCV,edgeSobelOCV,EDGE,unsigned)
ONE_ARG_GRAY_OUT_ACTION(SelectHSI,selectHSI,SELECT_HSI,unsigned)
ONE_ARG_GRAY_OUT_ACTION(SelectColor,selectColor,SELECT_COLOR,unsigned)

ONE_ARG_GRAY_OUT_ACTION(EdgeGradient,edgeGradient,EDGE,unsigned)
ONE_ARG_GRAY_OUT_ACTION(EdgeDetect,edgeDetect,EDGE,unsigned)


#define TWO_ARG_GRAY_OUT_ACTION(NAME,CALL,TYPE,VAL0T,VAL1T)                                                                                           \
template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >                      \
class NAME : public Action<ImageSrc,ImageTgt> {                                                                                                       \
public:                                                                                                                                               \
   typedef Action<ImageSrc,ImageTgt> SuperT;                                                                                                          \
   typedef NAME<ImageSrc,ImageTgt> ThisT;                                                                                                             \
   typedef typename ImageSrc::pixel_type pixel_type;                                                                                                  \
                                                                                                                                                      \
private:                                                                                                                                              \
   VAL0T mVal0;                                                                                                                                       \
   VAL1T mVal1;                                                                                                                                       \
                                                                                                                                                      \
   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,VAL0T val0,VAL1T val1) const {                                       \
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);                                                                               \
      algorithm::CALL(types::roi2view(src,roi),tgtview,val0,val1);                                                                                    \
   }                                                                                                                                                  \
                                                                                                                                                      \
   enum { NUM_PARAMETERS = 2 };                                                                                                                       \
                                                                                                                                                      \
public:                                                                                                                                               \
   NAME(VAL0T val0,VAL1T val1) : mVal0(val0), mVal1(val1) {}                                                                                          \
                                                                                                                                                      \
   virtual ~NAME() {}                                                                                                                                 \
                                                                                                                                                      \
   virtual ActionType type() const { return TYPE; }                                                                                                   \
                                                                                                                                                      \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {                                                                                        \
      run(src,tgt,view2roi(src.defaultView()),mVal0,mVal1);                                                                                           \
   }                                                                                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {              \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                            \
      VAL0T val0 = utility::parseWord<VAL0T>(parameters[0]);                                                                                          \
      VAL1T val1 = utility::parseWord<VAL1T>(parameters[1]);                                                                                          \
      run(src,tgt,roi,val0,val1);                                                                                                                     \
   }                                                                                                                                                  \
                                                                                                                                                      \
   static NAME* make(std::istream& ins) {                                                                                                             \
      VAL0T val0 = utility::parseWord<VAL0T>(ins);                                                                                                    \
      VAL1T val1 = utility::parseWord<VAL1T>(ins);                                                                                                    \
      return new NAME<ImageSrc,ImageTgt>(val0,val1);                                                                                                  \
   }                                                                                                                                                  \
};                                                                                                                                                    \
/* End of EDGE_ACTION */

TWO_ARG_GRAY_OUT_ACTION(EdgeGradientClipped,edgeGradientClipped,EDGE,unsigned,double)
//TWO_ARG_GRAY_OUT_ACTION(EdgeGradient,edgeGradient,EDGE,unsigned,unsigned)
//TWO_ARG_GRAY_OUT_ACTION(EdgeDetect,edgeDetect,EDGE,unsigned,unsigned)


template<typename ImageT>
class HistogramModifyAnyRGB : public Action<ImageT> {
public:
   typedef HistogramModifyAnyRGB<ImageT> ThisT;

private:
   unsigned mLow;
   unsigned mHigh;
   unsigned mChannel;

   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,
            unsigned low, unsigned high,
            unsigned channel) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::histogramModifyAnyRGB(types::roi2view(src,roi),tgtview,
                         low,high,channel);
   }

   enum { NUM_PARAMETERS = 3 };

public:   
   HistogramModifyAnyRGB(unsigned low,unsigned high,
                         unsigned channel) : 
      mLow(low), mHigh(high),
      mChannel(channel) {}

   virtual ~HistogramModifyAnyRGB() {}

   virtual ActionType type() const { return HISTOGRAM_MOD; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mLow,mHigh,mChannel);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned low = utility::parseWord<unsigned>(parameters[0]);
      unsigned high = utility::parseWord<unsigned>(parameters[1]);
      unsigned channel = utility::parseWord<unsigned>(parameters[2]);
      run(src,tgt,roi,low,high,channel);
   }

   static HistogramModifyAnyRGB* make(std::istream& ins) {
      unsigned low = utility::parseWord<unsigned>(ins);
      unsigned high = utility::parseWord<unsigned>(ins);
      unsigned channel = utility::parseWord<unsigned>(ins);
      return new HistogramModifyAnyRGB<ImageT>(low,high,channel);                                     
   }
};


template<typename ImageT>
class HistogramModifyAnyHSI : public Action<ImageT> {
public:
   typedef HistogramModifyAnyHSI<ImageT> ThisT;

private:
   unsigned mLowI;
   unsigned mHighI;
   unsigned mLowS;
   unsigned mHighS;
   unsigned mLowH;
   unsigned mHighH;

   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,
            unsigned lowI, unsigned highI,
            unsigned lowS, unsigned highS,
            unsigned lowH, unsigned highH) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::histogramModifyHSI(types::roi2view(src,roi),tgtview,
                         lowI,highI,lowS,highS,lowH,highH);
   }

   enum { NUM_PARAMETERS = 6 };

public:   
   HistogramModifyAnyHSI(unsigned lowI,unsigned highI,
                         unsigned lowS,unsigned highS,
                         unsigned lowH,unsigned highH) : 
      mLowI(lowI), mHighI(highI),
      mLowS(lowS), mHighS(highS),
      mLowH(lowH), mHighH(highH) {}

   virtual ~HistogramModifyAnyHSI() {}

   virtual ActionType type() const { return HISTOGRAM_MOD; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mLowI,mHighI,mLowS,mHighS,mLowH,mHighH);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned lowI = utility::parseWord<unsigned>(parameters[0]);
      unsigned highI = utility::parseWord<unsigned>(parameters[1]);
      unsigned lowS = utility::parseWord<unsigned>(parameters[2]);
      unsigned highS = utility::parseWord<unsigned>(parameters[3]);
      unsigned lowH = utility::parseWord<unsigned>(parameters[4]);
      unsigned highH = utility::parseWord<unsigned>(parameters[5]);
      run(src,tgt,roi,lowI,highI,lowS,highS,lowH,highH);
   }

   static HistogramModifyAnyHSI* make(std::istream& ins) {
      unsigned lowI = utility::parseWord<unsigned>(ins);
      unsigned highI = utility::parseWord<unsigned>(ins);
      unsigned lowS = utility::parseWord<unsigned>(ins);
      unsigned highS = utility::parseWord<unsigned>(ins);
      unsigned lowH = utility::parseWord<unsigned>(ins);
      unsigned highH = utility::parseWord<unsigned>(ins);
      return new HistogramModifyAnyHSI<ImageT>(lowI,highI,lowS,highS,lowH,highH);                                     
   }
};

template<typename ImageT>
class BinarizeColor : public Action<ImageT> {
public:
   typedef Action<ImageT> SuperT;
   typedef BinarizeColor<ImageT> ThisT;
   typedef typename ImageT::pixel_type pixel_type;

private:
   float mThreshold;
   pixel_type mReferenceColor;

   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,float threshold,const pixel_type& referenceColor) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::binarizeColor(types::roi2view(src,roi),tgtview,threshold,referenceColor);
   }

   enum { NUM_PARAMETERS = 4 };

public:
   BinarizeColor(float threshold,const pixel_type& referenceColor) : mThreshold(threshold), mReferenceColor(referenceColor) {}

   virtual ~BinarizeColor() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mThreshold,mReferenceColor);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      float threshold = utility::parseWord<float>(parameters[0]);
      pixel_type referenceColor;
      referenceColor.namedColor.red = utility::parseWord<unsigned>(parameters[1]);
      referenceColor.namedColor.green = utility::parseWord<unsigned>(parameters[2]);
      referenceColor.namedColor.blue = utility::parseWord<unsigned>(parameters[3]);
      run(src,tgt,roi,threshold,referenceColor);
   }

   static BinarizeColor* make(std::istream& ins) {
      float threshold = utility::parseWord<float>(ins);
      pixel_type referenceColor;
      referenceColor.namedColor.red = utility::parseWord<unsigned>(ins);
      referenceColor.namedColor.green = utility::parseWord<unsigned>(ins);
      referenceColor.namedColor.blue = utility::parseWord<unsigned>(ins);
      return new BinarizeColor<ImageT>(threshold,referenceColor);
   }
};


#define ORIENTED_EDGE_ACTION(NAME,ALGO)                                                                                                               \
template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >                      \
class NAME : public Action<ImageSrc,ImageTgt> {                                                                                                       \
public:                                                                                                                                               \
   typedef Action<ImageSrc,ImageTgt> SuperT;                                                                                                          \
   typedef NAME<ImageSrc,ImageTgt> ThisT;                                                                                                             \
   typedef typename ImageSrc::pixel_type pixel_type;                                                                                                  \
                                                                                                                                                      \
private:                                                                                                                                              \
   /*unsigned mKernelType; */ /* TODO: should this use the enum? */                                                                                   \
   unsigned mWindowSize;                                                                                                                              \
   float mLowBound;                                                                                                                                   \
   float mHighBound;                                                                                                                                  \
                                                                                                                                                      \
   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi/*,unsigned kernelType*/,unsigned windowSize,                         \
            float lowBound,float highBound) const {                                                                                                   \
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);                                                                               \
      algorithm::ALGO(types::roi2view(src,roi),tgtview/*,(algorithm::edge::Kernel)kernelType*/,windowSize,lowBound,highBound);                        \
   }                                                                                                                                                  \
                                                                                                                                                      \
   enum { NUM_PARAMETERS = 3 };                                                                                                                       \
                                                                                                                                                      \
public:                                                                                                                                               \
   NAME(/*unsigned kernel,*/unsigned windowSize,float lowBound,float highBound) :                                                                     \
      /*mKernelType(kernel),*/mWindowSize(windowSize),mLowBound(lowBound),mHighBound(highBound) {}                                                    \
                                                                                                                                                      \
   virtual ~NAME() {}                                                                                                                                 \
                                                                                                                                                      \
   virtual ActionType type() const { return EDGE; }                                                                                                   \
                                                                                                                                                      \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {                                                                                        \
      run(src,tgt,view2roi(src.defaultView())/*,mKernelType*/,mWindowSize,mLowBound,mHighBound);                                                      \
   }                                                                                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {              \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                            \
      /*unsigned kernel = utility::parseWord<unsigned>(parameters[0]); */                                                                             \
      unsigned windowSize = utility::parseWord<unsigned>(parameters[0]);                                                                              \
      float lowBound = utility::parseWord<float>(parameters[1]);                                                                                      \
      float highBound = utility::parseWord<float>(parameters[2]);                                                                                     \
      run(src,tgt,roi/*,kernel*/,windowSize,lowBound,highBound);                                                                                      \
   }                                                                                                                                                  \
                                                                                                                                                      \
   static NAME* make(std::istream& ins) {                                                                                                             \
      /*unsigned kernel = utility::parseWord<unsigned>(ins);*/                                                                                        \
      unsigned windowSize = utility::parseWord<unsigned>(ins);                                                                                        \
      float lowBound = utility::parseWord<float>(ins);                                                                                                \
      float highBound = utility::parseWord<float>(ins);                                                                                               \
      return new NAME<ImageSrc,ImageTgt>(/*kernel,*/windowSize,lowBound,highBound);                                                                   \
   }                                                                                                                                                  \
};                                                                                                                                                    \
/* End of EDGE_ACTION */

ORIENTED_EDGE_ACTION(OrientedEdgeGradient,orientedEdgeGradient)
ORIENTED_EDGE_ACTION(OrientedEdgeDetect,orientedEdgeDetect)



template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >
class EdgeCannyOCV : public Action<ImageSrc,ImageTgt> {
public:
   typedef EdgeCannyOCV<ImageSrc> ThisT;

private:
   unsigned mWindowSize;
   double mLowThresh;
   double mHighThresh;

   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,unsigned windowSize,double tlow,double thigh) const {
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::edgeCannyOCV(types::roi2view(src,roi),tgtview,windowSize,tlow,thigh);
   }

   enum { NUM_PARAMETERS = 3 };

public:
   explicit EdgeCannyOCV(unsigned windowSize,double tlow,double thigh) : mWindowSize(windowSize), mLowThresh(tlow), mHighThresh(thigh) {}

   virtual ~EdgeCannyOCV() {}

   virtual ActionType type() const { return EDGE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mWindowSize,mLowThresh,mHighThresh);
   }

   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned windowSize = utility::parseWord<unsigned>(parameters[0]);
      double tlow = utility::parseWord<double>(parameters[1]);
      double thigh = utility::parseWord<double>(parameters[2]);
      run(src,tgt,roi,windowSize,tlow,thigh);
   }

   static EdgeCannyOCV* make(std::istream& ins) {
      unsigned windowSize = utility::parseWord<unsigned>(ins);
      double tlow = utility::parseWord<double>(ins);
      double thigh = utility::parseWord<double>(ins);
      return new EdgeCannyOCV<ImageSrc>(windowSize,tlow,thigh);
   }
};

#define FOUR_ARG_ACTION(NAME,CALL,TYPE,VAL0T,VAL1T,VAL2T,VAL3T)                                                                                              \
template<typename ImageT>                                                                                                                                    \
class NAME : public Action<ImageT> {                                                                                                                         \
public:                                                                                                                                                      \
   typedef Action<ImageT> SuperT;                                                                                                                            \
   typedef NAME<ImageT> ThisT;                                                                                                                               \
   typedef typename ImageT::pixel_type pixel_type;                                                                                                           \
                                                                                                                                                             \
private:                                                                                                                                                     \
   VAL0T mP0;                                                                                                                                                \
   VAL1T mP1;                                                                                                                                                \
   VAL2T mP2;                                                                                                                                                \
   VAL3T mP3;                                                                                                                                                \
                                                                                                                                                             \
   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,VAL0T p0,VAL1T p1,VAL2T p2,VAL3T p3) const {                                    \
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);                                                                                        \
      algorithm::CALL(types::roi2view(src,roi),tgtview,p0,p1,p2,p3);                                                                                         \
   }                                                                                                                                                         \
                                                                                                                                                             \
   enum { NUM_PARAMETERS = 4 };                                                                                                                              \
                                                                                                                                                             \
public:                                                                                                                                                      \
   NAME(VAL0T p0,VAL1T p1,VAL2T p2,VAL3T p3) : mP0(p0),mP1(p1),mP2(p2),mP3(p3) {}                                                                            \
                                                                                                                                                             \
   virtual ~NAME() {}                                                                                                                                        \
                                                                                                                                                             \
   virtual ActionType type() const { return TYPE; }                                                                                                          \
                                                                                                                                                             \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                                         \
                                                                                                                                                             \
   virtual void run(const ImageT& src,ImageT& tgt) const {                                                                                                   \
      run(src,tgt,view2roi(src.defaultView()),mP0,mP1,mP2,mP3);                                                                                              \
   }                                                                                                                                                         \
                                                                                                                                                             \
   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {                         \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                                   \
      VAL0T p0 = utility::parseWord<VAL0T>(parameters[0]);                                                                                                   \
      VAL1T p1 = utility::parseWord<VAL1T>(parameters[1]);                                                                                                   \
      VAL2T p2 = utility::parseWord<VAL2T>(parameters[2]);                                                                                                   \
      VAL3T p3 = utility::parseWord<VAL3T>(parameters[3]);                                                                                                   \
      run(src,tgt,roi,p0,p1,p2,p3);                                                                                                                          \
   }                                                                                                                                                         \
                                                                                                                                                             \
   static NAME* make(std::istream& ins) {                                                                                                                    \
      VAL0T p0 = utility::parseWord<VAL0T>(ins);                                                                                                             \
      VAL1T p1 = utility::parseWord<VAL1T>(ins);                                                                                                             \
      VAL2T p2 = utility::parseWord<VAL2T>(ins);                                                                                                             \
      VAL3T p3 = utility::parseWord<VAL3T>(ins);                                                                                                             \
      return new NAME<ImageT>(p0,p1,p2,p3);                                                                                                                  \
   }                                                                                                                                                         \
};                                                                                                                                                           \
/* FOUR_ARG_GRAY_OUT_ACTION */

FOUR_ARG_ACTION(Filter,filter,FILTER,double,double,double,double)
FOUR_ARG_ACTION(FilterResponse,filterResponse,FILTER,double,double,double,double)

} // namespace operation
} // namespace batchIP

