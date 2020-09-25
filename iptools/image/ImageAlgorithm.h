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
void add(const SrcImageT& src, TgtImageT& tgt, Value value,
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
void histogram(const SrcImageT& src, TgtImageT& tgt,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   reportIfNotEqual("cols!=max",tgt.cols(),(unsigned)TgtImageT::pixel_type::traits::max()+1u);
 
   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();

   // Thinking 4 billion is more than big enough for histogram height
   typedef std::vector<unsigned int> HistogramT;
   typedef typename HistogramT::iterator HistogramIterator;
   
   // First compute Histogram
   HistogramT histogram(TgtImageT::pixel_type::traits::max()+1,0u);
   unsigned int maxColumnHeight = 0;
   for(;spos != send;++spos) {
      ++histogram[spos->namedColor.gray];
      if(histogram[spos->namedColor.gray] > maxColumnHeight) ++maxColumnHeight;
   }

   // Now we need to normalize the Histogram to the bounds of the rows dimension

   HistogramIterator hpos = histogram.begin();
   const HistogramIterator hend = histogram.end();
   float normScale = (float) tgt.rows() / maxColumnHeight;
   for(;hpos != hend;++hpos) *hpos = static_cast<typename TgtImageT::pixel_type::value_type>(*hpos * normScale);

   // Now we need to compose the histogram, which is a little tricky if done with iteration
   // Instead, we'll simply use the pixel coordinates to paint. This would be much, much simpler if I first
   // created an Image transpose function...

   // 1) first whiten the whole image
   typename TgtImageT::iterator tpos = tgt.begin();
   typename TgtImageT::iterator tend = tgt.end();
   for(;tpos != tend;++tpos) tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();

   // 2) blacken the histograms working from the bottom
   unsigned height = tgt.rows();
   hpos = histogram.begin();
   //std::cout << "Histogram: ";
   for(unsigned col = 0;hpos != hend;++hpos,++col) {
      //std::cout << *hpos << " ";
      for(unsigned r = *hpos;r > 0;--r) {
         tgt.pixel(height - r,col).namedColor.gray = TgtImageT::pixel_type::traits::min();
      }
   }
   //std::cout << std::endl;
}

template<typename SrcImageT,typename TgtImageT>
void histogram(const SrcImageT& src, TgtImageT& tgt,unsigned channel,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   reportIfNotEqual("cols!=max",tgt.cols(),(unsigned)TgtImageT::pixel_type::traits::max()+1u);
   reportIfNotLessThan("channel < maxChannels",channel,(unsigned)TgtImageT::pixel_type::MAX_CHANNELS);
 
   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();

   // Thinking 4 billion is more than big enough for histogram height
   typedef std::vector<unsigned int> HistogramT;
   typedef typename HistogramT::iterator HistogramIterator;
   
   // First compute Histogram
   HistogramT histogram(TgtImageT::pixel_type::traits::max()+1,0u);
   unsigned int maxColumnHeight = 0;
   for(;spos != send;++spos) {
      ++histogram[spos->indexedColor[channel]];
      if(histogram[spos->indexedColor[channel]] > maxColumnHeight) ++maxColumnHeight;
   }

   // Now we need to normalize the Histogram to the bounds of the rows dimension

   HistogramIterator hpos = histogram.begin();
   const HistogramIterator hend = histogram.end();
   float normScale = (float) tgt.rows() / maxColumnHeight;
   for(;hpos != hend;++hpos) *hpos = static_cast<typename TgtImageT::pixel_type::value_type>(*hpos * normScale);

   // Now we need to compose the histogram, which is a little tricky if done with iteration
   // Instead, we'll simply use the pixel coordinates to paint. This would be much, much simpler if I first
   // created an Image transpose function...

   // 1) first whiten the whole image
   typename TgtImageT::iterator tpos = tgt.begin();
   typename TgtImageT::iterator tend = tgt.end();
   for(;tpos != tend;++tpos) tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();

   // 2) blacken the histograms working from the bottom
   unsigned height = tgt.rows();
   hpos = histogram.begin();
   //std::cout << "Histogram: ";
   for(unsigned col = 0;hpos != hend;++hpos,++col) {
      //std::cout << *hpos << " ";
      for(unsigned r = *hpos;r > 0;--r) {
         tgt.pixel(height - r,col).namedColor.gray = TgtImageT::pixel_type::traits::min();
      }
   }
   //std::cout << std::endl;
}

template<typename SrcImageT,typename TgtImageT,typename Value>
void histogramModify(const SrcImageT& src, TgtImageT& tgt,Value low,Value high,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   reportIfNotLessThan("cols!=max",low,high);

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   // Algorithm
   float rescale = (float) TgtImageT::pixel_type::traits::max()/(high - low);

   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray <= low) tpos->namedColor.gray = TgtImageT::pixel_type::traits::min();
      else if(spos->namedColor.gray <= high)
         tpos->namedColor.gray = static_cast<typename TgtImageT::pixel_type::value_type>(rescale * (spos->namedColor.gray - low));
      else tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();
   }
}

