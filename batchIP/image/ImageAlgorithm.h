#pragma once

#include "Image.h"
#include "Pixel.h"
#include "utility/Error.h"
#include <cmath>
#include <iostream>

namespace batchIP {
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
         typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {
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


// Thinking 4 billion is more than big enough for any histogram height
typedef std::vector<double> HistogramT;

template<typename SrcImageT>
HistogramT computeHistogram(const SrcImageT& src,unsigned cols,unsigned channel,double& maxColumnHeight) {

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();

   // Compute Histogram
   HistogramT histogram(cols,0u);
   maxColumnHeight = 0;
   for(;spos != send;++spos) {
      ++histogram[spos->indexedColor[channel]];
      if(histogram[spos->indexedColor[channel]] > maxColumnHeight) ++maxColumnHeight;
   }
   return histogram;
}


void normalizeHistogram(HistogramT& histogram,unsigned rows,double maxColumnHeight) {
   // Normalize the Histogram to the bounds of the rows dimension, to scale to drawing
   HistogramT::iterator       hpos = histogram.begin();
   const HistogramT::iterator hend = histogram.end();

   double normScale1 = (double) 1.0 / maxColumnHeight;
   double normScale2 = (double) rows;
   
   for(;hpos != hend;++hpos) *hpos = std::min(1.0,*hpos * normScale1)*normScale2;
}


void logNormalizeHistogram(HistogramT& histogram,unsigned rows,double maxColumnHeight,unsigned logBase) {
   // Log Normalize the Histogram to the bounds of the rows dimension, to scale to drawing
   HistogramT::iterator       hpos = histogram.begin();
   const HistogramT::iterator hend = histogram.end();

   // TODO: All log functions are scale invariant, so doesn't really 
   // matter what the base is. It's possible to define a similar
   // function that that "exaggerates values", basically we divide
   // by a sigmoid funcion (or something like that). TBD...
   double logD = std::log(2.0);
   double logMaxColumnHeight = std::log(maxColumnHeight)/logD + 1.0;

   double normScale1 = (double) 1.0 / logMaxColumnHeight;
   double normScale2 = (double) rows;
   
   for(;hpos != hend;++hpos) {
      *hpos = *hpos > 0.0 ? std::max(0.0,std::log(*hpos)/logD) + 1.0 : 0.0;
      *hpos = std::min(1.0,*hpos * normScale1)*normScale2;
   }
}


template<typename TgtImageT>
void drawHistogram(TgtImageT& tgt,const HistogramT& histogram) {
   // Now we need to compose the histogram, which is a little tricky if done with iterators.
   // Instead, we'll simply use the pixel coordinates to paint. This would be much, much simpler if I first
   // created an Image transpose function...

   // 1) first whiten the whole image
   typename TgtImageT::iterator tpos = tgt.begin();
   typename TgtImageT::iterator tend = tgt.end();
   for(;tpos != tend;++tpos) tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();

   // 2) blacken the histograms working from the top, down
   unsigned height = tgt.rows();
   HistogramT::const_iterator hpos = histogram.begin();
   HistogramT::const_iterator hend = histogram.end();
   for(unsigned col = 0;hpos != hend;++hpos,++col) {
      for(unsigned r = (unsigned)*hpos;r > 0;--r) {
         tgt.pixel(height - r,col).namedColor.gray = TgtImageT::pixel_type::traits::min();
      }
   }
}


template<typename SrcImageT,typename TgtImageT>
void histogram(const SrcImageT& src, TgtImageT& tgt,unsigned logBase,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotEqual("cols!=max",tgt.cols(),(unsigned)TgtImageT::pixel_type::traits::max()+1u);
 
   // First compute Histogram
   double maxColumnHeight = 0;
   HistogramT histogram(computeHistogram(src,TgtImageT::pixel_type::traits::max()+1u,SrcImageT::pixel_type::GRAY_CHANNEL,maxColumnHeight));

   if(logBase < 2) normalizeHistogram(histogram,tgt.rows(),maxColumnHeight);
   else logNormalizeHistogram(histogram,tgt.rows(),maxColumnHeight,logBase);

   drawHistogram(tgt,histogram);
}

template<typename SrcImageT,typename TgtImageT>
void histogram(const SrcImageT& src, TgtImageT& tgt,unsigned logBase,unsigned channel,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotEqual("cols!=max",tgt.cols(),(unsigned)SrcImageT::pixel_type::traits::max()+1u);
   utility::reportIfNotLessThan("channel < maxChannels",channel,(unsigned)SrcImageT::pixel_type::MAX_CHANNELS);
 
   // First compute Histogram
   double maxColumnHeight = 0;
   HistogramT histogram(computeHistogram(src,TgtImageT::pixel_type::traits::max()+1u,channel,maxColumnHeight));

   if(logBase < 2) normalizeHistogram(histogram,tgt.rows(),maxColumnHeight);
   else logNormalizeHistogram(histogram,tgt.rows(),maxColumnHeight,logBase);

   drawHistogram(tgt,histogram);
}


// Note: with this algorithm, we can stretch each HSI channel separately
template<typename SrcImageT,typename TgtImageT,typename Value>
void afixAnyHSI(const SrcImageT& src, TgtImageT& tgt,
         Value value,unsigned channel,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotLessThan("value<max",value,SrcImageT::pixel_type::traits::max());

   // Algorithm
   // Idea here is simple. 
   // 1) Convert image from RGBA to HSI
   // 2) assigned afixed value to appropriate channel
   // 3) convert HSI back into RGBA
   typedef types::HSIPixel<double> HSIPixelT;
   typedef types::Image<HSIPixelT> HSIImage;

   utility::reportIfNotLessThan("channels",channel,(unsigned)HSIImage::pixel_type::MAX_CHANNELS);

   HSIImage hsiImage(src);

   double normVal = (double)value/SrcImageT::pixel_type::traits::max();

   typename HSIImage::iterator spos = hsiImage.begin();
   typename HSIImage::iterator send = hsiImage.end();

   for(;spos != send;++spos) {
      spos->indexedColor[channel] = normVal;
   }

   tgt = hsiImage.defaultView();
}


template<typename SrcImageT,typename TgtImageT,typename Value>
void histogramModify(const SrcImageT& src, TgtImageT& tgt,Value low,Value high,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotLessThan("cols!=max",low,high);

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   float rescale = (float) TgtImageT::pixel_type::traits::max()/(high - low);

   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray <= low) tpos->namedColor.gray = TgtImageT::pixel_type::traits::min();
      else if(spos->namedColor.gray <= high)
         tpos->namedColor.gray = static_cast<typename TgtImageT::pixel_type::value_type>(rescale * (spos->namedColor.gray - low));
      else tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();
   }
}


