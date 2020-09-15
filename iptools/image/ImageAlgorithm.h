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
                   const typename SrcImageT::pixel_type& thresholdBase,
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
      double diffr = (double)spos->namedColor.red -
                     (double)thresholdBase.namedColor.red;
      double diffg = (double)spos->namedColor.green -
                     (double)thresholdBase.namedColor.green;
      double diffb = (double)spos->namedColor.blue -
                     (double)thresholdBase.namedColor.blue;
      double distance = std::sqrt(diffr*diffr + diffg*diffg + diffb*diffb);

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



///*-----------------------------------------------------------------------**/
//void utility::scale(image &src, image &tgt, float ratio)
//{
//   int rows = (int)((float)src.getNumberOfRows() * ratio);
//   int cols  = (int)((float)src.getNumberOfColumns() * ratio);
//   tgt.resize(rows, cols);
//   for (int i=0; i<rows; i++)
//   {
//      for (int j=0; j<cols; j++)
//      {   
//         /* Map the pixel of new image back to original image */
//         int i2 = (int)floor((float)i/ratio);
//         int j2 = (int)floor((float)j/ratio);
//         if (ratio == 2) {
//            /* Directly copy the value */
//            tgt.setPixel(i,j,checkValue(src.getPixel(i2,j2)));
//         }
//
//         if (ratio == 0.5) {
//            /* Average the values of four pixels */
//            int value = src.getPixel(i2,j2) + src.getPixel(i2,j2+1) + src.getPixel(i2+1,j2) + src.getPixel(i2+1,j2+1);
//            tgt.setPixel(i,j,checkValue(value/4));
//         }
//      }
//   }
//}



template<typename ImageT>
class Intensity : public Action<ImageT> {
public:
   typedef Intensity<ImageT> ThisT;

private:
   int mAmount;

public:
   Intensity(int amount) : mAmount(amount) {}

   virtual ~Intensity() {}

   virtual ActionType type() { return INTENSITY; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      add(roi2view(src,roi),tgtview,mAmount);
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

public:
   Binarize(unsigned threshold) : mThreshold(threshold) {}
   
   virtual ~Binarize() {}

   virtual ActionType type() { return BINARIZE; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      binarize(roi2view(src,roi),tgtview,mThreshold);
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

public:
   BinarizeDT(unsigned thresholdLow,unsigned thresholdHigh) : 
      mThresholdLow(thresholdLow),
      mThresholdHigh(thresholdHigh)
   {}

   virtual ~BinarizeDT() {}

   virtual ActionType type() { return BINARIZE; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      binarizeDouble(roi2view(src,roi),tgtview,mThresholdLow,mThresholdHigh);
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
   pixel_type mFromPos;

public:
   BinarizeColor(float threshold,const pixel_type& fromPos) : mThreshold(0), mFromPos(fromPos) {}

   virtual ~BinarizeColor() {}

   virtual ActionType type() { return BINARIZE; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      binarizeColor(roi2view(src,roi),tgtview,mThreshold,mFromPos);
   }

   static BinarizeColor* make(std::istream& ins) {
      float threshold = parseWord<float>(ins);
      pixel_type fromPos;
      fromPos.namedColor.red = parseWord<unsigned>(ins);
      fromPos.namedColor.green = parseWord<unsigned>(ins);
      fromPos.namedColor.blue = parseWord<unsigned>(ins);
      return new BinarizeColor<ImageT>(threshold,fromPos);
   }
};

template<typename ImageT>
class UniformSmooth : public Action<ImageT> {
public:
   typedef UniformSmooth<ImageT> ThisT;

private:
   unsigned mWindowSize;

public:
   UniformSmooth(unsigned windowSize) : mWindowSize(windowSize) {}
   
   virtual ~UniformSmooth() {}

   virtual ActionType type() { return UNIFORM_SMOOTH; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      uniformSmooth(roi2view(src,roi),tgtview,mWindowSize);
   }

   static UniformSmooth* make(std::istream& ins) {
      unsigned windowSize = parseWord<unsigned>(ins);
      return new UniformSmooth<ImageT>(windowSize);
   }
};


} // namespace algorithm


