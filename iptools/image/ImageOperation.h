#pragma once

#include "RegionOfInterest.h"
#include "ImageAction.h"
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// Operation - a simple wrapper class that manages an Action type
//             on a set of ROIs, or by default the entire image 
//             if no ROIs are added to the Operation.
//
template<typename ActionT,
         typename ImageSrc = typename ActionT::src_image_type,
         typename ImageTgt = typename ActionT::tgt_image_type>
class Operation {
private:
   const ActionT* mAction;
   
   typedef std::pair<RegionOfInterest,ParameterPack> ParameterizedRegionT;

   typedef std::vector<ParameterizedRegionT> ParameterizedRegionsT;
   ParameterizedRegionsT mRegions;

   void operateOnRegions(const ImageSrc& src,ImageTgt& tgt) {
      // If no regions defined, assume operation is to operate
      // on entire image
      if(0 == mRegions.size()) {
         // Call default RegionOfInterest (the whole image); algo uses the default parameters.
         mAction->run(src,tgt,view2roi(src.defaultView()));
      }
      // O.w. iterate all of the given regions
      else {
         ParameterizedRegionsT::const_iterator pos = mRegions.begin();
         ParameterizedRegionsT::const_iterator end = mRegions.end();
         for(;pos != end;++pos) {
            // Call run with RegionOfInterest and ParameterPack
            mAction->run(src,tgt,pos->first,pos->second);
         }
      }
   }

public:
   Operation(const ActionT* action) : 
      mAction(action) {}

   ~Operation() {
      delete mAction;
      mAction = 0;
   }

   void addRegionAndParameterPack(const RegionOfInterest& roi,const ParameterPack& parameters) {
      mRegions.push_back(std::make_pair(roi,parameters));
   }


   ImageTgt run(const ImageSrc& src) {
	   ImageTgt tgt(src); 
      operateOnRegions(src,tgt);
      return tgt;
   }

   void run(const ImageSrc& src,ImageTgt& tgt) {
      operateOnRegions(src,tgt);
   }
};



// parseROIs - grabs region of interest data from an input stream
//             and adds it to an abstract Operation type.
template<typename OperationT>
void parseROIsAndParameters(OperationT& op,unsigned numParameters,std::istream& ins) {

   while(!ins.eof()) {
      try {
         std::string regionID = parseWord<std::string>(ins);
         if(regionID.size() == 0) break;
         if(regionID == "ROI:") {
            unsigned row = parseWord<unsigned>(ins);
            unsigned col = parseWord<unsigned>(ins);
            unsigned rows = parseWord<unsigned>(ins);
            unsigned cols = parseWord<unsigned>(ins);
            ParameterPack params;
            for(unsigned i = 0; i < numParameters;++i) params.push_back(parseWord<std::string>(ins));
            op.addRegionAndParameterPack(RegionOfInterest(rows,cols,row,col),params);
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