template<typename SrcPixelT,typename TgtPixelT,typename Bounds,typename Constraints>
inline void linearlyStretch(const SrcPixelT& src,TgtPixelT& tgt,double rescale,Bounds min, Bounds max,Constraints low,Constraints high) {
   if(src <= low) tgt = min;
   else if(src <= high) tgt = std::min(max,static_cast<Bounds>(rescale * (src - low)));
   else tgt = max;
}


template<typename SrcImageT,typename TgtImageT,typename Value>
void histogramModifyRGB(const SrcImageT& src, TgtImageT& tgt,Value low,Value high,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotLessThan("low<high",low,high);

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   double rescale = ((double) TgtImageT::pixel_type::traits::max() - 
                     (double) TgtImageT::pixel_type::traits::min()) /
                              (high - low);

   typename TgtImageT::pixel_type::value_type min = TgtImageT::pixel_type::traits::min();
   typename TgtImageT::pixel_type::value_type max = TgtImageT::pixel_type::traits::max();

   for(;spos != send;++spos,++tpos) {
      linearlyStretch(spos->namedColor.red,tpos->namedColor.red,rescale,min,max,low,high);
      linearlyStretch(spos->namedColor.green,tpos->namedColor.green,rescale,min,max,low,high);
      linearlyStretch(spos->namedColor.blue,tpos->namedColor.blue,rescale,min,max,low,high);
   }
}


