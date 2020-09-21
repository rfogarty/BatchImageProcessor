#pragma once

#include "Image.h"
#include "ImageAction.h"
#include "Pixel.h"
#include "utility/Error.h"
#include "utility/StringParse.h"
#include <cmath>

namespace algorithm {

template<typename ChannelT,typename ValueT>
inline ValueT checkValue(ValueT value)
{
   static const ValueT max = static_cast<ValueT>(ChannelT::traits::max());
   static const ValueT min = static_cast<ValueT>(ChannelT::traits::min());
   if (value > max) return max;
   if (value < min) return min;
   return value;
}

/*-----------------------------------------------------------------------**/

template<typename SrcImageT,typename TgtImageT,typename Value>
void add(const SrcImageT &src, TgtImageT &tgt, Value value,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {
   // TODO: SFINAE selection of integral, versus floating point types, or need way to select
   // signed value type that is larger than native type(if possible)
 
   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      *tpos = *spos;
      int gray = static_cast<int>(tpos->namedColor.gray);
      gray += value;
      tpos->namedColor.gray = checkValue<typename TgtImageT::pixel_type>(gray);
   }
}

template<typename SrcImageT,typename TgtImageT>
void add(const SrcImageT &src, TgtImageT &tgt, int value,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {
 
   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      *tpos = *spos;
      // Update Red
      int signedColor = tpos->namedColor.red;
      signedColor += value;
      tpos->namedColor.red = checkValue<typename TgtImageT::pixel_type>(signedColor);
      // Update Blue
      signedColor = tpos->namedColor.blue;
      signedColor += value;
      tpos->namedColor.blue = checkValue<typename TgtImageT::pixel_type>(signedColor);
      // Update Green
      signedColor = tpos->namedColor.green;
      signedColor += value;
      tpos->namedColor.green = checkValue<typename TgtImageT::pixel_type>(signedColor);
   }
}


/*-----------------------------------------------------------------------**/
template<typename SrcImageT,typename TgtImageT>
void binarize(const SrcImageT &src, TgtImageT &tgt, unsigned threshold,
              // This ugly bit is an unnamed argument with a default which means it neither           
              // contributes to the mangled declaration name nor requires an argument. So what is the 
              // point? It still participates in SFINAE to help select that this is an appropriate    
              // matching function given its arguments. Note, SFINAE techniques are incompatible with 
              // deduction so can't be applied to in parameter directly.                              
              typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray < threshold) tpos->namedColor.gray = TgtImageT::pixel_type::traits::min();
      else                                  tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();

   }
}

/*-----------------------------------------------------------------------**/
template<typename SrcImageT,typename TgtImageT>
void binarizeColor(const SrcImageT &src, TgtImageT &tgt, float thresholdDistance,
                   const typename SrcImageT::pixel_type& referenceColor,
                   // This ugly bit is an unnamed argument with a default which means it neither           
                   // contributes to the mangled declaration name nor requires an argument. So what is the 
                   // point? It still participates in SFINAE to help select that this is an appropriate    
                   // matching function given its arguments. Note, SFINAE techniques are incompatible with 
                   // deduction so can't be applied to in parameter directly.                              
                   typename std::enable_if<is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

#ifdef DEBUG_BINARIZE_COLOR
   unsigned count = 0;
#endif
   for(;spos != send;++spos,++tpos) {
      double diffr = (double)spos->namedColor.red -
                     (double)referenceColor.namedColor.red;
      double diffg = (double)spos->namedColor.green -
                     (double)referenceColor.namedColor.green;
      double diffb = (double)spos->namedColor.blue -
                     (double)referenceColor.namedColor.blue;
      double distance = std::sqrt(diffr*diffr + diffg*diffg + diffb*diffb);

#ifdef DEBUG_BINARIZE_COLOR
      if(++count % 10000 == 0) 
         std::cout << "diffr=" << diffr << " "
                   << "diffg=" << diffg << " "
                   << "diffb=" << diffb << " "
                   << "distance=" << distance << " "
                   << "thresholdDistance=" << thresholdDistance << " "
                   << "distance<thresholdDistance=" << (distance < thresholdDistance) << std::endl;
#endif

      if(distance < thresholdDistance) {
         // Anything below the threshold is white
         tpos->namedColor.red = TgtImageT::pixel_type::traits::max();
         tpos->namedColor.green = TgtImageT::pixel_type::traits::max();
         tpos->namedColor.blue = TgtImageT::pixel_type::traits::max();
      }
      else {
         // Anything above the threshold is red
         tpos->namedColor.red = TgtImageT::pixel_type::traits::max();
         tpos->namedColor.green = TgtImageT::pixel_type::traits::min();
         tpos->namedColor.blue = TgtImageT::pixel_type::traits::min();
      }
   }
}



