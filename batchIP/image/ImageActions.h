#pragma once

#include "ImageAction.h"
#include "RegionOfInterest.h"
#include "ImageAlgorithm.h"
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

   virtual ActionType type() const { return INTENSITY; }

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
class Intensity : public Action<ImageT> {
public:
   typedef Intensity<ImageT> ThisT;

private:
   int mAmount;

   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,int amount) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::add(types::roi2view(src,roi),tgtview,amount);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   explicit Intensity(int amount) : mAmount(amount) {}

   virtual ~Intensity() {}

   virtual ActionType type() const { return INTENSITY; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mAmount);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      int amount = utility::parseWord<int>(parameters[0]);
      run(src,tgt,roi,amount);
   }

   static Intensity* make(std::istream& ins) {
      int amount = utility::parseWord<int>(ins);
      return new Intensity<ImageT>(amount);
   }
};


#define HISTACTION(NAME,CALL)                                                                                      \
template<typename ImageT>                                                                                          \
class NAME : public Action<ImageT> {                                                                               \
public:                                                                                                            \
   typedef NAME<ImageT> ThisT;                                                                                     \
private:                                                                                                           \
   unsigned mLow;                                                                                                  \
   unsigned mHigh;                                                                                                 \
                                                                                                                   \
   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,unsigned low, unsigned high) const {  \
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);                                              \
      algorithm::CALL(types::roi2view(src,roi),tgtview,low,high);                                                  \
   }                                                                                                               \
                                                                                                                   \
   enum { NUM_PARAMETERS = 2 };                                                                                    \
                                                                                                                   \
public:                                                                                                            \
   explicit NAME(unsigned low,unsigned high) : mLow(low), mHigh(high) {}                                           \
                                                                                                                   \
   virtual ~NAME() {}                                                                                              \
                                                                                                                   \
   virtual ActionType type() const { return HISTOGRAM_MOD; }                                                       \
                                                                                                                   \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                               \
                                                                                                                   \
   virtual void run(const ImageT& src,ImageT& tgt) const {                                                         \
      run(src,tgt,view2roi(src.defaultView()),mLow,mHigh);                                                         \
   }                                                                                                               \
                                                                                                                   \
   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,                              \
                                                  const types::ParameterPack& parameters) const {                  \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());         \
      unsigned low = utility::parseWord<unsigned>(parameters[0]);                                                  \
      unsigned high = utility::parseWord<unsigned>(parameters[1]);                                                 \
      run(src,tgt,roi,low,high);                                                                                   \
   }                                                                                                               \
                                                                                                                   \
   static NAME* make(std::istream& ins) {                                                                          \
      unsigned low = utility::parseWord<unsigned>(ins);                                                            \
      unsigned high = utility::parseWord<unsigned>(ins);                                                           \
      return new NAME<ImageT>(low,high);                                                                           \
   }                                                                                                               \
};                                                                                                                 \
/* End of Macro HISTACTION */

HISTACTION(HistogramModify,histogramModify)
HISTACTION(HistogramModifyRGB,histogramModifyRGB)
HISTACTION(HistogramModifyIntensity,histogramModifyIntensity)


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
class Histogram : public Action<ImageT> {
public:
   typedef Histogram<ImageT> ThisT;

private:

   unsigned mLogBase;
   mutable bool mRunOnce;

   void runHist(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,unsigned logBase) const {
      algorithm::histogram(types::roi2view(src,roi),tgt,logBase);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   explicit Histogram(unsigned logBase) : mLogBase(logBase) {}

   virtual ~Histogram() {}

   virtual ActionType type() const { return HISTOGRAM; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      runHist(src,tgt,view2roi(src.defaultView()),mLogBase);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      if(!mRunOnce) {
         mRunOnce = true;
         utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
         unsigned logBase = utility::parseWord<unsigned>(parameters[0]);
         runHist(src,tgt,roi,logBase);
      }
      else {
         utility::fail("Histogram (hist) supports only one ROI (call multiple times or use crop first)!");
      }
   }

   static Histogram* make(std::istream& ins) {
      unsigned logBase = utility::parseWord<unsigned>(ins);
      return new Histogram<ImageT>(logBase);
   }
};


template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >
class HistogramChannel : public Action<ImageSrc,ImageTgt> {
public:
   typedef HistogramChannel<ImageSrc,ImageTgt> ThisT;

private:

