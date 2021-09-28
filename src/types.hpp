/*
Copyright 2011-2019 Frederic Langlet
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef _types_
#define _types_

#include <cstddef>
#include <stdlib.h>
#include <cstdint>

	#ifdef _MSC_VER
		#if !defined(__x86_64__)
			#define __x86_64__  _M_X64
		#endif
		#if !defined(__i386__)
			#define __i386__  _M_IX86 
		#endif
	#endif

	#ifdef __x86_64__
		#include <emmintrin.h> //SSE2
		#include <intrin.h> //SSE4A, AVX, AVX2
	#endif

   #ifdef _MSC_VER
	  #if _MSC_VER >= 1920 && _MSC_VER <= 1929 
		 #define _MSC_VER_STR 2019
	  #endif
   #endif

	#ifndef _GLIBCXX_USE_NOEXCEPT
	#define _GLIBCXX_USE_NOEXCEPT
	#endif

	#ifndef THROW
	   #if __cplusplus >= 201103L
	      #define THROW
	   #else 
          #if defined(__GNUC__)
		     #define THROW
          #else
	         #define THROW noexcept(false)
          #endif
	   #endif
	#endif

	typedef int8_t int8;
	typedef uint8_t uint8;
	typedef int16_t int16;
	typedef int32_t int32;
	typedef int64_t int64;
	typedef uint16_t uint16;
	typedef uint32_t uint;
	typedef uint32_t uint32;
	typedef uint64_t uint64;

   #if defined(WIN32) || defined(_WIN32) 
      #define PATH_SEPARATOR '\\' 
   #else 
      #define PATH_SEPARATOR '/' 
   #endif

   #if defined(_MSC_VER)
      #define ALIGNED_(x) __declspec(align(x))
   #else
      #if defined(__GNUC__)
         #define ALIGNED_(x) __attribute__ ((aligned(x)))
      #endif
   #endif

	#ifndef STR_TRUE
		#define STR_TRUE "TRUE"
	#endif

	#ifndef STR_FALSE
		#define STR_FALSE "FALSE"
	#endif

#endif