template<typename SrcPixelT,typename TgtPixelT,typename Bounds,typename Constraints>
inline void linearlyStretch(const SrcPixelT& src,TgtPixelT& tgt,float rescale,Bounds min, Bounds max,Constraints low,Constraints high) {
   if(src <= low) tgt = min;
   else if(src <= high) tgt = static_cast<Bounds>(rescale * (src - low));
   else tgt = max;
}

template<typename SrcImageT,typename TgtImageT,typename Value>
void histogramModifyRGB(const SrcImageT& src, TgtImageT& tgt,Value low,Value high,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   reportIfNotLessThan("cols!=max",low,high);

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   // Algorithm
   float rescale = (float) TgtImageT::pixel_type::traits::max()/(high - low);

   typename TgtImageT::pixel_type::value_type min = TgtImageT::pixel_type::traits::min();
   typename TgtImageT::pixel_type::value_type max = TgtImageT::pixel_type::traits::max();

   for(;spos != send;++spos,++tpos) {
      linearlyStretch(spos->namedColor.red,tpos->namedColor.red,rescale,min,max,low,high);
      linearlyStretch(spos->namedColor.green,tpos->namedColor.green,rescale,min,max,low,high);
      linearlyStretch(spos->namedColor.blue,tpos->namedColor.blue,rescale,min,max,low,high);
   }
}



