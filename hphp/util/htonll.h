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

/*
 * Tries to find a suitable implementation of htonll/ntohll if it doesn't
 * already exist. This could go into portability.h, but seemed specific enough
 * to be worth pulling out.
 */

#include <folly/portability/Sockets.h>
#include <byteswap.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define htolell(x) (x)
# define letohll(x) (x)
# if !defined(htonll) && !defined(ntohll)
#  define htonll(x) bswap_64(x)
#  define ntohll(x) bswap_64(x)
# endif
#else
# define htolell(x) bswap_64(x)
# define letohll(x) bswap_64(x)
# if !defined(htonll) && !defined(ntohll)
#  define htonll(x) (x)
#  define ntohll(x) (x)
# endif
#endif
