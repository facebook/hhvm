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
#ifndef incl_HPHP_WORD_MEM_H_
#define incl_HPHP_WORD_MEM_H_

#include <limits>
#include <folly/Portability.h>

#include "hphp/util/assertions.h"

extern "C" void* _memcpy8(void* dst, const void* src, size_t len);
extern "C" void* _memcpy16(void* dst, const void* src, size_t len);
extern "C" void _bcopy32(void* dst, const void* src, size_t len);
extern "C" void _bcopy_in_64(void* dst, const void* src, size_t lenIn64);

namespace HPHP {

/*
 * Specialized memcpy implementations that takes advantage of the known
 * properties in length and alignment.
 *
 *  o memcpy8(dst, src, len) is equivalent to
 *        static_cast<char*>(memcpy(dst, src, (len + 7) / 8 * 8)) + len;
 *    it returns a char* pointing to dst[len] instead of dst, in order to
 *    ease its use in string operations.
 *
 *    Note that it could overrun the buffer by up to 7 bytes, depending on len
 *    and alignment of the buffers.  When both src and dst are aligned to 8
 *    bytes, it is safe.  It can also be used in other situations given
 *    sufficient readable space after the buffers.
 *
 *  o memcpy16(dst, src, len) is equivalent to
 *        assert(len > 0 && len % 16 == 0);
 *        memcpy(dst, src, len);
 *
 *  o bcopy32(dst, src, len) is equivalent to
 *         assert(len >= 32);
 *         memcpy(dst, src, len / 32 * 32);
 *    except that it returns void.
 *
 *  o bcopy_in_64(dst, src, lenIn64) is equivalent to
 *         assert(lenIn64 > 0);
 *         memcpy(dst, src, 64 * lenIn64);
 *    except that it returns void.
 */

inline char* memcpy8(void* dst, const void* src, uint32_t len) {
#if defined(__x86_64__) && !defined(__CYGWIN__) && !defined(__MINGW__)
  return reinterpret_cast<char*>(_memcpy8(dst, src, len));
#else
  memcpy(dst, src, len);
  return reinterpret_cast<char*>(dst) + len;
#endif
}

inline char* memcpy16(void* dst, const void* src, uint32_t len) {
  assertx(len > 0 && len % 16 == 0);
#if defined(__x86_64__) && !defined(__CYGWIN__) && !defined(__MINGW__)
  return reinterpret_cast<char*>(_memcpy16(dst, src, len));
#else
  return reinterpret_cast<char*>(memcpy(dst, src, len));
#endif
}

inline void bcopy32(void* dst, const void* src, uint32_t len) {
  assertx(len >= 32);
#if defined(__x86_64__) && !defined(__CYGWIN__) && !defined(__MINGW__)
  _bcopy32(dst, src, len);
#else
  memcpy(dst, src, len / 32 * 32);
#endif
}

inline void bcopy_in_64(void* dst, const void* src, uint32_t lenIn64) {
  assertx(lenIn64 != 0);
#if defined(__x86_64__) && !defined(__CYGWIN__) && !defined(__MINGW__)
  _bcopy_in_64(dst, src, lenIn64);
#else
  memcpy(dst, src, lenIn64 * 64);
#endif
}

// Inline assembly version to avoid a function call.
inline void bcopy32_inline(void* dst, const void* src, uint32_t len) {
  assertx(len >= 32);
#if defined(__x86_64__)
  __asm__ __volatile__("shr    $5, %0\n"
                       ".LBCP32%=:\n"
                       "movdqu (%1), %%xmm0\n"
                       "movdqu 16(%1), %%xmm1\n"
                       "add    $32, %1\n"
                       "movdqu %%xmm0, (%2)\n"
                       "movdqu %%xmm1, 16(%2)\n"
                       "add    $32, %2\n"
                       "dec    %0\n"
                       "jg     .LBCP32%=\n"
                       : "+r"(len), "+r"(src), "+r"(dst)
                       :: "xmm0", "xmm1"
                      );
#else
  bcopy32(dst, src, len);
#endif
}

inline void memcpy16_inline(void* dst, const void* src, uint64_t len) {
  assertx(len >=16 && len % 16 == 0);
#if defined(__x86_64__)
  __asm__ __volatile__("movdqu -16(%1, %0), %%xmm0\n"
                       "movdqu %%xmm0, -16(%2, %0)\n"
                       "shr    $5, %0\n"
                       "jz     .LEND%=\n"
                       ".LR32%=:\n"
                       "movdqu (%1), %%xmm0\n"
                       "movdqu 16(%1), %%xmm1\n"
                       "add    $32, %1\n"
                       "movdqu %%xmm0, (%2)\n"
                       "movdqu %%xmm1, 16(%2)\n"
                       "add    $32, %2\n"
                       "dec    %0\n"
                       "jg     .LR32%=\n"
                       ".LEND%=:\n"
                       : "+r"(len), "+r"(src), "+r"(dst)
                       :: "xmm0", "xmm1"
                      );
#else
  memcpy16(dst, src, len);
#endif
}

//////////////////////////////////////////////////////////////////////

// ASan is less precise than valgrind and believes this function overruns reads
#ifndef FOLLY_SANITIZE_ADDRESS

/*
 * Word at a time comparison for two strings of length `lenBytes'.  Returns
 * true if the regions are the same. This should be invoked only when we know
 * the two strings have the same length. It will not check for the null
 * terminator.
 *
 * Assumes it can load more words than the size to compare (this is often
 * possible in HPHP when you know you dealing with request-allocated memory).
 * The final word compare is adjusted to handle the slack in lenBytes so only
 * the bytes we care about are compared.
 */
ALWAYS_INLINE
bool wordsame(const void* mem1, const void* mem2, uint32_t lenBytes) {
  using T = uint64_t;
  auto constexpr W = sizeof(T);

  assert(reinterpret_cast<const uintptr_t>(mem1) % W == 0);
  // We would like to make sure `mem2' is also aligned.  But `strintern_eq' has
  // always been using `wordsame()' on non-aligned pointers, and this doesn't
  // seem to cause crashes in practice.
  // assert(reinterpret_cast<const uintptr_t>(mem2) % W == 0);

  // Inverse of lenBytes.  Do the negation here to avoid doing it later on the
  // critical path.
  int32_t const nBytes = -lenBytes;
  // Check if `lenBytes' is 0, do the check right here to reuse the flags of
  // the neg instruction.  This saves a test instruction.
  if (UNLIKELY(nBytes == 0)) return true;
  // Do the shift here to avoid doing it later on the critical path.  But we
  // will have to switch to 64 bit here to support very long strings.
  int64_t nBits = static_cast<int64_t>(nBytes) * 8u;

  // Use the base+index addressing mode in x86, so that we only need to
  // increment the base pointer in the loop.
  auto p1 = reinterpret_cast<intptr_t>(mem1);
  auto const diff = reinterpret_cast<intptr_t>(mem2) - p1;

  T data;
  do {
    data = *(reinterpret_cast<const T*>(p1));
    data ^= *(reinterpret_cast<const T*>(p1 + diff));
    p1 += W;
    nBits += W * 8;
    if (nBits >= 0) {
      // As a note for future consideration, we could consider precomputing a
      // 64-bit mask, so that the fraction of the last qword can be checked
      // faster.  But that would require an additional register for the
      // mask.  So it depends on register pressure of the call site.
      return !(data << nBits);
    }
  } while (data == 0);
  return false;
}

#else // FOLLY_SANITIZE_ADDRESS

ALWAYS_INLINE
bool wordsame(const void* mem1, const void* mem2, size_t lenBytes) {
  assert(reinterpret_cast<const uintptr_t>(mem1) % 4 == 0);
  return !memcmp(mem1, mem2, lenBytes);
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
  assert(numWords != 0);
  auto d = (int64_t*)to;
  auto s = (int64_t*)from;
  do {
    *d++ = *s++;
  } while (--numWords);
  return to;
}

/*
 * Like Memset, but operates on words at a time.
 */
template<class T>
T* wordfill(T* ptr, T value, size_t numT) {
  assert(numT < std::numeric_limits<int64_t>::max() &&
         (numT * sizeof(T)) % 8 == 0);
  assert(numT != 0);
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
