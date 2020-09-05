#ifndef UTILITY_H
#define UTILITY_H

#include "../image/image.h"

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
template<typename ImageT>
void addGrey(const ImageT &src, ImageT &tgt, int value)
{
   tgt.resize(src.rows(), src.cols());

   typename ImageT::const_iterator spos = src.begin();
   typename ImageT::const_iterator send = src.end();
   typename ImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      *tpos = *spos;
      int gray = tpos->namedColor.gray;
      gray += value;
      tpos->namedColor.gray = checkValue(gray);
   }
}

/*-----------------------------------------------------------------------**/
template<typename ImageT>
void binarize(const ImageT &src, ImageT &tgt, unsigned threshold)
{
   tgt.resize(src.rows(), src.cols());

   typename ImageT::const_iterator spos = src.begin();
   typename ImageT::const_iterator send = src.end();
   typename ImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray < threshold) tpos->namedColor.gray = MINRGB;
      else                                  tpos->namedColor.gray = MAXRGB;

   }
}

/*-----------------------------------------------------------------------**/
template<typename ImageT>
void binarizeDouble(const ImageT &src, ImageT &tgt, unsigned thresholdLow,unsigned thresholdHigh)
{
   tgt.resize(src.rows(), src.cols());

   typename ImageT::const_iterator spos = src.begin();
   typename ImageT::const_iterator send = src.end();
   typename ImageT::iterator tpos = tgt.begin();

   for(;spos != send;++spos,++tpos) {
      if(spos->namedColor.gray < thresholdLow ||
         spos->namedColor.gray >= thresholdHigh)
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

