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

#include <cstdint>

#ifdef HHVM_FACEBOOK
#include "common/time/ClockGettimeNS.h" // nolint
#else
/*
 * Read the calling thread's CPU time (CLOCK_THREAD_CPUTIME_ID) in nanoseconds
 * entirely in userspace, via the Linux perf_events interface, avoiding the
 * clock_gettime() syscall on a very hot accounting path.
 *
 * Writes the value to *ns and returns 0 on success, or returns non-zero if the
 * fast path is unavailable (unsupported platform/CPU, or a kernel that does not
 * advertise userspace timekeeping); callers should fall back to clock_gettime()
 * in that case.
 */
int fb_perf_get_thread_cputime_ns(uint64_t* ns);
#endif