template<typename SrcImageT,typename TgtImageT,typename Value>
void histogramModifyAnyRGB(const SrcImageT& src, TgtImageT& tgt,Value low,Value high,unsigned channel,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotLessThan("channels",channel,(unsigned)SrcImageT::pixel_type::MAX_CHANNELS);
   utility::reportIfNotLessThan("low<high",low,high);

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   double rescale = ((double) TgtImageT::pixel_type::traits::max() - 
                     (double) TgtImageT::pixel_type::traits::min()) /
                              (high - low);

   typename TgtImageT::pixel_type::value_type min = TgtImageT::pixel_type::traits::min();
   typename TgtImageT::pixel_type::value_type max = TgtImageT::pixel_type::traits::max();

   for(;spos != send;++spos,++tpos) {
      linearlyStretch(spos->indexedColor[channel],tpos->indexedColor[channel],rescale,min,max,low,high);
   }
}


template<typename HSIImage,typename Bounds,typename Constraints>
void linearlyStretchChannel(HSIImage& hsiImage,Bounds minBounds,Bounds maxBounds,
                            Constraints low,Constraints high,unsigned channel) {

   utility::reportIfNotLessThan("channels",channel,(unsigned)HSIImage::pixel_type::MAX_CHANNELS);

   double rescale = ((double) maxBounds - 
                     (double) minBounds) /
                        (high - low);

   double lowNorm = (double)low/maxBounds;
   double highNorm = (double)high/maxBounds;

   typename HSIImage::pixel_type::value_type min = HSIImage::pixel_type::traits::min();
   typename HSIImage::pixel_type::value_type max = HSIImage::pixel_type::traits::max();

   typename HSIImage::iterator spos = hsiImage.begin();
   typename HSIImage::iterator send = hsiImage.end();

   for(;spos != send;++spos) {
      linearlyStretch(spos->indexedColor[channel],
                      spos->indexedColor[channel],
                      rescale,min,max,lowNorm,highNorm);
   }
}


template<typename SrcImageT,typename TgtImageT,typename Value>
void histogramModifyIntensity(const SrcImageT& src, TgtImageT& tgt,Value low,Value high,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotLessThan("low<high",low,high);

   // Algorithm
   // Idea here is simple. 
   // 1) Convert image from RGBA to HSI
   // 2) histogram stretch just the intensity channel
   // 3) convert HSI back into RGBA
   typedef types::HSIPixel<double> HSIPixelT;
   typedef types::Image<HSIPixelT> HSIImage;

   HSIImage hsiImage(src);

   linearlyStretchChannel(hsiImage,
                          SrcImageT::pixel_type::traits::min(),
                          SrcImageT::pixel_type::traits::max(),
                          low,high,HSIPixelT::INTENSITY_CHANNEL);

   tgt = hsiImage.defaultView();
}


// Note: with this algorithm, we can stretch each HSI channel separately
template<typename SrcImageT,typename TgtImageT,typename Value>
void histogramModifyHSI(const SrcImageT& src, TgtImageT& tgt,
         Value lowI,Value highI,Value lowS, Value highS,Value lowH, Value highH,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotLessThan("lowI<highI",lowI,highI);
   utility::reportIfNotLessThan("lowS<highS",lowS,highS);
   utility::reportIfNotLessThan("lowH<highH",lowH,highH);

   // Algorithm
   // Idea here is simple. 
   // 1) Convert image from RGBA to HSI
   // 2) histogram stretch just the intensity,saturation and hue channels
   // 3) convert HSI back into RGBA
   typedef types::HSIPixel<double> HSIPixelT;
   typedef types::Image<HSIPixelT> HSIImage;

   HSIImage hsiImage(src);

   linearlyStretchChannel(hsiImage,
                          SrcImageT::pixel_type::traits::min(),
                          SrcImageT::pixel_type::traits::max(),
                          lowI,highI,HSIPixelT::INTENSITY_CHANNEL);
   linearlyStretchChannel(hsiImage,
                          SrcImageT::pixel_type::traits::min(),
                          SrcImageT::pixel_type::traits::max(),
                          lowS,highS,HSIPixelT::SATURATION_CHANNEL);
   linearlyStretchChannel(hsiImage,
                          SrcImageT::pixel_type::traits::min(),
                          SrcImageT::pixel_type::traits::max(),
                          lowH,highH,HSIPixelT::HUE_CHANNEL);

   tgt = hsiImage.defaultView();
}



