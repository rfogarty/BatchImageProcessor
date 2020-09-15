#pragma once

#include "RegionOfInterest.h"
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// ActionType is a simple enumeration that can operate as a label for
// debugging purposes.
//
enum ActionType {
   UNKNOWN_ACTION,
   INTENSITY,
   BINARIZE,
   BINARIZE_DT,
   SCALE,
   UNIFORM_SMOOTH
};

// Used to push ROI function parameters to run callbacks.
typedef std::vector<std::string> ParameterPack;

///////////////////////////////////////////////////////////////////////////////
// Action - an abstract process or algorithm to run on some Image type.
//
// This class is an abstract base class (ABC) so is composed only of
// pure virtual functions and a virtual destructor. 
//
template<typename ImageT>
class Action {
public:
   typedef ImageT image_type;

   virtual ~Action() {}

   virtual ActionType type() const = 0;

   virtual unsigned numParameters() const = 0;

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) = 0;
   
   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi,const ParameterPack& parameters) = 0;
};