/*-----------------------------------------------------------------------**/
template<typename SrcImageT,typename TgtImageT>
void binarizeDouble(const SrcImageT &src, TgtImageT &tgt, unsigned thresholdLow,unsigned thresholdHigh,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray < thresholdLow || spos->namedColor.gray >= thresholdHigh)
         tpos->namedColor.gray = TgtImageT::pixel_type::traits::min();
      else
         tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();

   }
}




template<typename PixelT,typename AccumulatorVariableTT>
struct PixelSubtractor {
   AccumulatorVariableTT& mAccumulator;
   PixelSubtractor(AccumulatorVariableTT& accumulator) : mAccumulator(accumulator) {}
   void operator()(const PixelT& pixel) { mAccumulator -= pixel.namedColor.gray; }
};

template<typename PixelT,typename AccumulatorVariableTT>
struct PixelAdder {
   AccumulatorVariableTT& mAccumulator;
   PixelAdder(AccumulatorVariableTT& accumulator) : mAccumulator(accumulator) {}
   void operator()(const PixelT& pixel) { mAccumulator += pixel.namedColor.gray; }
};



template<typename ImageViewT,
         typename PixelT = typename ImageViewT::pixel_type,
         typename AccumulatorVariableT = typename AccumulatorVariableSelect<PixelT>::type >
class SmoothX {
private:
   typedef ElasticImageView<const PixelT>               ConstElasticViewT;
   typedef PixelSubtractor<PixelT,AccumulatorVariableT> SubtractorT;
   typedef PixelAdder<PixelT,AccumulatorVariableT>      AdderT;

   typename ImageViewT::const_image_view mBoundingView;
   ConstElasticViewT                     mElasticView;
   AccumulatorVariableT                  mAccumulator;
   SubtractorT                           mSubtractor;
   AdderT                                mAdder;

public:
   typedef typename PixelT::value_type value_type;

   SmoothX(const ImageViewT& imageView,unsigned windowSize) :
      mBoundingView(imageView),
      mElasticView(mBoundingView.elastic_view(1,windowSize)),
      // Initialize mAccumulator with element 0,0
      mAccumulator(mElasticView.pixel(0,0).namedColor.gray),
      mSubtractor(mAccumulator),
      mAdder(mAccumulator)
   {}

   value_type average() {
      return static_cast<value_type>(checkValue<PixelT>(static_cast<AccumulatorVariableT>((double) mAccumulator / mElasticView.size())));
   }

   void operator++() {
      mElasticView.moveRight(mSubtractor,mAdder);
   }
};

template<typename ImageViewT,
         typename PixelT = typename ImageViewT::pixel_type,
         typename AccumulatorVariableT = typename AccumulatorVariableSelect<PixelT>::type >
class SmoothY {
private:
   typedef ElasticImageView<const PixelT>               ConstElasticViewT;
   typedef PixelSubtractor<PixelT,AccumulatorVariableT> SubtractorT;
   typedef PixelAdder<PixelT,AccumulatorVariableT>      AdderT;

   typename ImageViewT::const_image_view mBoundingView;
   ConstElasticViewT                     mElasticView;
   AccumulatorVariableT                  mAccumulator;
   SubtractorT                           mSubtractor;
   AdderT                                mAdder;

public:
   typedef typename PixelT::value_type value_type;

   SmoothY(const ImageViewT& imageView,unsigned windowSize) :
      mBoundingView(imageView),
      mElasticView(mBoundingView.elastic_view(windowSize,1)),
      // Initialize mAccumulator with element 0,0
      mAccumulator(mElasticView.pixel(0,0).namedColor.gray),
      mSubtractor(mAccumulator),
      mAdder(mAccumulator)
   {}

   value_type average() {
      return static_cast<value_type>(checkValue<PixelT>(static_cast<AccumulatorVariableT>((double) mAccumulator / mElasticView.size())));
   }

   void operator++() {
      mElasticView.moveDown(mSubtractor,mAdder);
   }
};


