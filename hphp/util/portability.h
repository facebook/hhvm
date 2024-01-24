/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#pragma once

#if !defined(NO_FOLLY)
#include <folly/Likely.h> // defining LIKELY/UNLIKELY is part of this header
#include <folly/Portability.h>
#include <folly/CPortability.h> // FOLLY_DISABLE_ADDRESS_SANITIZER, FOLLY_EXPORT
#endif

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
#ifdef ATTRIBUTE_PRINTF
# undef ATTRIBUTE_PRINTF
#endif
#ifdef ATTRIBUTE_PRINTF_STRING
# undef ATTRIBUTE_PRINTF_STRING
#endif

#ifdef FOLLY_PRINTF_FORMAT
#define ATTRIBUTE_PRINTF_STRING FOLLY_PRINTF_FORMAT
#else
#define ATTRIBUTE_PRINTF_STRING
#endif

#define ATTRIBUTE_PRINTF(a1, a2) \
  __attribute__((__format__ (__printf__, a1, a2)))
#define ATTRIBUTE_UNUSED   __attribute__((__unused__))
#define ATTRIBUTE_USED     __attribute__((__used__))
#ifndef NDEBUG
# define FLATTEN           /*nop*/
# define ALWAYS_INLINE     inline
# define INLINE_FLATTEN    inline
#else
# define FLATTEN           __attribute__((__flatten__))
# define ALWAYS_INLINE     inline __attribute__((__always_inline__))
# define INLINE_FLATTEN    inline __attribute__((__always_inline__,__flatten__))
#endif
#define NEVER_INLINE       __attribute__((__noinline__))
#define UNUSED             __attribute__((__unused__))

#ifdef __clang__
#define NO_OPT [[clang::optnone]]
#define EXTERNALLY_VISIBLE ATTRIBUTE_USED FOLLY_EXPORT
#else
#define NO_OPT __attribute__((__optimize__("O0")))
#define EXTERNALLY_VISIBLE __attribute__((__externally_visible__))
#endif

#if defined(__GNUC__)
# define HHVM_ATTRIBUTE_WEAK __attribute__((__weak__))
#elif defined(__clang__)
# define HHVM_ATTRIBUTE_WEAK __attribute__((__weak_import__))
#else
# define HHVM_ATTRIBUTE_WEAK
#endif

#ifndef NDEBUG
# define DEBUG_ONLY /* nop */
#else
# define DEBUG_ONLY UNUSED
#endif

#ifndef NDEBUG
#define DEBUG_NOEXCEPT
#else
#define DEBUG_NOEXCEPT noexcept
#endif


/*
 * AARCH64 needs to create a walkable stack frame for
 * getFrameRegs() when a FixupEntry isIndirect()
 */
#ifdef __aarch64__
#define AARCH64_WALKABLE_FRAME() asm("" ::: "memory");
#else
#define AARCH64_WALKABLE_FRAME()
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
# define KEEP_SECTION \
    __attribute__((__section__(".text.keep")))

//////////////////////////////////////////////////////////////////////
// DECLARE_FRAME_POINTER

#if defined(__x86_64__)

# define DECLARE_FRAME_POINTER(fp) \
  auto const fp = (ActRec*) __builtin_frame_address(0)
# define FRAME_POINTER_IS_ACCURATE

#elif defined(_M_X64)

// TODO: FIXME! Without this implemented properly, the JIT
// will fail "pretty spectacularly".
# define DECLARE_FRAME_POINTER(fp) \
  always_assert(false);            \
  ActRec* fp = nullptr;

#elif defined(__AARCH64EL__)

# if defined(__clang__)
# define DECLARE_FRAME_POINTER(fp) ActRec* fp = (ActRec*) \
  __builtin_frame_address(0)
#else
# define DECLARE_FRAME_POINTER(fp) ActRec* fp asm("x29")
#endif

#else

# error What are the stack and frame pointers called on your architecture?

#endif

//////////////////////////////////////////////////////////////////////
// CALLEE_SAVED_BARRIER

#ifdef __AARCH64EL__
  #define CALLEE_SAVED_BARRIER()\
    asm volatile("" : : : "x19", "x20", "x21", "x22", "x23", "x24", "x25",\
                 "x26", "x27", "x28", \
                 "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15")
#else
  #define CALLEE_SAVED_BARRIER()\
    asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15");
#endif

//////////////////////////////////////////////////////////////////////

// We reserve the exit status 127 to signal a failure. 127 is a valid
// exit code on all reasonable architectures: POSIX requires at least
// 8 unsigned bits and Windows 32 signed bits.
#define HPHP_EXIT_FAILURE 127

// When HHVM server fails to shutdown within a timeout, we forcefully
//exit the process with this exit code, similar to an unclean exit
// upon SIGTERM (signal 15).
#define HPHP_SHUTDOWN_TIMEOUT (128 + 15)

//////////////////////////////////////////////////////////////////////

#ifdef HHVM_FACEBOOK
#define USE_FOLLY_SYMBOLIZER 1
// Linking in libbfd is a gigantic PITA, but if folly symbolizer doesn't
// work on your platform, you'll need to figure it out.
#define HAVE_LIBBFD 1
#endif

#ifndef PACKAGE
// The value doesn't matter, but it must be defined before you include
// bfd.h
#define PACKAGE "hhvm"
#endif

//////////////////////////////////////////////////////////////////////

#define ASM_LOCAL_LABEL(x) ".L" x
