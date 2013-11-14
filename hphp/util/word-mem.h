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
#ifndef incl_HPHP_WORD_MEM_H_
#define incl_HPHP_WORD_MEM_H_

#include <limits>
#include "folly/Portability.h"
#include "hphp/util/util.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// ASan is less precise than valgrind and believes this function overruns reads
#ifndef FOLLY_SANITIZE_ADDRESS

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
  assert(reinterpret_cast<const uintptr_t>(mem1) % 4 == 0);
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

#else // FOLLY_SANITIZE_ADDRESS

ALWAYS_INLINE
bool wordsame(const void* mem1, const void* mem2, size_t lenBytes) {
  assert(reinterpret_cast<const uintptr_t>(mem1) % 4 == 0);
  return !memcmp(mem1, mem2, lenBytes+1);
}

#endif

/*
 * Like memcpy, but copies numT POD values 8 bytes at a time.
 * The actual number of bytes copied must be a multiple of 8.
 */
template<class T>
T* wordcpy(T* to, const T* from, size_t numT) {
  assert(numT < std::numeric_limits<int64_t>::max() &&
         (numT * sizeof(T)) % 8 == 0);
  size_t numWords = numT * sizeof(T) / 8;
  auto d = (int64_t*)to;
  auto s = (int64_t*)from;
  do {
    *d++ = *s++;
  } while (--numWords);
  return to;
}

/*
 * like Memset, but operates on words at a time.
 */
template<class T>
T* wordfill(T* ptr, T value, size_t numT) {
  assert(numT < std::numeric_limits<int64_t>::max() &&
         (numT * sizeof(T)) % 8 == 0);
  auto numWords = numT * sizeof(T) / 8;
  auto d = (int64_t*)ptr;
  do {
    *d++ = (int64_t)value;
  } while (--numWords);
  return ptr;
}

//////////////////////////////////////////////////////////////////////

}

#endif