/*-----------------------------------------------------------------------**/
template<typename SrcImageT,typename TgtImageT>
void uniformSmooth(const SrcImageT &src, TgtImageT &tgt,unsigned windowSize,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   reportIfNotLessThan("windowSize",2u,windowSize);
   reportIfNotEqual("windowSize (which should be odd)",windowSize-1,((windowSize >> 1u) << 1u));
   reportIfNotEqual("src.rows() != tgt.rows()",src.rows(),tgt.rows());
   reportIfNotEqual("src.cols() != tgt.cols()",src.cols(),tgt.cols());

   unsigned rows = src.rows();
   unsigned cols = src.cols();

   // We can write out result to iterator operating over entire passed Image or ImageView
   // since the iteration order is exactly the same as our loops.
   typename TgtImageT::iterator tpos = tgt.begin();

   // Start by smoothing along X for each row
   for(unsigned i = 0;i < rows;++i) {
      typedef SmoothX<SrcImageT> SmoothXT;
      typedef typename SrcImageT::image_view RowViewT;
      
      RowViewT rowView(src.view(1,cols,i));
      SmoothXT smoothX(rowView,windowSize);
      // We always start with the first answer precomputed
      // which also implies we will only iterate and shift cols-1 times.
      tpos->namedColor.gray = smoothX.average();
      ++tpos;
      for(unsigned j = 1;j < cols;++j,++tpos) {
         ++smoothX;
         tpos->namedColor.gray = smoothX.average();
      }
   }

   typedef typename TgtImageT::image_view ColumnViewT; // not strictly a Column type, but will be
                                                       // parameterized below to operate as one.
   typedef typename ColumnViewT::iterator ColumnIteratorT;
   // In this case our iteration order is flipped, and since we don't have a column-first
   // iterator type, we just need to create column views to write out our output.
   for(unsigned j = 0; j < cols; ++j) {
      typedef SmoothY<TgtImageT> SmoothYT;
      ColumnViewT columnView(tgt.view(rows,1,0,j));
      SmoothYT smoothY(columnView,windowSize);
      ColumnIteratorT cpos = columnView.begin();
      // As above, we always start with the first answer precomputed
      // which also implies we will only iterate and shift rows-1 times.
      cpos->namedColor.gray = smoothY.average();
      ++cpos;
      for(unsigned i = 1;i < rows;++i,++cpos) {
         ++smoothY;
         cpos->namedColor.gray = smoothY.average();
      }
   }
}

/*-----------------------------------------------------------------------**/
template<typename SrcImageT,typename TgtImageT>
void scale(const SrcImageT &src, TgtImageT &tgt, float ratio,
              // This ugly bit is an unnamed argument with a default which means it neither           
              // contributes to the mangled declaration name nor requires an argument. So what is the 
              // point? It still participates in SFINAE to help select that this is an appropriate    
              // matching function given its arguments. Note, SFINAE techniques are incompatible with 
              // deduction so can't be applied to in parameter directly.                              
              typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   if(ratio > 1.9f) { // assume ratio is double
      tgt.resize(src.rows()*2,src.cols()*2);
      unsigned rows = src.rows()*2;
      unsigned cols = src.cols()*2;
      for(unsigned i = 0; i < rows; ++i) {
         unsigned i2 = i >> 1; // divide by 2
         for(unsigned j = 0; j < cols; ++j) {
            unsigned j2 = j >> 1; // divide by 2
            tgt.pixel(i,j) = src.pixel(i2,j2);
         }
      }
   }
   else if(ratio < 0.55) { // assume ratio is half
      typedef typename SrcImageT::pixel_type pixel_type;
      typedef typename pixel_type::value_type value_type;
      typedef typename AccumulatorVariableSelect<pixel_type>::type AccumulatorT;

      tgt.resize(src.rows()/2,src.cols()/2);
      unsigned rows = src.rows() >> 1;
      unsigned cols = src.cols() >> 1;
      for(unsigned i = 0; i < rows; ++i) {
         unsigned i2 = i << 1; // multiply by 2
         for(unsigned j = 0; j < cols; ++j) {
            unsigned j2 = j << 1; // multiply by 2
            // average the values of four pixels
            AccumulatorT acc = src.pixel(i2,j2).namedColor.gray;
            acc += src.pixel(i2,j2+1).namedColor.gray;
            acc += src.pixel(i2+1,j2).namedColor.gray;
            acc += src.pixel(i2+1,j2+1).namedColor.gray;
            // Now divide value by 4
            acc >>= 2;
            tgt.pixel(i,j).namedColor.gray = static_cast<value_type>(checkValue<pixel_type>(acc));
         }
      }
   }
}


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
   Scale(int amount) : mAmount(amount) {}

   virtual ~Scale() {}

   virtual ActionType type() const { return INTENSITY; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   // Note: RegionOfInterest is ignored.
   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      scale(src,tgt,mAmount);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) {
      fail("scale function does not support regions");
   }

   static Scale* make(std::istream& ins) {
      int amount = parseWord<float>(ins);
      return new Scale<ImageT>(amount);
   }
};


