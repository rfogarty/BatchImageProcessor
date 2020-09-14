#pragma once

#include "RegionOfInterest.h"
#include "ImageAction.h"

///////////////////////////////////////////////////////////////////////////////
// Operation - a simple wrapper class that manages an Action type
//             on a set of ROIs, or by default the entire image 
//             if no ROIs are added to the Operation.
//
template<typename ActionT,typename ImageT = typename ActionT::image_type>
class Operation {
private:
   ActionT* mAction;
   typedef std::vector<RegionOfInterest> RegionsT;
   RegionsT mRegions;

   void operateOnRegions(const ImageT& src,ImageT& tgt) {
      // If no regions defined, assume operation is to operate
      // on entire image
      if(0 == mRegions.size()) {
         mAction->run(src,tgt,view2roi(src.defaultView()));
      }
      // O.w. iterate all of the given regions
      else {
         RegionsT::const_iterator pos = mRegions.begin();
         RegionsT::const_iterator end = mRegions.end();
         for(;pos != end;++pos) {
            mAction->run(src,tgt,*pos);
         }
      }
   }

public:
   Operation(ActionT* action) : 
      mAction(action) {}

   ~Operation() {
      delete mAction;
      mAction = 0;
   }

   void addRegion(const RegionOfInterest& roi) {
      mRegions.push_back(roi);
   }

   ImageT run(const ImageT& src) {
	   ImageT tgt(src); // Copy construct the target image from the source
      operateOnRegions(src,tgt);
      return tgt;
   }
};



