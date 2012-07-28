/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#ifndef incl_ASSERT_H_
#define incl_ASSERT_H_

#include <cassert>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////

// might hide problems
#undef ASSERT

#ifdef RELEASE
#ifndef ALWAYS_ASSERT
#define ASSERT(x)
#else
#define ASSERT(x) assert(x)
#endif
#else
#define ASSERT(x) assert(x)
#endif

#define IMPLIES(a, b) (!(a) || (b))

// causes a division by zero error at compile time if the assertion fails
// NOTE: use __LINE__, instead of __COUNTER__, for better compatibility
#define CT_CONCAT_HELPER(a, b) a##b
#define CT_CONCAT(a, b) CT_CONCAT_HELPER(a, b)
#define CT_ASSERT(cond) \
  enum { CT_CONCAT(compile_time_assert_, __LINE__) = 1/(!!(cond)) }

#ifndef __GXX_EXPERIMENTAL_CXX0X__
# define static_assert(what, why) CT_ASSERT((what))
#endif

// Usage example: const_assert(hhvm);
#define const_assert(c) do {                                                  \
  if (!(c)) {                                                                 \
    assert(false);                                                            \
  }                                                                           \
} while (0)

#define not_reached() /* gcc-4.5 supports __builtin_unreachable() */ \
  assert(false)

#define NOT_REACHED not_reached

#define not_implemented() do {                   \
  fprintf(stderr, "not implemented: %s:%d %s\n", \
          __FILE__, __LINE__, __FUNCTION__);     \
  assert(false);                                 \
  not_reached();                                 \
} while(0)

#define assert_not_implemented(pred) do {        \
  if (! (pred) ) {                               \
    not_implemented();                           \
  }                                              \
} while(0)

#define ASSERT_NOT_IMPLEMENTED assert_not_implemented
#define NOT_IMPLEMENTED        not_implemented

//////////////////////////////////////////////////////////////////////

#endif