   unsigned mLogBase;
   unsigned mChannel;
   mutable bool mRunOnce;

   void runHist(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,unsigned logBase,unsigned channel) const {
      algorithm::histogram(types::roi2view(src,roi),tgt,logBase,channel);
   }

   enum { NUM_PARAMETERS = 2 };

public:
   HistogramChannel(unsigned logBase,unsigned channel) : mLogBase(logBase),mChannel(channel), mRunOnce(false) {}

   virtual ~HistogramChannel() {}

   virtual ActionType type() const { return HISTOGRAM; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {
      runHist(src,tgt,view2roi(src.defaultView()),mLogBase,mChannel);
   }

   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      if(!mRunOnce) {
         mRunOnce = true;
         utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
         unsigned logBase = utility::parseWord<unsigned>(parameters[0]);
         unsigned channel = utility::parseWord<unsigned>(parameters[1]);
         runHist(src,tgt,roi,logBase,channel);
      }
      else {
         utility::fail("Histogram (histChan) supports only one ROI (call multiple times or use crop first)!");
      }
   }

   static HistogramChannel* make(std::istream& ins) {
      unsigned logBase = utility::parseWord<unsigned>(ins);
      unsigned channel = utility::parseWord<unsigned>(ins);
      return new HistogramChannel<ImageSrc,ImageTgt>(logBase,channel);
   }
};


template<typename ImageT>
class Binarize : public Action<ImageT> {
public:
   typedef Binarize<ImageT> ThisT;

private:
   unsigned mThreshold;

   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,unsigned threshold) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::binarize(types::roi2view(src,roi),tgtview,threshold);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   explicit Binarize(unsigned threshold) : mThreshold(threshold) {}
   
   virtual ~Binarize() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mThreshold);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned threshold = utility::parseWord<unsigned>(parameters[0]);
      run(src,tgt,roi,threshold);
   }

   static Binarize* make(std::istream& ins) {
      unsigned threshold = utility::parseWord<unsigned>(ins);
      return new Binarize<ImageT>(threshold);
   }
};


template<typename ImageT>
class OptimalBinarize : public Action<ImageT> {
public:
   typedef OptimalBinarize<ImageT> ThisT;

private:

   void runPr(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::optimalBinarize(types::roi2view(src,roi),tgtview);
   }

   enum { NUM_PARAMETERS = 0 };

public:
   virtual ~OptimalBinarize() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      runPr(src,tgt,view2roi(src.defaultView()));
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      runPr(src,tgt,roi);
   }

   static OptimalBinarize* make(std::istream& ins) {
      return new OptimalBinarize<ImageT>();
   }
};


template<typename ImageT>
class OtsuBinarize : public Action<ImageT> {
public:
   typedef OtsuBinarize<ImageT> ThisT;

private:

   void runPr(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::otsuBinarize(types::roi2view(src,roi),tgtview);
   }

   enum { NUM_PARAMETERS = 0 };

public:
   virtual ~OtsuBinarize() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      runPr(src,tgt,view2roi(src.defaultView()));
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      runPr(src,tgt,roi);
   }

   static OtsuBinarize* make(std::istream& ins) {
      return new OtsuBinarize<ImageT>();
   }
};


template<typename ImageT>
class BinarizeDT : public Action<ImageT> {
public:
   typedef Action<ImageT> SuperT;
   typedef BinarizeDT<ImageT> ThisT;

private:
   unsigned mThresholdLow;
   unsigned mThresholdHigh;

   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,unsigned thresholdLow,unsigned thresholdHigh) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::binarizeDouble(types::roi2view(src,roi),tgtview,thresholdLow,thresholdHigh);
   }

   enum { NUM_PARAMETERS = 2 };

