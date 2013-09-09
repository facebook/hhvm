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
#ifndef incl_HPHP_WORD_SAME_H_
#define incl_HPHP_WORD_SAME_H_

#include "hphp/util/util.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Word at a time comparison for two memory regions of length
 * `lenBytes' + 1 (for the null terminator).  Returns true if the
 * regions are the same.
 *
 * Assumes it can load more words than the size to compare (this is
 * often possible in HPHP when you know you are dealing with smart
 * allocated memory).  The final word compare is adjusted to handle
 * the slack in lenBytes so only the bytes we care about are compared.
 */
ALWAYS_INLINE
bool wordsame(const void* mem1, const void* mem2, size_t lenBytes) {
  auto p1 = reinterpret_cast<const uint32_t*>(mem1);
  auto p2 = reinterpret_cast<const uint32_t*>(mem2);
  auto constexpr W = sizeof(*p1);
  for (auto end = p1 + lenBytes / W; p1 < end; p1++, p2++) {
    if (*p1 != *p2) return false;
  }
  // let W = sizeof(*p1); now p1 and p2 point to the last 0..W-1 bytes plus
  // the 0 terminator, ie the last 1..W bytes.  Load W bytes, shift off the
  // bytes after the 0, then compare.
  auto shift = 8 * (W - 1) - 8 * (lenBytes % W);
  return (*p1 << shift) == (*p2 << shift);
}

//////////////////////////////////////////////////////////////////////

}

#endif
