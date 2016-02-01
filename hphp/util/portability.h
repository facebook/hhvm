/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/Likely.h> // defining LIKELY/UNLIKELY is part of this header
#include <folly/Portability.h>
#include <folly/CPortability.h> // defining FOLLY_DISABLE_ADDRESS_SANITIZER

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
#ifdef ATTRIBUTE_PRINTF_STRING
# undef ATTRIBUTE_PRINTF_STRING
#endif

#define ATTRIBUTE_PRINTF_STRING FOLLY_PRINTF_FORMAT

#ifdef _MSC_VER
#define ATTRIBUTE_NORETURN __declspec(noreturn)
#define ATTRIBUTE_PRINTF(a1, a2)
#ifndef __thread
# define __thread __declspec(thread)
#endif
#define ATTRIBUTE_UNUSED

#define ALWAYS_INLINE __forceinline
#define EXTERNALLY_VISIBLE
#define FLATTEN
#define NEVER_INLINE __declspec(noinline)
#define UNUSED
#else
#define ATTRIBUTE_NORETURN __attribute__((__noreturn__))
#define ATTRIBUTE_PRINTF(a1, a2) \
  __attribute__((__format__ (__printf__, a1, a2)))
#define ATTRIBUTE_UNUSED   __attribute__((__unused__))

#define ALWAYS_INLINE      inline __attribute__((__always_inline__))
#define EXTERNALLY_VISIBLE __attribute__((__externally_visible__))
#define FLATTEN            __attribute__((__flatten__))
#define INLINE_FLATTEN     inline __attribute__((__always_inline__,__flatten__))
#define NEVER_INLINE       __attribute__((__noinline__))
#define UNUSED             __attribute__((__unused__))
#endif

#if defined(__GNUC__)
# define HHVM_ATTRIBUTE_WEAK __attribute__((__weak__))
#elif defined(__clang__)
# define HHVM_ATTRIBUTE_WEAK __attribute__((__weak_import__))
#else
# define HHVM_ATTRIBUTE_WEAK
#endif

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
    __attribute__((__section__(".text,.text.keep")))
#endif

#if defined(__APPLE__)
// OS X has a macro "isset" defined in this header. Force the include so we can
// make sure the macro gets undef'd. (I think this also applies to BSD, but we
// can cross that road when we come to it.)
# include <sys/param.h>
# ifdef isset
#  undef isset
# endif
#endif

//////////////////////////////////////////////////////////////////////
// DECLARE_FRAME_POINTER

#if defined(__x86_64__)

# if defined(__clang__)
#  define DECLARE_FRAME_POINTER(fp)               \
    ActRec* fp;                                   \
    asm volatile("mov %%rbp, %0" : "=r" (fp) ::)
# else
#  define DECLARE_FRAME_POINTER(fp) register ActRec* fp asm("rbp");
# endif

#elif defined(_M_X64)

// TODO: FIXME! Without this implemented properly, the JIT
// will fail "pretty spectacularly".
# define DECLARE_FRAME_POINTER(fp) \
  always_assert(false);            \
  register ActRec* fp = nullptr;

#elif defined(__AARCH64EL__)

# if defined(__clang__)
#  error Clang implementation not done for ARM
# endif
# define DECLARE_FRAME_POINTER(fp) register ActRec* fp asm("x29");

#elif defined(__powerpc64__)

# if defined(__clang__)
#  error Clang implementation not done for PPC64
# endif
# define DECLARE_FRAME_POINTER(fp) register ActRec* fp = (ActRec*) __builtin_frame_address(0);

#else

# error What are the stack and frame pointers called on your architecture?

#endif

//////////////////////////////////////////////////////////////////////
// CALLEE_SAVED_BARRIER

#if defined(__CYGWIN__) || defined(__MINGW__)
  #define CALLEE_SAVED_BARRIER()\
    asm volatile("" : : : "rbx", "rsi", "rdi", "r12", "r13", "r14", "r15");
#elif defined(_MSC_VER)
  // Unfortunately, we have no way to tell MSVC to do this, so we'll
  // probably have to use a pair of assembly stubs to manage this.
  #define CALLEE_SAVED_BARRIER() always_assert(false);
#elif defined (__powerpc64__)
  // PPC64 port under development
  #define CALLEE_SAVED_BARRIER()\
      not_implemented();
#elif defined (__AARCH64EL__)
  #define CALLEE_SAVED_BARRIER()\
    asm volatile("" : : : "x19", "x20", "x21", "x22", "x23", "x24", "x25",\
                 "x26", "x27", "x28")
#else
  #define CALLEE_SAVED_BARRIER()\
    asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15");
#endif

//////////////////////////////////////////////////////////////////////

// We reserve the exit status 127 to signal a failure in the
// interpreter. 127 is a valid exit code on all reasonable
// architectures: POSIX requires at least 8 unsigned bits and
// Windows 32 signed bits.
#define HPHP_EXIT_FAILURE 127

//////////////////////////////////////////////////////////////////////

#if FACEBOOK
// Linking in libbfd is a gigantic PITA. If you want this yourself in a non-FB
// build, feel free to define HAVE_LIBBFD and specify the right options to link
// in libbfd.a in the extra C++ options.
#define HAVE_LIBBFD 1
#endif

#ifndef PACKAGE
// The value doesn't matter, but it must be defined before you include
// bfd.h
#define PACKAGE "hhvm"
#endif

//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
# include "hphp/util/portability/fnmatch.h"
# include "hphp/util/portability/glob.h"
# include "hphp/util/portability/rand_r.h"
# include "hphp/util/portability/strfmon.h"
# include "hphp/util/portability/strptime.h"
#endif

#if defined(_MSC_VER) && _MSC_FULL_VER <= 190023506 // 2015 Update 1 or below
// MSVC2015 has an issue with getting function pointers to templated functions
// if the expected result type isn't auto. Unfortunately, when I made the
// initial bug report, I oversimplified the use-case, and, while the case I
// reported was indeed fixed in Update 1 RC, none of our actual uses of it were
// fixed.
// This is being tracked at MS as #163251.
# define MSVC_REQUIRE_AUTO_TEMPLATED_OVERLOAD 1
// 2015 RTM doesn't like it when you try to add via a double duration.
// Bug Report: https://connect.microsoft.com/VisualStudio/feedback/details/1839243
# define MSVC_NO_STD_CHRONO_DURATION_DOUBLE_ADD 1
#endif

#endif
