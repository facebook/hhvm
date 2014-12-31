/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/hash.h"
#include <string.h>

namespace HPHP {

#ifdef USE_SSECRC
NEVER_INLINE
strhash_t hash_string_i_unsafe(const char *arKey, uint32_t nKeyLength) {
  return crc8_i_unsafe(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_i(const char *arKey, uint32_t nKeyLength, uint64_t mask) {
  return hash_string_i_inline(arKey, nKeyLength, mask);
}

#else
NEVER_INLINE
strhash_t hash_string_i(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i_inline(arKey, nKeyLength);
}

#endif


}
