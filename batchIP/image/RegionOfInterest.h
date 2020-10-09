#pragma once

#include "image/Image.h"
#include <istream>
#include <iostream>

namespace batchIP {
namespace types {

///////////////////////////////////////////////////////////////////////////////
// RegionOfInterest - simple struct to identify regions of Images on which
//                    to operate.
//
// Due to the simplicity of this class, no encapsulation is warranted.
//
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
typename ImageT::image_view roi2view(ImageT& image,const RegionOfInterest& roi) {
   return image.view(roi.mRows,roi.mCols,roi.mRowBegin,roi.mColBegin);
}

template<typename ImageT>
typename ImageT::const_image_view roi2view(const ImageT& image,const RegionOfInterest& roi) {
   return image.view(roi.mRows,roi.mCols,roi.mRowBegin,roi.mColBegin);
}

template<typename ImageViewT>
RegionOfInterest view2roi(ImageViewT& view) {
  return RegionOfInterest(view.rows(),view.cols(),view.rowBegin(),view.colBegin()); 
}


} // namespace types
} // namespace batchIP

