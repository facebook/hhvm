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
#ifndef incl_HPHP_UTIL_COMPACT_SIZED_PTR_H_
#define incl_HPHP_UTIL_COMPACT_SIZED_PTR_H_

/*
 * A combined pointer + size intended to save space on x64.  The
 * maximum size should be 2^16 anywhere you use this class (although
 * the non-x64 implementation uses a larger size type).
 *
 * For the x64 version, this class relies on the x64 property that a
 * "canonical form" address must have all the upper bits set to the same
 * value as bit 47.  Further, it relies on the OS using only "lower
 * half" canonical form addresses for userland pointers.  For more,
 * see:
 *
 *     http://en.wikipedia.org/wiki/X86-64#Canonical_form_addresses
 *
 * The portable version just uses a pointer plus a uint32_t for the
 * size.
 */

namespace HPHP {

//////////////////////////////////////////////////////////////////////

#ifdef __x86_64__

template<class T>
struct CompactSizedPtr {
  CompactSizedPtr() { set(0, 0); }

  void set(uint32_t size, T* ptr) {
    assert(size <= 0xffffu);
    assert(!(uintptr_t(ptr) >> 48));
    m_data = uintptr_t(ptr) | (size_t(size) << 48);
  }

  uint32_t size() const { return m_data >> 48; }

  const T* ptr() const {
    return const_cast<CompactSizedPtr*>(this)->ptr();
  }
  T* ptr() {
    return reinterpret_cast<T*>(m_data & (-1ull >> 16));
  }

private:
  uintptr_t m_data;
};

#else

template<class T>
struct CompactSizedPtr {
  CompactSizedPtr() { set(0, 0); }

  void set(uint32_t size, T* ptr) {
    assert(size <= 0xffffu);
    m_size = size;
    m_ptr = ptr;
  }

  uint32_t size() const { return m_size; }
  const T* ptr()  const { return m_ptr; }
        T* ptr()        { return m_ptr; }

private:
  T* m_ptr;
  uint32_t m_size;
};

#endif

//////////////////////////////////////////////////////////////////////

}

#endif
