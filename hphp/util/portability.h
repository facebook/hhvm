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

#include <folly/Likely.h> // defining LIKELY/UNLIKELY is part of this header
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

#if defined(__x86_64__)

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
// This is a terrible hack that I wish wasn't needed :(
// It is due to MSVC 2015's initial release not allowing
// you to do an offsetof within a constexpr member function.
// It's allowed at the global scope, but that still requires
// the fields to be public.
// Bug Report: https://connect.microsoft.com/VisualStudio/feedback/details/1414341
# define MSVC_NO_CONSTEXPR_MEMBER_OFFSETOF 1
// MSVC2015's initial release has an issue with getting
// function pointers to templated functions if the expected
// result type isn't auto.
// Bug Report: https://connect.microsoft.com/VisualStudio/feedback/details/1464651
# define MSVC_REQUIRE_AUTO_TEMPLATED_OVERLOAD 1

# if _MSC_FULL_VER <= 190022816 // 2015 RC or below
// TODO: Check if these four are present in 2015's
//       full release, which happens July 20th UTC 2015.
#  define MSVC_NO_TEMPLATED_IMPLICIT_CONVERSIONS 1
#  define MSVC_NO_STD_CHRONO_DURATION_DOUBLE_ADD 1
#  define MSVC_REPO_PROXY_CONSTRUCTION_ISSUES 1
// Something doesn't like boost::variant at times,
// because it will cause the compiler to simply crash,
// and not produce any errors to explain it.
#  define MSVC_BOOST_VARIANT_CLOSURE_ISSUES 1

// 2015 RC doesn't support the use of a vector's size in a constexpr context.
#  define MSVC_NO_CONSTEXPR_VECTOR_SIZE 1
// 2015 RC doesn't support initializing a managed ref constexpr.
#  define MSVC_NO_CONSTEXPR_MANAGED_REF_INITIALIZERS 1
# endif

#ifdef MSVC_NO_CONSTEXPR_MANAGED_REF_INITIALIZERS
template<void(*ctor)()>
struct static_constructor
{
  struct constructor { constructor() { ctor(); } };
  static constructor c;
};

template<void(*ctor)()>
typename static_constructor<ctor>::constructor static_constructor<ctor>::c;
#endif

#endif
