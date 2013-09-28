/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_COMPATIBILITY_H_
#define incl_HPHP_COMPATIBILITY_H_

#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define PHP_DIR_SEPARATOR '/'

#if defined(__APPLE__) || defined(__FreeBSD__)
char *strndup(const char* str, size_t len);
int dprintf(int fd, const char *format, ...) ATTRIBUTE_PRINTF(2,3);
typedef int clockid_t;
#endif

int gettime(clockid_t which_clock, struct timespec *tp);
int64_t gettime_diff_us(const timespec &start, const timespec &end);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_COMPATIBILITY_H_
