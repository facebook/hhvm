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
#ifndef incl_HPHP_ASSERTIONS_H_
#define incl_HPHP_ASSERTIONS_H_

#include <cassert>
#include <stdio.h>
#include <cstdlib>

//////////////////////////////////////////////////////////////////////

#ifndef DEBUG
#  ifndef ALWAYS_ASSERT
#    define ASSERT(x)
#  else
#    define ASSERT(x) assert(x)
#  endif
#else
#  define ASSERT(x) assert(x)
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

#ifdef __INTEL_COMPILER
#define not_reached()                                                \
  do {                                                               \
    assert(false);                                                   \
  } while (true)
#else
#define not_reached() /* gcc-4.5 supports __builtin_unreachable() */  \
  do {                                                                \
    assert(false);                                                    \
    __builtin_unreachable();                                          \
  } while (true)
#endif

#define NOT_REACHED not_reached

#define not_implemented() do {                   \
  fprintf(stderr, "not implemented: %s:%d %s\n", \
          __FILE__, __LINE__, __FUNCTION__);     \
  not_reached();                                 \
} while(0)

#define assert_not_implemented(pred) do {        \
  if (! (pred) ) {                               \
    not_implemented();                           \
  }                                              \
} while(0)

#define ASSERT_NOT_IMPLEMENTED assert_not_implemented
#define NOT_IMPLEMENTED        not_implemented

/*
 * Assertion-like checks that should remain on even in a production
 * build.
 */

namespace HPHP {

void assert_fail(const char*, const char*,
                 unsigned int, const char*) __attribute__((noreturn));
inline void assert_fail(const char* e,
                        const char* file,
                        unsigned int line,
                        const char* func) {
#ifndef NDEBUG
  __assert_fail(e, file, line, func);
#else
  extern void impl_assert_fail(const char*,
                               const char*,
                               unsigned int,
                               const char*) __attribute__((noreturn));
  impl_assert_fail(e, file, line, func);
#endif
}

}

#define always_assert(e)                                                \
  ((e) ? static_cast<void>(0)                                           \
       : (::HPHP::assert_fail(#e, __FILE__, __LINE__, __PRETTY_FUNCTION__), \
          static_cast<void>(0)))

//////////////////////////////////////////////////////////////////////

#endif