public:
   BinarizeDT(unsigned thresholdLow,unsigned thresholdHigh) : 
      mThresholdLow(thresholdLow),
      mThresholdHigh(thresholdHigh)
   {}

   virtual ~BinarizeDT() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mThresholdLow,mThresholdHigh);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned thresholdLow = utility::parseWord<unsigned>(parameters[0]);
      unsigned thresholdHigh = utility::parseWord<unsigned>(parameters[1]);
      run(src,tgt,roi,thresholdLow,thresholdHigh);
   }

   static BinarizeDT* make(std::istream& ins) {
      unsigned thresholdLow = utility::parseWord<unsigned>(ins);
      unsigned thresholdHigh = utility::parseWord<unsigned>(ins);
      return new BinarizeDT<ImageT>(thresholdLow,thresholdHigh);
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


template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >
class SelectColor : public Action<ImageSrc,ImageTgt> {
public:
   typedef Action<ImageSrc,ImageTgt> SuperT;
   typedef SelectColor<ImageSrc,ImageTgt> ThisT;
   typedef typename ImageSrc::pixel_type pixel_type;

private:
   unsigned mChannel;

   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,unsigned channel) const {
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::selectColor(types::roi2view(src,roi),tgtview,channel);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   explicit SelectColor(unsigned channel) : mChannel(channel) {}

   virtual ~SelectColor() {}

   virtual ActionType type() const { return SELECT_COLOR; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mChannel);
   }

   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned channel = utility::parseWord<unsigned>(parameters[0]);
      run(src,tgt,roi,channel);
   }

   static SelectColor* make(std::istream& ins) {
      unsigned channel = utility::parseWord<unsigned>(ins);
      return new SelectColor<ImageSrc,ImageTgt>(channel);
   }
};


template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >
class SelectHSI : public Action<ImageSrc,ImageTgt> {
public:
   typedef Action<ImageSrc,ImageTgt> SuperT;
   typedef SelectHSI<ImageSrc,ImageTgt> ThisT;
   typedef typename ImageSrc::pixel_type pixel_type;

private:
   unsigned mChannel;

   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,unsigned channel) const {
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::selectHSI(types::roi2view(src,roi),tgtview,channel);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   explicit SelectHSI(unsigned channel) : mChannel(channel) {}

   virtual ~SelectHSI() {}

   virtual ActionType type() const { return SELECT_HSI; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mChannel);
   }

   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned channel = utility::parseWord<unsigned>(parameters[0]);
      run(src,tgt,roi,channel);
   }

   static SelectHSI* make(std::istream& ins) {
      unsigned channel = utility::parseWord<unsigned>(ins);
      return new SelectHSI<ImageSrc,ImageTgt>(channel);
   }
};


template<typename ImageT>
class AfixAnyHSI : public Action<ImageT> {
public:
   typedef Action<ImageT> SuperT;
   typedef AfixAnyHSI<ImageT> ThisT;
   typedef typename ImageT::pixel_type pixel_type;

private:
   uint8_t  mValue;
   unsigned mChannel;

   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,uint8_t value,unsigned channel) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::afixAnyHSI(types::roi2view(src,roi),tgtview,value,channel);
   }

   enum { NUM_PARAMETERS = 2 };

public:
   AfixAnyHSI(uint8_t value,unsigned channel) : mValue(value),mChannel(channel) {}

   virtual ~AfixAnyHSI() {}

   virtual ActionType type() const { return SELECT_HSI; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mValue,mChannel);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      uint8_t value = (uint8_t)utility::parseWord<unsigned>(parameters[0]);
      unsigned channel = utility::parseWord<unsigned>(parameters[1]);
      run(src,tgt,roi,value,channel);
   }

   static AfixAnyHSI* make(std::istream& ins) {
      uint8_t value = (uint8_t)utility::parseWord<unsigned>(ins);
      unsigned channel = utility::parseWord<unsigned>(ins);
      return new AfixAnyHSI<ImageT>(value,channel);
   }
};