template<typename SrcImageT,typename TgtImageT>
void add(const SrcImageT& src, TgtImageT& tgt, int value,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {
 
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
              typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

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
void optimalBinarize(const SrcImageT& src, TgtImageT& tgt,
              // This ugly bit is an unnamed argument with a default which means it neither           
              // contributes to the mangled declaration name nor requires an argument. So what is the 
              // point? It still participates in SFINAE to help select that this is an appropriate    
              // matching function given its arguments. Note, SFINAE techniques are incompatible with 
              // deduction so can't be applied to in parameter directly.                              
              typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef typename SrcImageT::pixel_type PixelT;
   typedef typename PixelT::value_type    ValueT;

   // First compute the average as starting threshold
   typedef typename types::AccumulatorVariableSelect<typename SrcImageT::pixel_type>::type AccumulatorT;
   AccumulatorT accum = 0;
   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   unsigned number = 0;
   for(;spos != send;++spos,++number) {
      accum += spos->namedColor.gray;
   }
   
   ValueT threshold = static_cast<ValueT>((double)accum/number);
   ValueT thresholdLast = 0;
   // TODO: come up with a generic way to specify a more intelligent thresholdCondition.
   // E.g. does this work properly for larger bit depths, and also for float pointing types???
   ValueT thresholdCondition = (SrcImageT::pixel_type::traits::max() - SrcImageT::pixel_type::traits::min())/255;
   // Now create loop checking stop condition.
   // Note a ternary expression is used to ensure unsigned math is performed correctly.
   while((thresholdLast > threshold ? thresholdLast - threshold : threshold - thresholdLast) > thresholdCondition) {
      // First save off thresholdLast
      thresholdLast = threshold;
      // Now compute new threshold
      typename SrcImageT::const_iterator sposl = src.begin();
      typename SrcImageT::const_iterator sendl = src.end();
      AccumulatorT accumBG = 0;
      AccumulatorT accumFG = 0;
      unsigned numberBG = 0;
      unsigned numberFG = 0;
      for(;sposl != sendl;++sposl) {
         if(sposl->namedColor.gray < threshold) accumBG += sposl->namedColor.gray,++numberBG;
         else accumFG += sposl->namedColor.gray,++numberFG;
      }
      // Now compute new threshold
      threshold = static_cast<ValueT>(((double)accumFG/numberFG + (double)accumBG/numberBG)/2);
   }

   // Now do the actual binarization based on threshold.
   spos = src.begin();
   typename TgtImageT::iterator tpos = tgt.begin();
   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray < threshold) tpos->namedColor.gray = TgtImageT::pixel_type::traits::min();
      else                                  tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();
   }
}