template<typename SrcImageT,typename TgtImageT>
void add(const SrcImageT& src, TgtImageT& tgt, int value,
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
void binarize(const SrcImageT& src, TgtImageT& tgt, unsigned threshold,
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
void binarizeColor(const SrcImageT& src, TgtImageT& tgt, float thresholdDistance,
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

   // To avoid expensive sqrt on all distance calculations,
   // we may instead compare to the squared thresholdDistance.
   double thresholdDistance2 = thresholdDistance * thresholdDistance;
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
      //double distance = std::sqrt(diffr*diffr + diffg*diffg + diffb*diffb);
      double distance = diffr*diffr + diffg*diffg + diffb*diffb;

#ifdef DEBUG_BINARIZE_COLOR
      if(++count % 10000 == 0) 
         std::cout << "diffr=" << diffr << " "
                   << "diffg=" << diffg << " "
                   << "diffb=" << diffb << " "
                   << "distance=" << distance << " "
                   << "thresholdDistance2=" << thresholdDistance2 << " "
                   << "distance<thresholdDistance=" << (distance < thresholdDistance) << std::endl;
#endif

      if(distance < thresholdDistance2) {
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
void binarizeDouble(const SrcImageT& src, TgtImageT& tgt, unsigned thresholdLow,unsigned thresholdHigh,
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
struct PixelSquareSubtractor {
   AccumulatorVariableTT& mSquareAccumulator;
   PixelSquareSubtractor(AccumulatorVariableTT& squareAccumulator) : mSquareAccumulator(squareAccumulator) {}
   void operator()(const PixelT& pixel) { 
      mSquareAccumulator -= pixel.namedColor.gray * pixel.namedColor.gray; }
};


template<typename PixelT,typename AccumulatorVariableTT>
struct PixelSquareAdder {
   AccumulatorVariableTT& mSquareAccumulator;
   PixelSquareAdder(AccumulatorVariableTT& squareAccumulator) :  mSquareAccumulator(squareAccumulator) {}
   void operator()(const PixelT& pixel) { 
      mSquareAccumulator += pixel.namedColor.gray * pixel.namedColor.gray;
   }
};


// Computing Variance using the "Sum of Squares" method. Note:
// that under some circumstances that this approach is numerically unstable.
// For more details on this see:
// https://www.johndcook.com/blog/2008/09/26/comparing-three-methods-of-computing-standard-deviation/
//
// It was not apparent how to apply the Welford method to a moving windowed average/variance.
// But for low-precision (uint8_t) grayscale, we'd need to have rather large
// window size (larger than 16x16) to exceed the precision of the accumulators.
//
// Furthermore, for the derivation of the Variance + Mean^2 term, if the window
// shrinks to 1 (namely in the corners or edges of the Window, an unbiased variance calculation
// leads to divide by zero terms in both the sum of squares and the square of average term. If
// the biased mean, on the other hand is used, we have numerous benefits:
//   1) The square of the average term completely falls away (so does not need to be calculate)
//   2) The sum of the square terms is simply divided by the window size without needing
//      a difference term with large precision.
//   3) Window size may be increased rather dramatically from 15x15 to 257x257 (for uint8_t
//   grayscale, and unsigned int accumulator).
// I.e. using the biased estimate for variance provides a stable, numerical and faster solution.
// We also expect that the bias is in the same direction for all samples, and we really
// only care about the relative order of M^2 + S^2 terms for histogram equalization (by sorting),
// so bias is of no concern.
template<typename ImageViewT,
         typename PixelT = typename ImageViewT::pixel_type,
         typename AccumulatorVariableT = typename AccumulatorVariableSelect<PixelT>::type >
class SumOfSquares {
private:
   typedef ElasticImageView<const PixelT>                     ConstElasticViewT;
   typedef PixelSquareSubtractor<PixelT,AccumulatorVariableT> SubtractorT;
   typedef PixelSquareAdder<PixelT,AccumulatorVariableT>      AdderT;

   typename ImageViewT::const_image_view mBoundingView;
   ConstElasticViewT                     mElasticView;
   AccumulatorVariableT                  mSquareAccumulator;
   SubtractorT                           mSubtractor;
   AdderT                                mAdder;

public:
   typedef typename PixelT::value_type value_type;

   SumOfSquares(const ImageViewT& imageView,unsigned windowSize) :
      mBoundingView(imageView),
      mElasticView(mBoundingView.elastic_view(windowSize,windowSize)),
      // Initialize mAccumulator with element 0,0
      mSquareAccumulator(mElasticView.pixel(0,0).namedColor.gray*mElasticView.pixel(0,0).namedColor.gray),
      mSubtractor(mSquareAccumulator),
      mAdder(mSquareAccumulator)
   {}

   value_type squareAverage() {
      return static_cast<value_type>(checkValue<PixelT>(static_cast<AccumulatorVariableT>((double) mSquareAccumulator / mElasticView.size())));
   }

   void operator++() {
      mElasticView.moveRight(mSubtractor,mAdder);
   }
};


template<typename ParametricImageT>
void initParametricImage(ParametricImageT& parImage) {
   
   typedef typename ParametricImageT::pixel_type PixelT;

   for(unsigned r = 0; r < parImage.rows();++r) {
      for(unsigned c = 0; c < parImage.cols();++c) {
         PixelT& rpix = parImage.pixel(r,c);
         rpix.row = r;
         rpix.col = c;
      }
   }
}

template<typename SrcImageT,typename TgtImageT>
void histogramUnify(const SrcImageT& src, TgtImageT& tgt,unsigned windowSize,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef typename SrcImageT::pixel_type PixelT;
   typedef typename AccumulatorVariableSelect<PixelT>::type AccumulatorVariableT;
   typedef ParametricGrayAlphaPixel<AccumulatorVariableT> IntermediatePixelT;
   typedef Image<IntermediatePixelT> IntermediateImageT;

   reportIfNotLessThan("windowSize",2u,windowSize);
   reportIfNotEqual("windowSize (which should be odd)",windowSize-1,((windowSize >> 1u) << 1u));
   reportIfNotEqual("src.rows() != tgt.rows()",src.rows(),tgt.rows());
   reportIfNotEqual("src.cols() != tgt.cols()",src.cols(),tgt.cols());

   unsigned rows = src.rows();
   unsigned cols = src.cols();

   // We can write out result to iterator operating over entire passed Image or ImageView
   // since the iteration order is exactly the same as our loops.
   IntermediateImageT interImage(src.rows(),src.cols());
   // Start by initializing parametric values;
   initParametricImage(interImage);

   // TODO: To Complete!!!
//   typename TgtImageT::iterator tpos = tgt.begin();
//
//   // Start by smoothing along X for each row
//   for(unsigned i = 0;i < rows;++i) {
//      typedef SumOfSquares<SrcImageT> SquareSmoothT;
//      typedef typename SrcImageT::image_view RowViewT;
//      
//      RowViewT rowView(src.view(rows-i,cols,i));
//      SquareSmoothT squareSmooth(rowView,windowSize);
//      // We always start with the first answer precomputed
//      // which also implies we will only iterate and shift cols-1 times.
//      tpos->namedColor.gray = smoothX.average();
//      ++tpos;
//      for(unsigned j = 1;j < cols;++j,++tpos) {
//         ++smoothX;
//         tpos->namedColor.gray = smoothX.average();
//      }
//   }
//
//   typedef typename TgtImageT::image_view ColumnViewT; // not strictly a Column type, but will be
//                                                       // parameterized below to operate as one.
//   typedef typename ColumnViewT::iterator ColumnIteratorT;
//   // In this case our iteration order is flipped, and since we don't have a column-first
//   // iterator type, we just need to create column views to write out our output.
//   for(unsigned j = 0; j < cols; ++j) {
//      typedef SmoothY<TgtImageT> SmoothYT;
//      ColumnViewT columnView(tgt.view(rows,1,0,j));
//      SmoothYT smoothY(columnView,windowSize);
//      ColumnIteratorT cpos = columnView.begin();
//      // As above, we always start with the first answer precomputed
//      // which also implies we will only iterate and shift rows-1 times.
//      cpos->namedColor.gray = smoothY.average();
//      ++cpos;
//      for(unsigned i = 1;i < rows;++i,++cpos) {
//         ++smoothY;
//         cpos->namedColor.gray = smoothY.average();
//      }
//   }
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
void uniformSmooth(const SrcImageT& src, TgtImageT& tgt,unsigned windowSize,
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
void scale(const SrcImageT& src, TgtImageT& tgt, float ratio,
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
   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {
      scale(src,tgt,mAmount);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {
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

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,int amount) const {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      add(roi2view(src,roi),tgtview,amount);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   Intensity(int amount) : mAmount(amount) {}

   virtual ~Intensity() {}

   virtual ActionType type() const { return INTENSITY; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {
      run(src,tgt,roi,mAmount);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      int amount = parseWord<int>(parameters[0]);
      run(src,tgt,roi,amount);
   }

   static Intensity* make(std::istream& ins) {
      int amount = parseWord<int>(ins);
      return new Intensity<ImageT>(amount);
   }
};


#define HISTACTION(NAME,CALL)                                                                                            \
template<typename ImageT>                                                                                                \
class NAME : public Action<ImageT> {                                                                                     \
public:                                                                                                                  \
   typedef NAME<ImageT> ThisT;                                                                                           \
private:                                                                                                                 \
   unsigned mLow;                                                                                                        \
   unsigned mHigh;                                                                                                       \
                                                                                                                         \
   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,unsigned low, unsigned high) const {               \
      typename ImageT::image_view tgtview = roi2view(tgt,roi);                                                           \
      CALL(roi2view(src,roi),tgtview,low,high);                                                                          \
   }                                                                                                                     \
                                                                                                                         \
   enum { NUM_PARAMETERS = 2 };                                                                                          \
                                                                                                                         \
public:                                                                                                                  \
   NAME(unsigned low,unsigned high) : mLow(low), mHigh(high) {}                                                          \
                                                                                                                         \
   virtual ~NAME() {}                                                                                                    \
                                                                                                                         \
   virtual ActionType type() const { return HISTOGRAM_MOD; }                                                             \
                                                                                                                         \
   virtual unsigned numParameters() const { return NUM_PARAMETERS; }                                                     \
                                                                                                                         \
   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {                                   \
      run(src,tgt,roi,mLow,mHigh);                                                                                       \
   }                                                                                                                     \
                                                                                                                         \
   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {   \
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());                        \
      unsigned low = parseWord<unsigned>(parameters[0]);                                                                 \
      unsigned high = parseWord<unsigned>(parameters[1]);                                                                \
      run(src,tgt,roi,low,high);                                                                                         \
   }                                                                                                                     \
                                                                                                                         \
   static NAME* make(std::istream& ins) {                                                                                \
      unsigned low = parseWord<unsigned>(ins);                                                                           \
      unsigned high = parseWord<unsigned>(ins);                                                                          \
      return new NAME<ImageT>(low,high);                                                                                 \
   }                                                                                                                     \
};                                                                                                                       \
/* End of Macro HISTACTION */

HISTACTION(HistogramModify,histogramModify)
HISTACTION(HistogramModifyRGB,histogramModifyRGB)


template<typename ImageT>
class Histogram : public Action<ImageT> {
public:
   typedef Histogram<ImageT> ThisT;

private:

   void runHist(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {
      histogram(roi2view(src,roi),tgt);
   }

   enum { NUM_PARAMETERS = 0 };

public:
   Histogram() {}

   virtual ~Histogram() {}

   virtual ActionType type() const { return HISTOGRAM; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {
      runHist(src,tgt,roi);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      runHist(src,tgt,roi);
   }

   static Histogram* make(std::istream& ins) {
      return new Histogram<ImageT>();
   }
};


template<typename ImageSrc,typename ImageTgt = Image<GrayAlphaPixel<typename ImageSrc::pixel_type::value_type> > >
class HistogramChannel : public Action<ImageSrc,ImageTgt> {
public:
   typedef HistogramChannel<ImageSrc,ImageTgt> ThisT;

private:

   unsigned mChannel;

   void runHist(const ImageSrc& src,ImageTgt& tgt,const RegionOfInterest& roi,unsigned channel) const {
      histogram(roi2view(src,roi),tgt,channel);
   }

   enum { NUM_PARAMETERS = 0 };

public:
   HistogramChannel(unsigned channel) : mChannel(channel) {}

   virtual ~HistogramChannel() {}

   virtual ActionType type() const { return HISTOGRAM; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageSrc& src,ImageTgt& tgt,const RegionOfInterest& roi) const {
      runHist(src,tgt,roi,mChannel);
   }

   virtual void run(const ImageSrc& src,ImageTgt& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {
      reportIfNotEqual("parameters.size()",(unsigned)NUM_PARAMETERS,(unsigned)parameters.size());
      unsigned channel = parseWord<unsigned>(parameters[0]);
      runHist(src,tgt,roi,channel);
   }

   static HistogramChannel* make(std::istream& ins) {
      unsigned channel = parseWord<unsigned>(ins);
      return new HistogramChannel<ImageSrc,ImageTgt>(channel);
   }
};


template<typename ImageT>
class Binarize : public Action<ImageT> {
public:
   typedef Binarize<ImageT> ThisT;

private:
   unsigned mThreshold;

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,unsigned threshold) const {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      binarize(roi2view(src,roi),tgtview,threshold);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   Binarize(unsigned threshold) : mThreshold(threshold) {}
   
   virtual ~Binarize() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {
      run(src,tgt,roi,mThreshold);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {
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

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,unsigned thresholdLow,unsigned thresholdHigh) const {
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

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {
      run(src,tgt,roi,mThresholdLow,mThresholdHigh);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {
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

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,float threshold,const pixel_type& referenceColor) const {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      binarizeColor(roi2view(src,roi),tgtview,threshold,referenceColor);
   }

   enum { NUM_PARAMETERS = 4 };

public:
   BinarizeColor(float threshold,const pixel_type& referenceColor) : mThreshold(threshold), mReferenceColor(referenceColor) {}

   virtual ~BinarizeColor() {}

   virtual ActionType type() const { return BINARIZE; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {
      run(src,tgt,roi,mThreshold,mReferenceColor);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {
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

   void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,unsigned windowSize) const {
      typename ImageT::image_view tgtview = roi2view(tgt,roi);
      uniformSmooth(roi2view(src,roi),tgtview,windowSize);
   }

   enum { NUM_PARAMETERS = 1 };

public:
   UniformSmooth(unsigned windowSize) : mWindowSize(windowSize) {}
   
   virtual ~UniformSmooth() {}

   virtual ActionType type() const { return UNIFORM_SMOOTH; }

   virtual unsigned numParameters() const { return NUM_PARAMETERS; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) const {
      run(src,tgt,roi,mWindowSize);
   }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) const {
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


