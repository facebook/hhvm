#pragma once

// clang-format off

#if __clang__ || __GNUC__
#  define HAVE_TI_MODE 1
#  define HAVE_INLINE_ASM 1
#endif

#if __x86_64__ || __aarch64__
#  define NATIVE_LITTLE_ENDIAN  1
#endif


#ifndef HAVE_MMINTRIN_H
#  if defined __has_include
#    if __has_include(<mmintrin.h>)
#      define HAVE_MMINTRIN_H 1
#    endif
#  endif
#endif


#ifndef HAVE_EMMINTRIN_H
#  if defined __has_include
#    if __has_include(<emmintrin.h>)
#      define HAVE_EMMINTRIN_H 1
#    endif
#  endif
#endif


#ifndef HAVE_PMMINTRIN_H
#  if defined __has_include
#    if __has_include(<pmmintrin.h>)
#      define HAVE_PMMINTRIN_H 1
#    endif
#  endif
#endif


#ifndef HAVE_TMMINTRIN_H
#  if defined __has_include
#    if __has_include(<tmmintrin.h>)
#      define HAVE_TMMINTRIN_H 1
#    endif
#  endif
#endif


#ifndef HAVE_SMMINTRIN_H
#  if defined __has_include
#    if __has_include(<smmintrin.h>)
#      define HAVE_SMMINTRIN_H 1
#    endif
#  endif
#endif


#ifndef HAVE_AVXINTRIN_H
#  if defined __has_include
#    if __has_include(<avxintrin.h>)
#      define HAVE_AVXINTRIN_H 1
#    endif
#  endif
#endif


#ifndef HAVE_WMMINTRIN_H
#  if defined __has_include
#    if __has_include(<wmmintrin.h>)
#      define HAVE_WMMINTRIN_H 1
#    endif
#  endif
#endif


#ifndef HAVE_AVX2INTRIN_H
#  if defined __has_include
#    if __has_include(<avx2intrin.h>)
#      define HAVE_AVX2INTRIN_H 1
#    endif
#  endif
#endif


#ifndef HAVE_ARMCRYPTO
#  if defined __has_include
#    if __has_include(<arm_neon.h>)
#      define HAVE_ARMCRYPTO 1
#    endif
#  endif
#endif


// clang-format on
