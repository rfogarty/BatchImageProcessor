#pragma once

#if __cplusplus >= 201103L
   constexpr
#else
   inline
#endif
   const double pi()           { return 3.141592653589793; }

#if __cplusplus >= 201103L
   constexpr
#else
   inline
#endif
   const double twoPi()        { return 6.283185307179586; }

#if __cplusplus >= 201103L
   constexpr
#else
   inline
#endif
   const double oneThirdsPi()  { return 1.0471975511965976; }

#if __cplusplus >= 201103L
   constexpr
#else
   inline
#endif
   const double twoThirdsPi()  { return 2.0943951023931953; }

#if __cplusplus >= 201103L
   constexpr
#else
   inline
#endif
   const double fourThirdsPi() { return 4.1887902047863905; }




