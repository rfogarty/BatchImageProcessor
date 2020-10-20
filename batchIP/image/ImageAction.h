#pragma once

#include <vector>
#include <string>

namespace batchIP {

namespace types {
   struct RegionOfInterest;

   // Used to push ROI function parameters to run callbacks.
   typedef std::vector<std::string> ParameterPack;
}

namespace operation {

///////////////////////////////////////////////////////////////////////////////
// ActionType is a simple enumeration that can operate as a label for
// debugging purposes.
//
enum ActionType {
   UNKNOWN_ACTION,
   INTENSITY,
   BINARIZE,
   BINARIZE_DT,
   CROP,
   SCALE,
   HISTOGRAM,
   HISTOGRAM_MOD,
   SELECT_COLOR,
   SELECT_HSI,
   UNIFORM_SMOOTH,
   EDGE // TODO: possibly support family of edge algorithms
};

///////////////////////////////////////////////////////////////////////////////
// Action - an abstract process or algorithm to run on some Image type.
//
// This class is an abstract base class (ABC) so is composed only of
// pure virtual functions and a virtual destructor. 
//
template<typename ImageSrc,typename ImageTgt=ImageSrc>
class Action {
public:
   typedef ImageSrc src_image_type;
   typedef ImageTgt tgt_image_type;

   virtual ~Action() {}

   virtual ActionType type() const = 0;

   virtual unsigned numParameters() const = 0;

   virtual void run(const ImageSrc& src,ImageTgt& tgt) const = 0;
   
   virtual void run(const ImageSrc& src,ImageTgt& tgt,const types::RegionOfInterest& roi,
                    const types::ParameterPack& parameters) const = 0;
};

} // namespace operation
} // namespace batchIP

