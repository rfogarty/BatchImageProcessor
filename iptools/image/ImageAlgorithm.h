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
   static const ValueT min = static_cast<ValueT>(ChannelT::traits::max());
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

#if 0
template<typename PixelT,typename AccumulatorT = AccumulatorSelect<PixelT>::type >
class Accumulator {
private:
   typedef ElasticImageView<PixelT> ElasticViewT;
   ElasticViewT mView;
   AccumulatorT mAccumulator;

   struct
public:
   double average() {
      return (double) mAccumulator / mView.size();
   }

   void operator++() {
      mView.shiftRight();
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

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   reportIfNotLessThan("windowSize",2u,windowSize);
   reportIfNotEqual("windowSize (which should be odd)",windowSize,((windowSize >> 1u) << 1u));

   // Start by operating on border of region ("Adaptive box size")
   // starting with the smallest window size and growing as we move out to the largest window
   // size available.

   // Maybe instead I should come up with an elastic or shrinkable window
   //

   // Start by creating a view that is equal to the window size.
   

   for(;spos != send;++spos,++tpos) {

   }
}
#endif



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


} // namespace algorithm


