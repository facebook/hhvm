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

#ifndef incl_HPHP_UTIL_VDSO_H
#define incl_HPHP_UTIL_VDSO_H

#include <folly/portability/Time.h>

namespace HPHP { namespace vdso {
///////////////////////////////////////////////////////////////////////////////

/*
 * Calls __vdso_clock_gettime() if available, otherwise falls back
 * to calling clock_gettime().
 */
int clock_gettime(clockid_t, timespec*);

/*
 * A custom version of clock_gettime() that returns its result in
 * nanoseconds.
 */
int64_t clock_gettime_ns(clockid_t);

////////////////////////////////////////////////////////////////////////////////
}}

#endif
