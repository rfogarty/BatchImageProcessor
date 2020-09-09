#ifndef UTILITY_H
#define UTILITY_H

#include "../image/image.h"
#include <cmath>

namespace utility {

#define MAXRGB 255
#define MINRGB 0


inline int checkValue(int value)
{
   if (value > MAXRGB)
      return MAXRGB;
   if (value < MINRGB)
      return MINRGB;
   return value;
}

/*-----------------------------------------------------------------------**/

template<typename SrcImageT,typename TgtImageT>
void add(const SrcImageT &src, TgtImageT &tgt, int value,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {
 
   // TODO: this should probably be moved out of here.
   //tgt.resize(src.rows(), src.cols());

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      *tpos = *spos;
      int gray = static_cast<int>(tpos->namedColor.gray);
      gray += value;
      tpos->namedColor.gray = checkValue(gray);
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
 
   // TODO: this should probably be moved out of here.
   //tgt.resize(src.rows(), src.cols());

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      *tpos = *spos;
      // Update Red
      int signedColor = tpos->namedColor.red;
      signedColor += value;
      tpos->namedColor.red = checkValue(signedColor);
      // Update Blue
      signedColor = tpos->namedColor.blue;
      signedColor += value;
      tpos->namedColor.blue = checkValue(signedColor);
      // Update Green
      signedColor = tpos->namedColor.green;
      signedColor += value;
      tpos->namedColor.green = checkValue(signedColor);
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

   // TODO: this should probably be moved out of here.
   //tgt.resize(src.rows(), src.cols());

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray < threshold) tpos->namedColor.gray = MINRGB;
      else                                  tpos->namedColor.gray = MAXRGB;

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

   // TODO: this should probably be moved out of here.
   //tgt.resize(src.rows(), src.cols());

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
         tpos->namedColor.red = MAXRGB;
         tpos->namedColor.green = MAXRGB;
         tpos->namedColor.blue = MAXRGB;
      }
      else {
         // Anything above the threshold is red
         tpos->namedColor.red = MAXRGB;
         tpos->namedColor.green = MINRGB;
         tpos->namedColor.blue = MINRGB;
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

   // TODO: this should probably be moved out of here.
   //tgt.resize(src.rows(), src.cols());

   typename SrcImageT::const_iterator spos = src.begin();
   typename SrcImageT::const_iterator send = src.end();
   typename TgtImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray < thresholdLow || spos->namedColor.gray >= thresholdHigh)
         tpos->namedColor.gray = MINRGB;
      else
         tpos->namedColor.gray = MAXRGB;

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

} // namespace utility

#endif

