#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "cppTools/NumericConstants.h"
#include "Channel.h"
#include <cmath>
#include <iostream>
#include <ostream>

template<typename ChannelT,typename ChannelTTraits> struct RGBAPixel;
template<typename ChannelT,typename ChannelTTraits> struct HSIPixel;
template<typename ChannelT,typename ChannelTTraits> struct GrayAlphaPixel;
template<typename ChannelT,typename ChannelTTraits> struct MonochromePixel;

template<typename ChannelT1,typename ChannelT1Traits,
         typename ChannelT2,typename ChannelT2Traits>
void rgba2hsi(const RGBAPixel<ChannelT1,ChannelT1Traits>& rgba,
              HSIPixel<ChannelT2,ChannelT2Traits>& hsi,
              // This ugly bit is an unnamed argument with a default which means it neither           
              // contributes to the mangled declaration name nor requires an argument. So what is the 
              // point? It still participates in SFINAE to help select that this is an appropriate    
              // matching function given its arguments. Note, SFINAE techniques are incompatible with 
              // deduction so can't be applied to in parameter directly.                              
              typename std::enable_if<std::is_integral<ChannelT1>::value,int>::type* = 0,
              typename std::enable_if<std::is_floating_point<ChannelT2>::value,int>::type* = 0) {

   // This is the expected conversion, since HSI will usually be normalized floating point

   double sumRGB = (double)rgba.namedColor.red 
                 + (double)rgba.namedColor.green 
                 + (double)rgba.namedColor.blue;
   double r = sumRGB != 0 ? (double)rgba.namedColor.red/sumRGB : 0.0;
   double g = sumRGB != 0 ? (double)rgba.namedColor.green/sumRGB : 0.0;
   double b = sumRGB != 0 ? (double)rgba.namedColor.blue/sumRGB : 0.0;

   double numerator = 0.5 * ((r - g) + (r - b));
   double denominator = std::sqrt((r-g)*(r-g) + (r-b)*(g-b));
   // Kludge: because of numeric instability, in rare circumstances,
   // numerator > denominator, which results in a ratio > 1.0, but the
   // ratio *needs* to be <= 1.0 o.w. acos is undefined (and results in
   // nan. This isn't just hypothetical, experiments showed a few rare
   // cases where this happens. Easiest solution? 
   // if numerator > denominator set them equal!!
   if(numerator > denominator) numerator = denominator;

   double h = denominator != 0 ? std::acos(numerator/denominator) : 0.0;
   if(b > g) h = twoPi() - h;

   // Because rgb are normalized by their sum
   double s = 1.0 - 3.0*std::min(r,std::min(g,b));
   double i = sumRGB/(3*ChannelT1Traits::max());

   hsi.namedColor.hue = static_cast<ChannelT2>(h);
   hsi.namedColor.intensity = static_cast<ChannelT2>(i);
   // The above s is "unnormalized saturation", we need to now "normalize"
   // s before saving it in HSIPixel. This compensation on saturation ensures
   // its space is valid from (0,1) throughout the intensity and hue space.
   // This compensation needs to be undone right before conversion back to RGB space.
   if((twoThirds() < i) && (i < 1.0)) hsi.namedColor.saturation = s /(2.0/i - 2.0);
   else hsi.namedColor.saturation = s;
}

template<typename ChannelT,typename ChannelTTraits>
inline void assignRGB(RGBAPixel<ChannelT,ChannelTTraits>& rgba,
               double r, double g,double b) {
   // Note: that each of the r,g,b terms are unit normalized
   // TODO: A full-proof conversion should consider deriving the terms so that
   // they are [0.0, 1.0) - i.e. 0 inclusive, 1.0 exclusive. Unfortunately, the
   // underlying models have NOT taken this into account. As a result, we need to scale
   // by a small amount under the ChannelT range (max) - thus explains the odd addition
   // of a number close to 1 below.
   // TODO TODO: this looks really broken, especially when Channel is not integral type
   //  believe this works for now but is a bug ready to happen.
   rgba.namedColor.red   = static_cast<ChannelT>(r * (ChannelTTraits::max()+0.999));
   rgba.namedColor.green = static_cast<ChannelT>(g * (ChannelTTraits::max()+0.999));
   rgba.namedColor.blue  = static_cast<ChannelT>(b * (ChannelTTraits::max()+0.999));
}