/*-----------------------------------------------------------------------**/
template<typename SrcImageT,typename TgtImageT>
void otsuBinarize(const SrcImageT& src, TgtImageT& tgt,
              // This ugly bit is an unnamed argument with a default which means it neither           
              // contributes to the mangled declaration name nor requires an argument. So what is the 
              // point? It still participates in SFINAE to help select that this is an appropriate    
              // matching function given its arguments. Note, SFINAE techniques are incompatible with 
              // deduction so can't be applied to in parameter directly.                              
              typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   // First compute histogram:
   //uint32_t maxChannelHeight = 0u;
   double maxChannelHeight = 0u;
   unsigned maxColor = SrcImageT::pixel_type::traits::max();
   HistogramT histogram(computeHistogram(src,maxColor + 1u,SrcImageT::pixel_type::GRAY_CHANNEL,maxChannelHeight));

   // Otsu Algorithm, find max sigma(t) = w1(t)*w2(t)*(u1(t) - u2(t))^2
   // so need to quickly compute u1 and u2, and w1(t) and w2(t)
   // Note: w1 and w2 will be later normalized by size
   unsigned w1 = 0;
   unsigned w2 = 0;
   // Using a "BigAccumulator", because for very large images, say 10000x10000, we could easily
   // exceed the max range of normal Accumulator.
   typedef typename types::BigAccumulatorVariableSelect<typename SrcImageT::pixel_type>::type AccumulatorT;
   // Note: u1/u2 need to be normalized by w1/w2 to compute means
   AccumulatorT u1 = 0;
   AccumulatorT u2 = 0;

   // Initialize accumulator of u2 and w2
   for(unsigned t = 0;t < maxColor;++t) {
      w2 += (unsigned)histogram[t];
      u2 += static_cast<AccumulatorT>(histogram[t]*t);
   }
   // Now compute means using incremental solution (much like a sliding average, except
   // copmuting average of subset by iterating histogram).
   unsigned maxThreshold = 0;
   double maxSigma = 0.0;
   unsigned size = src.size();
   for(unsigned t = 0;t < maxColor;++t) {
      w1 += histogram[t];
      u1 += histogram[t]*t;
      w2 -= histogram[t];
      u2 -= histogram[t]*t;
      // Need to protect the divide by zero pathological case, which is why the ternary expresions.
      double meanDiff = ((double)u1/(w1 > 0 ? w1 : 1) - (double)u2/(w2 > 0 ? w2 : 1));
      double sigma = (double)w1/size*(double)w2/size*meanDiff*meanDiff;
      if(sigma > maxSigma) maxSigma = sigma, maxThreshold = t+1;
   }

   // Now do the actual binarization based on threshold.
   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();
   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray < maxThreshold) tpos->namedColor.gray = TgtImageT::pixel_type::traits::min();
      else                                     tpos->namedColor.gray = TgtImageT::pixel_type::traits::max();
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
                   typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

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
void selectColor(const SrcImageT& src, TgtImageT& tgt, unsigned channel,
                   // This ugly bit is an unnamed argument with a default which means it neither           
                   // contributes to the mangled declaration name nor requires an argument. So what is the 
                   // point? It still participates in SFINAE to help select that this is an appropriate    
                   // matching function given its arguments. Note, SFINAE techniques are incompatible with 
                   // deduction so can't be applied to in parameter directly.                              
                   typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) channel2mono(*spos,*tpos,channel);
}

/*-----------------------------------------------------------------------**/
template<typename SrcImageT,typename TgtImageT>
void selectHSI(const SrcImageT& src, TgtImageT& tgt, unsigned channel,
               // This ugly bit is an unnamed argument with a default which means it neither           
               // contributes to the mangled declaration name nor requires an argument. So what is the 
               // point? It still participates in SFINAE to help select that this is an appropriate    
               // matching function given its arguments. Note, SFINAE techniques are incompatible with 
               // deduction so can't be applied to in parameter directly.                              
               typename std::enable_if<types::is_rgba<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef types::Image<types::HSIPixel<float> > HSIImageT;

   HSIImageT hsiSrc(src.rows(),src.cols());
   hsiSrc = src;

   typename HSIImageT::const_iterator spos = hsiSrc.begin();
   typename HSIImageT::const_iterator send = hsiSrc.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) channel2mono(*spos,*tpos,channel);
}



/*-----------------------------------------------------------------------**/
template<typename SrcImageT,typename TgtImageT>
void binarizeDouble(const SrcImageT& src, TgtImageT& tgt, unsigned thresholdLow,unsigned thresholdHigh,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

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
         typename AccumulatorVariableT = typename types::AccumulatorVariableSelect<PixelT>::type >
class SmoothX {
private:
   typedef types::ElasticImageView<const PixelT>        ConstElasticViewT;
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
         typename AccumulatorVariableT = typename types::AccumulatorVariableSelect<PixelT>::type >
class SmoothY {
private:
   typedef types::ElasticImageView<const PixelT>        ConstElasticViewT;
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
         typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   utility::reportIfNotLessThan("windowSize",2u,windowSize);
   utility::reportIfNotEqual("windowSize (which should be odd)",windowSize-1,((windowSize >> 1u) << 1u));
   utility::reportIfNotEqual("src.rows() != tgt.rows()",src.rows(),tgt.rows());
   utility::reportIfNotEqual("src.cols() != tgt.cols()",src.cols(),tgt.cols());

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
              typename std::enable_if<types::is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

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
      typedef typename types::AccumulatorVariableSelect<pixel_type>::type AccumulatorT;

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

} // namespace algorithm
} // namespace batchIP

