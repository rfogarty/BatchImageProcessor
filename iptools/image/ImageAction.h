#pragma once

#include "RegionOfInterest.h"

enum ActionType {
   INTENSITY,
   BINARIZE,
   BINARIZE_DT,
   SCALE
};

template<typename ImageT>
class Action {
public:
   typedef ImageT image_type;

   virtual ~Action() {}

   virtual ActionType type() = 0;

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) = 0;
};


