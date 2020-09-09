#pragma once

#include "image/Image.h"


struct RegionOfInterest {
   unsigned mRows;
   unsigned mCols;
   unsigned mRowBegin;
   unsigned mColBegin;

   RegionOfInterest(unsigned rows = 0,unsigned cols = 0,
                    unsigned rowBegin = 0,unsigned colBegin = 0) :
      mRows(rows),
      mCols(cols),
      mRowBegin(rowBegin),
      mColBegin(colBegin)
   {}
};

template<typename ImageT>
typename ImageT::ImageViewT roi2view(ImageT& image,const RegionOfInterest& roi) {
   return image.view(roi.mRows,roi.mCols,roi.mRowBegin,roi.mColBegin);
}

template<typename ImageT>
typename ImageT::ConstImageViewT roi2view(const ImageT& image,const RegionOfInterest& roi) {
   return image.view(roi.mRows,roi.mCols,roi.mRowBegin,roi.mColBegin);
}

template<typename ImageViewT>
RegionOfInterest view2roi(ImageViewT& view) {
  return RegionOfInterest(view.rows(),view.cols(),view.rowBegin(),view.colBegin()); 
}


