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

#ifndef incl_HPHP_ASSERT_THROW_H_
#define incl_HPHP_ASSERT_THROW_H_

#include "hphp/util/assertions.h"

/*
 * This file replaces assert and always_assert with assert_throw and
 * always_assert_throw, respectively. You probably don't want to
 * include it from a header.
 */

// This can make debugging in gdb a pain, so make it two levels of
// opt-in for now. Uncomment the next line to enable it for files that
// have included this header.

//#define DO_ASSERT_THROW
#ifdef DO_ASSERT_THROW

#undef assert
#undef assert_log
#undef always_assert
#undef always_assert_log

#ifndef NDEBUG
#define assert(e) assert_throw(e)
#define assert_log(e, l) assert_throw_log(e, l)
#else
#define assert(e)
#define assert_log(e, l)
#endif

#define always_assert(e) always_assert_throw(e)
#define always_assert_log(e, l) always_assert_throw_log(e, l)

#endif

#endif
