#ifndef MY_COMPILER_INCLUDED
#define MY_COMPILER_INCLUDED

/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file include/my_compiler.h
  Header for compiler-dependent features.

  Intended to contain a set of reusable wrappers for preprocessor
  macros, attributes, pragmas, and any other features that are
  specific to a target compiler.
*/

#ifndef MYSQL_ABI_CHECK
#include <assert.h>
#include <stddef.h> /* size_t */
#endif

#include "my_config.h"

/*
  The macros below are borrowed from include/linux/compiler.h in the
  Linux kernel. Use them to indicate the likelyhood of the truthfulness
  of a condition. This serves two purposes - newer versions of gcc will be
  able to optimize for branch predication, which could yield siginficant
  performance gains in frequently executed sections of the code, and the
  other reason to use them is for documentation
*/
#ifdef HAVE_BUILTIN_EXPECT

// likely/unlikely are likely to clash with other symbols, do not #define
#if defined(__cplusplus)
inline bool likely(bool expr) { return __builtin_expect(expr, true); }
inline bool unlikely(bool expr) { return __builtin_expect(expr, false); }
#else
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#endif

#else /* HAVE_BUILTIN_EXPECT */

#if defined(__cplusplus)
inline bool likely(bool expr) { return expr; }
inline bool unlikely(bool expr) { return expr; }
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#endif /* HAVE_BUILTIN_EXPECT */

/* Comunicate to the compiler the unreachability of the code. */
#ifdef HAVE_BUILTIN_UNREACHABLE
#define MY_ASSERT_UNREACHABLE() __builtin_unreachable()
#else
#define MY_ASSERT_UNREACHABLE() \
  do {                          \
    assert(0);                  \
  } while (0)
#endif

/* Visual Studio requires '__inline' for C code */
#if !defined(__cplusplus) && defined(_MSC_VER)
#define inline __inline
#endif

/* Provide __func__ macro definition for Visual Studio. */
#if defined(_MSC_VER)
#define __func__ __FUNCTION__
#endif

/*
  Disable MY_ATTRIBUTE for Sun Studio and Visual Studio.
  Note that Sun Studio supports some __attribute__ variants,
  but not format or unused which we use quite a lot.
*/
#ifndef MY_ATTRIBUTE
#if defined(__GNUC__) || defined(__clang__)
#define MY_ATTRIBUTE(A) __attribute__(A)
#else
#define MY_ATTRIBUTE(A)
#endif
#endif

#if defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#endif

#if defined(_MSC_VER)
#define NO_INLINE __declspec(noinline)
#else
#define NO_INLINE __attribute__((noinline))
#endif

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef SUPPRESS_UBSAN
// clang -fsanitize=undefined
#if defined(HAVE_UBSAN) && defined(__clang__)
#define SUPPRESS_UBSAN MY_ATTRIBUTE((no_sanitize("undefined")))
// gcc -fsanitize=undefined
#elif defined(HAVE_UBSAN) && __has_attribute(no_sanitize_undefined)
#define SUPPRESS_UBSAN MY_ATTRIBUTE((no_sanitize_undefined))
#else
#define SUPPRESS_UBSAN
#endif
#endif /* SUPPRESS_UBSAN */

#ifndef SUPPRESS_TSAN
#if defined(HAVE_TSAN) && defined(__clang__)
#define SUPPRESS_TSAN MY_ATTRIBUTE((no_sanitize("thread")))
#elif defined(HAVE_TSAN) && __has_attribute(no_sanitize_thread)
#define SUPPRESS_TSAN MY_ATTRIBUTE((no_sanitize_thread))
#else
#define SUPPRESS_TSAN
#endif
#endif /* SUPPRESS_TSAN */

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

#endif /* MY_COMPILER_INCLUDED */
