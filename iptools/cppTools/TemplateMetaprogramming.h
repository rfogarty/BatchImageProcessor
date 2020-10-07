#pragma once

#include "Platform.h"

#if __cplusplus >= 201103L

#   include <type_traits>


#define CONST_VAL_FUNC constexpr const

#else

namespace std {

   template<bool B, class T = void>
   struct enable_if {};
    
   template<class T>
   struct enable_if<true, T> { typedef T type; };
   
   
   template<class T, T v>
   struct integral_constant {
       static const T value = v;
       typedef T value_type;
       typedef integral_constant<T,v> type;
       operator value_type() const { return value; }
       const value_type operator()() const { return value; }
   };
   
   typedef integral_constant<bool,true> true_type;
   typedef integral_constant<bool,false> false_type;


   // Depending on the compiler/platform and particular build flags the following could conflict:
   //
   //    char vs. int8_t
   //    unsigned char vs. uint8_t 
   //    int vs. int32_t
   //    long vs. int32_t
   //    long vs. int64_t
   //    const char vs. const int8_t
   //    const unsigned char vs. const uint8_t
   //    const int vs. const int32_t
   //    const long vs. const int32_t
   //    const long vs. const int64_t
   //
   // To avoid any issues and simultaneously make things simple, 
   // we will only support the C99 types.
   //
   template<class T> struct is_integral              : public false_type {};
   template<> struct is_integral<int8_t>             : public true_type {};
   template<> struct is_integral<int16_t>            : public true_type {};
   template<> struct is_integral<int32_t>            : public true_type {};
   template<> struct is_integral<int64_t>            : public true_type {};
   template<> struct is_integral<uint8_t>            : public true_type {};
   template<> struct is_integral<uint16_t>           : public true_type {};
   template<> struct is_integral<uint32_t>           : public true_type {};
   template<> struct is_integral<uint64_t>           : public true_type {};
   template<> struct is_integral<const int8_t>       : public true_type {};
   template<> struct is_integral<const int16_t>      : public true_type {};
   template<> struct is_integral<const int32_t>      : public true_type {};
   template<> struct is_integral<const int64_t>      : public true_type {};
   template<> struct is_integral<const uint8_t>      : public true_type {};
   template<> struct is_integral<const uint16_t>     : public true_type {};
   template<> struct is_integral<const uint32_t>     : public true_type {};
   template<> struct is_integral<const uint64_t>     : public true_type {};

   template<class T> struct is_floating_point        : public false_type {};
   template<> struct is_floating_point<float>        : public true_type {};
   template<> struct is_floating_point<double>       : public true_type {};
   template<> struct is_floating_point<const float>  : public true_type {};
   template<> struct is_floating_point<const double> : public true_type {};
   // not supporting quad floats (aka long double)

   // Reference and CV qualifier removers
   template< class T > struct remove_reference            { typedef T type; };
   template< class T > struct remove_reference<T&>        { typedef T type; };
   template< class T > struct remove_const                { typedef T type; };
   template< class T > struct remove_const<const T>       { typedef T type; };
   template< class T > struct remove_volatile             { typedef T type; };
   template< class T > struct remove_volatile<volatile T> { typedef T type; };
   template< class T > struct remove_cv                   { typedef typename remove_const<typename remove_volatile<T>::type >::type type; };
 
}

#define CONST_VAL_FUNC inline const

#endif

template<class T> struct is_uint8                    : public std::false_type {};
template<> struct is_uint8<uint8_t>                  : public std::true_type {};

