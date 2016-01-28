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

#ifndef incl_HPHP_HTONLL_H_
#define incl_HPHP_HTONLL_H_

/*
 * Tries to find a suitable implementation of htonll/ntohll if it doesn't
 * already exist. This could go into portability.h, but seemed specific enough
 * to be worth pulling out.
 */

#include <arpa/inet.h>
#if defined(__FreeBSD__)
# include <sys/endian.h>
#elif defined(__APPLE__)
# include <machine/endian.h>
# include <libkern/OSByteOrder.h>
#elif defined(_MSC_VER)
# include <stdlib.h>
#else
# include <byteswap.h>
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define htolell(x) (x)
# define letohll(x) (x)
# if !defined(htonll) && !defined(ntohll)
#  if defined(__FreeBSD__)
#   define htonll(x) bswap64(x)
#   define ntohll(x) bswap64(x)
#  elif defined(__APPLE__)
#   define htonll(x) OSSwapInt64(x)
#   define ntohll(x) OSSwapInt64(x)
#  elif defined(_MSC_VER)
#   define htonll(x) _byteswap_uint64(x)
#   define ntohll(x) _byteswap_uint64(x)
#  else
#   define htonll(x) bswap_64(x)
#   define ntohll(x) bswap_64(x)
#  endif
# endif
#else
# if defined(__FreeBSD__)
#  define htolell(x) bswap64(x)
#  define letohll(x) bswap64(x)
# elif defined(__APPLE__)
#  define htolell(x) OSSwapInt64(x)
#  define letohll(x) OSSwapInt64(x)
# elif defined(_MSC_VER)
#  define htolell(x) _byteswap_uint64(x)
#  define letohll(x) _byteswap_uint64(x)
# else
#  define htolell(x) bswap_64(x)
#  define letohll(x) bswap_64(x)
# endif
# if !defined(htonll) && !defined(ntohll)
#  define htonll(x) (x)
#  define ntohll(x) (x)
# endif
#endif

#endif