template<typename ImageT>
class UniformSmooth : public Action<ImageT> {
public:
   typedef UniformSmooth<ImageT> ThisT;

private:
   unsigned mWindowSize;

   void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,unsigned windowSize) const {
      typename ImageT::image_view tgtview = types::roi2view(tgt,roi);
      algorithm::uniformSmooth(types::roi2view(src,roi),tgtview,windowSize);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   explicit UniformSmooth(unsigned windowSize) : mWindowSize(windowSize) {}
   
   virtual ~UniformSmooth() {}

   virtual ActionType type() const { return UNIFORM_SMOOTH; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt) const {
      run(src,tgt,view2roi(src.defaultView()),mWindowSize);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned windowSize = utility::parseWord<unsigned>(parameters[0]);
      run(src,tgt,roi,windowSize);
   }

   static UniformSmooth* make(std::istream& ins) {
      unsigned windowSize = utility::parseWord<unsigned>(ins);
      return new UniformSmooth<ImageT>(windowSize);
   }
};


#define EDGE_ACTION(NAME,ALGO)                                                                                                                        \
template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >                      \
class NAME : public Action<ImageSrc,ImageTgt> {                                                                                                       \
public:                                                                                                                                               \
   typedef Action<ImageSrc,ImageTgt> SuperT;                                                                                                          \
   typedef NAME<ImageSrc,ImageTgt> ThisT;                                                                                                             \
   typedef typename ImageSrc::pixel_type pixel_type;                                                                                                  \
                                                                                                                                                      \
private:                                                                                                                                              \
   unsigned mChannel;    /* TODO: this is stupid as a parameter for Grayscale stuff */                                                                \
   unsigned mKernelType; /* TODO: should this use the enum? */                                                                                        \
   unsigned mWindowSize;                                                                                                                              \
                                                                                                                                                      \
   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,unsigned channel,unsigned kernelType,unsigned windowSize) const {    \
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);                                                                               \
      algorithm::ALGO(types::roi2view(src,roi),tgtview,(algorithm::edge::Kernel)kernelType,windowSize,channel);                                       \
   }                                                                                                                                                  \
                                                                                                                                                      \
   enum { NUM_PARAMETERS = 3 };                                                                                                                       \
                                                                                                                                                      \
public:                                                                                                                                               \
   NAME(unsigned channel,unsigned kernel,unsigned windowSize) : mChannel(channel),mKernelType(kernel), mWindowSize(windowSize) {}                     \
                                                                                                                                                      \
   virtual ~NAME() {}                                                                                                                                 \
                                                                                                                                                      \
   virtual ActionType type() const { return EDGE; }                                                                                                   \
                                                                                                                                                      \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {                                                                                        \
      run(src,tgt,view2roi(src.defaultView()),mChannel,mKernelType,mWindowSize);                                                                      \
   }                                                                                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {              \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                            \
      unsigned channel = utility::parseWord<unsigned>(parameters[0]);                                                                                 \
      unsigned kernel = utility::parseWord<unsigned>(parameters[1]);                                                                                  \
      unsigned windowSize = utility::parseWord<unsigned>(parameters[2]);                                                                              \
      run(src,tgt,roi,channel,kernel,windowSize);                                                                                                     \
   }                                                                                                                                                  \
                                                                                                                                                      \
   static NAME* make(std::istream& ins) {                                                                                                             \
      unsigned channel = utility::parseWord<unsigned>(ins);                                                                                           \
      unsigned kernel = utility::parseWord<unsigned>(ins);                                                                                            \
      unsigned windowSize = utility::parseWord<unsigned>(ins);                                                                                        \
      return new NAME<ImageSrc,ImageTgt>(channel,kernel,windowSize);                                                                                  \
   }                                                                                                                                                  \
};                                                                                                                                                    \
/* End of EDGE_ACTION */

EDGE_ACTION(EdgeGradient,edgeGradient)
EDGE_ACTION(EdgeDetect,edgeDetect)


