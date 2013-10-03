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
#ifndef incl_HPHP_UTIL_BYTE_ORDER_H_
#define incl_HPHP_UTIL_BYTE_ORDER_H_

#include <cstdint>

#include <arpa/inet.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Utilities for dealing with network/host byte order conversions.
 */

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
# define htonq(a) (a)
# define ntohq(a) (a)
#else
# define ntohq(a)                                         \
  (uint64_t)(((uint64_t) (ntohl((uint32_t) ((a) >> 32)))) \
             | (((uint64_t) (ntohl((uint32_t)             \
                ((a) & 0x00000000ffffffff)))) << 32))
# define htonq(a)                                           \
  (uint64_t) (((uint64_t) (htonl((uint32_t) ((a) >> 32))))  \
              | (((uint64_t) (htonl((uint32_t)              \
                 ((a) & 0x00000000ffffffff)))) << 32))
#endif


//////////////////////////////////////////////////////////////////////

}


#endif