template<typename ChannelT1,typename ChannelT1Traits,
         typename ChannelT2,typename ChannelT2Traits>
void hsi2rgba(const HSIPixel<ChannelT1,ChannelT1Traits>& hsi,
              RGBAPixel<ChannelT2,ChannelT2Traits>& rgba,
              // This ugly bit is an unnamed argument with a default which means it neither           
              // contributes to the mangled declaration name nor requires an argument. So what is the 
              // point? It still participates in SFINAE to help select that this is an appropriate    
              // matching function given its arguments. Note, SFINAE techniques are incompatible with 
              // deduction so can't be applied to in parameter directly.                              
              typename std::enable_if<std::is_integral<ChannelT2>::value,int>::type* = 0,
              typename std::enable_if<std::is_floating_point<ChannelT1>::value,int>::type* = 0) {

   // This is the expected conversion, since HSI will usually be normalized floating point

   double h = hsi.namedColor.hue;
   double i = hsi.namedColor.intensity;

   // Before implementing the standard conversion, we need to convert normalized saturation
   // to "unnormalized".
   double s = hsi.namedColor.saturation;
   if(twoThirds() < i) s *= (2.0/i - 2.0);
   
   double h120 = (h < twoThirdsPi()) ? h : ((h < fourThirdsPi()) ? (h - twoThirdsPi()) : (h - fourThirdsPi()) );

   double x = i * (1.0 - s);
   double invHue = std::cos(h120) /
                   std::cos(oneThirdsPi() - h120);
   double y = i * (1.0 +  s * invHue);
   
   // Due to numeric imprecision, in some cases, factor y
   // can exceed 1.0. If we do not compensate for that the color
   // space can wrap around causing extreme artifacts during HSI
   // to RGB conversion. Kludge compensation is to recompute a
   // saturation that satisfies a bounded y, and then also recompute
   // x given the new saturation.
   if(y > 1.0) {
      // Solve for s that would make y = 1.0
      // so that variables x and z will also be
      // compensated properly.
      y = 1.0;
      s = (y/i - 1.0)/invHue;
      x = i * (1.0 - s);
   }
   double z = std::max(3.0*i - (x + y),0.0);

   if(h < twoThirdsPi()) assignRGB(rgba,y,z,x);
   else if(h < fourThirdsPi()) assignRGB(rgba,x,y,z);
   else assignRGB(rgba,z,x,y);
}


template<typename ChannelT1,typename ChannelT1Traits,
         typename ChannelT2,typename ChannelT2Traits>
void gray2rgba(const GrayAlphaPixel<ChannelT1,ChannelT1Traits>& gray,
               RGBAPixel<ChannelT2,ChannelT2Traits>& rgba) {

   double grayNorm = (double)gray.namedColor.gray / (double)ChannelT1Traits::max();
   ChannelT2 rescaled = static_cast<ChannelT2>(grayNorm * ChannelT2Traits::max());
   rgba.namedColor.red = rescaled;
   rgba.namedColor.green = rescaled;
   rgba.namedColor.blue = rescaled;
}

template<typename ChannelT1,typename ChannelT1Traits,
         typename ChannelT2,typename ChannelT2Traits>
void rgba2gray(const RGBAPixel<ChannelT1,ChannelT1Traits>& rgba,
               GrayAlphaPixel<ChannelT2,ChannelT2Traits>& gray) {

   double grayNorm =
     ((double)rgba.namedColor.red +
      (double)rgba.namedColor.green +
      (double)rgba.namedColor.blue) / (3.0 * ChannelT1Traits::max());
   ChannelT2 rescaled = static_cast<ChannelT2>(grayNorm * ChannelT2Traits::max());
   gray.namedColor.gray = rescaled;
}


template<template<typename,typename> class PixelT,
         typename ChannelT1,typename ChannelT1Traits,
         typename ChannelT2,typename ChannelT2Traits>
void channel2mono(const PixelT<ChannelT1,ChannelT1Traits>& pixel,
                  MonochromePixel<ChannelT2,ChannelT2Traits>& mono,unsigned channel) {

   double chanval =(double)pixel.indexedColor[channel] / ChannelT1Traits::max() * ChannelT2Traits::max();
   mono.namedColor.mono = static_cast<ChannelT2>(chanval);
}


