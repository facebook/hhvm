/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_PORTABILITY_H_
#define incl_HPHP_PORTABILITY_H_

#include "folly/Likely.h" // defining LIKELY/UNLIKELY is part of this header

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Various macros to make certain things conditional on either
 * compiler or architecture.
 *
 * Currently we don't *really* compile on anything other than gcc or
 * sometimes clang, and there are some parts of the code using
 * __attribute__ stuff directly, but some things go through these
 * macros to make it maybe easier to change later.
 */

//////////////////////////////////////////////////////////////////////

// TODO: does clang define __GNUC__ ?
#ifndef __GNUC__
# define __attribute__(x)
#endif

//////////////////////////////////////////////////////////////////////

#ifdef ATTRIBUTE_UNUSED
# undef ATTRIBUTE_UNUSED
#endif
#ifdef ATTRIBUTE_NORETURN
# undef ATTRIBUTE_NORETURN
#endif
#ifdef ATTRIBUTE_PRINTF
# undef ATTRIBUTE_PRINTF
#endif

#define ATTRIBUTE_UNUSED   __attribute__((__unused__))
#define ATTRIBUTE_NORETURN __attribute__((__noreturn__))

#define ATTRIBUTE_PRINTF(a1, a2) \
  __attribute__((__format__ (__printf__, a1, a2)))

#define ALWAYS_INLINE      inline __attribute__((__always_inline__))
#define NEVER_INLINE       __attribute__((__noinline__))
#define UNUSED             __attribute__((__unused__))
#define FLATTEN            __attribute__((__flatten__))
#define EXTERNALLY_VISIBLE __attribute__((__externally_visible__))

#ifdef DEBUG
# define DEBUG_ONLY /* nop */
#else
# define DEBUG_ONLY UNUSED
#endif

/*
 * We need to keep some unreferenced functions from being removed by
 * the linker. There is no compile time mechanism for doing this, but
 * by putting them in the same section as some other, referenced function
 * in the same file, we can keep them around.
 *
 * So this macro should be used to mark at least one function that is
 * referenced, and other functions that are not referenced in the same
 * file.
 *
 * Note: this may not work properly with LTO. We'll revisit when/if we
 * move to it.
 */
#ifndef __APPLE__
# define KEEP_SECTION \
    __attribute__((__section__(".text.keep")))
#else
# define KEEP_SECTION \
    __attribute__((__section__(".text.keep,")))
#endif

//////////////////////////////////////////////////////////////////////

#if defined(__x86_64__)

# define DECLARE_STACK_POINTER(sp)                \
    void* sp;                                     \
    asm volatile("mov %%rsp, %0" : "=r" (sp) ::)
# if defined(__clang__)
#  define DECLARE_FRAME_POINTER(fp)               \
    ActRec* fp;                                   \
    asm volatile("mov %%rbp, %0" : "=r" (fp) ::)
# else
#  define DECLARE_FRAME_POINTER(fp) register ActRec* fp asm("rbp");
# endif

#elif defined(__AARCH64EL__)

# if defined(__clang__)
#  error Clang implementation not done for ARM
# endif
# define DECLARE_STACK_POINTER(sp) register void*   sp asm("sp");
# define DECLARE_FRAME_POINTER(fp) register ActRec* fp asm("x29");

#else

# error What are the stack and frame pointers called on your architecture?

#endif

//////////////////////////////////////////////////////////////////////

}

#endif