#define ORIENTED_EDGE_ACTION(NAME,ALGO)                                                                                                               \
template<typename ImageSrc,typename ImageTgt = types::Image<types::GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >                      \
class NAME : public Action<ImageSrc,ImageTgt> {                                                                                                       \
public:                                                                                                                                               \
   typedef Action<ImageSrc,ImageTgt> SuperT;                                                                                                          \
   typedef NAME<ImageSrc,ImageTgt> ThisT;                                                                                                             \
   typedef typename ImageSrc::pixel_type pixel_type;                                                                                                  \
                                                                                                                                                      \
private:                                                                                                                                              \
   unsigned mChannel;   /* TODO: this is stupid as a parameter for Grayscale stuff */                                                                 \
   unsigned mKernelType; /* TODO: should this use the enum? */                                                                                        \
   unsigned mWindowSize;                                                                                                                              \
   float mLowBound;                                                                                                                                   \
   float mHighBound;                                                                                                                                  \
                                                                                                                                                      \
   void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,unsigned channel,unsigned kernelType,unsigned windowSize,            \
            float lowBound,float highBound) const {                                                                                                   \
      typename ImageTgt::image_view tgtview = types::roi2view(tgt,roi);                                                                               \
      algorithm::ALGO(types::roi2view(src,roi),tgtview,(algorithm::edge::Kernel)kernelType,windowSize,channel,lowBound,highBound);                    \
   }                                                                                                                                                  \
                                                                                                                                                      \
   enum { NUM_PARAMETERS = 5 };                                                                                                                       \
                                                                                                                                                      \
public:                                                                                                                                               \
   NAME(unsigned channel,unsigned kernel,unsigned windowSize,                                                                                         \
        float lowBound,float highBound) : mChannel(channel),mKernelType(kernel),mWindowSize(windowSize),mLowBound(lowBound),mHighBound(highBound) {}  \
                                                                                                                                                      \
   virtual ~NAME() {}                                                                                                                                 \
                                                                                                                                                      \
   virtual ActionType type() const { return EDGE; }                                                                                                   \
                                                                                                                                                      \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt) const {                                                                                        \
      run(src,tgt,view2roi(src.defaultView()),mChannel,mKernelType,mWindowSize,mLowBound,mHighBound);                                                 \
   }                                                                                                                                                  \
                                                                                                                                                      \
   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,const types::ParameterPack& parameters) const {              \
      utility::reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                                            \
      unsigned channel = utility::parseWord<unsigned>(parameters[0]);                                                                                 \
      unsigned kernel = utility::parseWord<unsigned>(parameters[1]);                                                                                  \
      unsigned windowSize = utility::parseWord<unsigned>(parameters[2]);                                                                              \
      float lowBound = utility::parseWord<float>(parameters[3]);                                                                                      \
      float highBound = utility::parseWord<float>(parameters[4]);                                                                                     \
      run(src,tgt,roi,channel,kernel,windowSize,lowBound,highBound);                                                                                  \
   }                                                                                                                                                  \
                                                                                                                                                      \
   static NAME* make(std::istream& ins) {                                                                                                             \
      unsigned channel = utility::parseWord<unsigned>(ins);                                                                                           \
      unsigned kernel = utility::parseWord<unsigned>(ins);                                                                                            \
      unsigned windowSize = utility::parseWord<unsigned>(ins);                                                                                        \
      float lowBound = utility::parseWord<float>(ins);                                                                                                \
      float highBound = utility::parseWord<float>(ins);                                                                                               \
      return new NAME<ImageSrc,ImageTgt>(channel,kernel,windowSize,lowBound,highBound);                                                               \
   }                                                                                                                                                  \
};                                                                                                                                                    \
/* End of EDGE_ACTION */

ORIENTED_EDGE_ACTION(OrientedEdgeGradient,orientedEdgeGradient)
ORIENTED_EDGE_ACTION(OrientedEdgeDetect,orientedEdgeDetect)

} // namespace operation
} // namespace batchIP

