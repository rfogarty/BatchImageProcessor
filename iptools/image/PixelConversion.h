#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "cppTools/NumericConstants.h"
#include <cmath>

template<typename ChannelT,typename ChannelTTraits> struct RGBAPixel;
template<typename ChannelT,typename ChannelTTraits> struct HSIPixel;
template<typename ChannelT,typename ChannelTTraits> struct GrayAlphaPixel;

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
   double r = (double)rgba.namedColor.red/sumRGB;
   double g = (double)rgba.namedColor.green/sumRGB;
   double b = (double)rgba.namedColor.blue/sumRGB;

   double numerator = 0.5 * (2*r - g - b );
   double denominator = std::sqrt((r-g)*(r-g) + (r-b)*(g-b));
   double h = std::acos(numerator/denominator);
   if(b > g) h = twoPi() - h;
   double s = 1.0 - 3.0*std::min(r,std::min(g,b));
   double i = sumRGB/(3*ChannelT1Traits::max());

   hsi.namedColor.hue = static_cast<ChannelT1>(h);
   hsi.namedColor.saturation = static_cast<ChannelT1>(s);
   hsi.namedColor.intensity = static_cast<ChannelT1>(i);
}

template<typename ChannelT,typename ChannelTTraits>
inline void assignRGB(RGBAPixel<ChannelT,ChannelTTraits>& rgba,
               double r, double g,double b) {
   rgba.namedColor.red   = static_cast<ChannelT>(r * ChannelTTraits::max());
   rgba.namedColor.green = static_cast<ChannelT>(g * ChannelTTraits::max());
   rgba.namedColor.blue  = static_cast<ChannelT>(b * ChannelTTraits::max());
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
   double s = hsi.namedColor.saturation;
   double i = hsi.namedColor.intensity;
   
   double h120 = (h < twoThirdsPi()) ? h : ((h < fourThirdsPi()) ? (h - twoThirdsPi()) : (h - fourThirdsPi()) );

   double x = i * (1.0 - s);
   double y = i * (1.0 +  s * std::cos(h120) /
                         std::cos(oneThirdsPi() - h120));
   double z = 3.0*i - (x + y);

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