template<typename ImageT>
class Intensity : public Action<ImageT> {
public:
   typedef Intensity<ImageT> ThisT;

private:
   int mAmount;

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,int amount) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      add(roi2view(src,roi),tgtview,amount);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   Intensity(int amount) : mAmount(amount) {}

   virtual ~Intensity() {}

   virtual ActionType type() const { return INTENSITY; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      run(src,tgt,roi,mAmount);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) {
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      int amount = parseWord<int>(parameters[0]);
      run(src,tgt,roi,amount);
   }

   static Intensity* make(std::istream& ins) {
      int amount = parseWord<int>(ins);
      return new Intensity<ImageT>(amount);
   }
};

template<typename ImageT>
class Binarize : public Action<ImageT> {
public:
   typedef Binarize<ImageT> ThisT;

private:
   unsigned mThreshold;

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,unsigned threshold) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      binarize(roi2view(src,roi),tgtview,threshold);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   Binarize(unsigned threshold) : mThreshold(threshold) {}
   
   virtual ~Binarize() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      run(src,tgt,roi,mThreshold);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) {
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned threshold = parseWord<unsigned>(parameters[0]);
      run(src,tgt,roi,threshold);
   }

   static Binarize* make(std::istream& ins) {
      unsigned threshold = parseWord<unsigned>(ins);
      return new Binarize<ImageT>(threshold);
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

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,unsigned thresholdLow,unsigned thresholdHigh) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      binarizeDouble(roi2view(src,roi),tgtview,thresholdLow,thresholdHigh);
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

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      run(src,tgt,roi,mThresholdLow,mThresholdHigh);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) {
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned thresholdLow = parseWord<unsigned>(parameters[0]);
      unsigned thresholdHigh = parseWord<unsigned>(parameters[1]);
      run(src,tgt,roi,thresholdLow,thresholdHigh);
   }

   static BinarizeDT* make(std::istream& ins) {
      unsigned thresholdLow = parseWord<unsigned>(ins);
      unsigned thresholdHigh = parseWord<unsigned>(ins);
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

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,float threshold,const pixel_type& referenceColor) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      binarizeColor(roi2view(src,roi),tgtview,threshold,referenceColor);
   }

   enum { NUM_PARAMETERS = 4 };

public:
   BinarizeColor(float threshold,const pixel_type& referenceColor) : mThreshold(threshold), mReferenceColor(referenceColor) {}

   virtual ~BinarizeColor() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      run(src,tgt,roi,mThreshold,mReferenceColor);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) {
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      float threshold = parseWord<float>(parameters[0]);
      pixel_type referenceColor;
      referenceColor.namedColor.red = parseWord<unsigned>(parameters[1]);
      referenceColor.namedColor.green = parseWord<unsigned>(parameters[2]);
      referenceColor.namedColor.blue = parseWord<unsigned>(parameters[3]);
      run(src,tgt,roi,threshold,referenceColor);
   }

   static BinarizeColor* make(std::istream& ins) {
      float threshold = parseWord<float>(ins);
      pixel_type referenceColor;
      referenceColor.namedColor.red = parseWord<unsigned>(ins);
      referenceColor.namedColor.green = parseWord<unsigned>(ins);
      referenceColor.namedColor.blue = parseWord<unsigned>(ins);
      return new BinarizeColor<ImageT>(threshold,referenceColor);
   }
};

template<typename ImageT>
class UniformSmooth : public Action<ImageT> {
public:
   typedef UniformSmooth<ImageT> ThisT;

private:
   unsigned mWindowSize;

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,unsigned windowSize) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      uniformSmooth(roi2view(src,roi),tgtview,windowSize);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   UniformSmooth(unsigned windowSize) : mWindowSize(windowSize) {}
   
   virtual ~UniformSmooth() {}

   virtual ActionType type() const { return UNIFORM_SMOOTH; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      run(src,tgt,roi,mWindowSize);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) {
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned windowSize = parseWord<unsigned>(parameters[0]);
      run(src,tgt,roi,windowSize);
   }

   static UniformSmooth* make(std::istream& ins) {
      unsigned windowSize = parseWord<unsigned>(ins);
      return new UniformSmooth<ImageT>(windowSize);
   }
};


} // namespace algorithm


