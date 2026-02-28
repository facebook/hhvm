/* Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_STACKTRACE_INCLUDED
#define MY_STACKTRACE_INCLUDED

/**
  @file include/my_stacktrace.h
*/

#include <stddef.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "my_compiler.h"
#include "my_config.h"
#include "my_inttypes.h"
#include "my_macros.h"

/*
  HAVE_BACKTRACE - Linux, FreeBSD, OSX, Solaris
  _WIN32 - Windows
*/
#if defined(HAVE_BACKTRACE) || defined(_WIN32)
#define HAVE_STACKTRACE 1
void my_init_stacktrace();
void my_print_stacktrace(const uchar *stack_bottom, ulong thread_stack);
void my_safe_puts_stderr(const char *val, size_t max_len);

#ifdef _WIN32
void my_set_exception_pointers(EXCEPTION_POINTERS *ep);
void my_create_minidump(const char *name, HANDLE process, DWORD pid);
#endif
#endif /* HAVE_BACKTRACE || _WIN32 */

void my_write_core(int sig);

/**
  Async-signal-safe utility functions used by signal handler routines.
  Declared here in order to unit-test them.
  These are not general-purpose, but tailored to the signal handling routines.
*/
/**
  Converts a longlong value to string.
  @param   base 10 for decimal, 16 for hex values (0..9a..f)
  @param   val  The value to convert
  @param   buf  Assumed to point to the *end* of the buffer.
  @returns Pointer to the first character of the converted string.
           Negative values:
           for base-10 the return string will be prepended with '-'
           for base-16 the return string will contain 16 characters
  Implemented with simplicity, and async-signal-safety in mind.
*/
char *my_safe_itoa(int base, longlong val, char *buf);

/**
  Converts a ulonglong value to string.
  @param   base 10 for decimal, 16 for hex values (0..9a..f)
  @param   val  The value to convert
  @param   buf  Assumed to point to the *end* of the buffer.
  @returns Pointer to the first character of the converted string.
  Implemented with simplicity, and async-signal-safety in mind.
*/
char *my_safe_utoa(int base, ulonglong val, char *buf);

/**
  A (very) limited version of snprintf.
  @param   to   Destination buffer.
  @param   n    Size of destination buffer.
  @param   fmt  printf() style format string.
  @returns Number of bytes written, including terminating '\0'
  Supports 'd' 'i' 'u' 'x' 'p' 's' conversion.
  Supports 'l' and 'll' modifiers for integral types.
  Does not support any width/precision.
  Implemented with simplicity, and async-signal-safety in mind.
*/
size_t my_safe_snprintf(char *to, size_t n, const char *fmt, ...)
    MY_ATTRIBUTE((format(printf, 3, 4)));

/**
  A (very) limited version of snprintf, which writes the result to STDERR.
  @sa my_safe_snprintf
  Implemented with simplicity, and async-signal-safety in mind.
  @note Has an internal buffer capacity of 512 bytes,
  which should suffice for our signal handling routines.
*/
size_t my_safe_printf_stderr(const char *fmt, ...)
    MY_ATTRIBUTE((format(printf, 1, 2)));

/**
  Writes up to count bytes from buffer to STDERR.
  Implemented with simplicity, and async-signal-safety in mind.
  @param   buf   Buffer containing data to be written.
  @param   count Number of bytes to write.
  @returns Number of bytes written.
*/
size_t my_write_stderr(const void *buf, size_t count);

/**
  Writes system time to STDERR without allocating new memory.
*/
void my_safe_print_system_time();

#endif  // MY_STACKTRACE_INCLUDED
