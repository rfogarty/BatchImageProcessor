#pragma once

#include "image/Image.h"
#include <istream>
#include <iostream>


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


// parseROIs - grabs region of interest data from an input stream
//             and adds it to an abstract Operation type.
template<typename OperationT>
void parseROIs(OperationT& op,std::istream& ins) {

   while(!ins.eof()) {
      try {
         std::string regionID = parseWord<std::string>(ins);
         if(regionID.size() == 0) break;
         if(regionID == "ROI:") {
            unsigned row = parseWord<unsigned>(ins);
            unsigned col = parseWord<unsigned>(ins);
            unsigned rows = parseWord<unsigned>(ins);
            unsigned cols = parseWord<unsigned>(ins);
            op.addRegion(RegionOfInterest(rows,cols,row,col));
         }
         else {
            // Log error and break
            std::cerr << "WARNING: expecting literal \"ROI:\" for Region of Interest,\n"
                      << "WARNNG: but parsed: " << regionID << std::endl;
            break;
         }
      }
      catch(const ParseError& pe) {
         std::cerr << "WARNING: trouble decoding Region of Interest.\n"
                   << "WARNING: processing on image may be incomplete." << std::endl;
      }
   }
}


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


