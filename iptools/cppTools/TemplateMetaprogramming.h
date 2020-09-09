#pragma once


#if __cplusplus >= 201103L

#   include <type_traits>

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

   template< class T > struct remove_reference      {typedef T type;};
   template< class T > struct remove_reference<T&>  {typedef T type;};

   template< class T > struct remove_const                { typedef T type; };
   template< class T > struct remove_const<const T>       { typedef T type; };
 
}


#endif

