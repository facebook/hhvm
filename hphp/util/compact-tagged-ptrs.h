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

#include "hphp/util/assertions.h"

#include <cstdint>

/*
 * A combined pointer + tag intended to save space.  The tag must fit in
 * a 16 bit integer anywhere you use this class.
 *
 * For all supported architectures, this class relies on the property that a
 * "canonical form" address must have all the upper bits set to the same value
 * as bit 47.  Further, it relies on the OS using only "lower half" canonical
 * form addresses for userland pointers.  For more, see:
 *
 *     http://en.wikipedia.org/wiki/X86-64#Canonical_form_addresses
 */

namespace HPHP {

//////////////////////////////////////////////////////////////////////

#if !(defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__))
#error CompactTaggedPtr is not supported on your architecture.
#endif

template<class T, class TagType = uint16_t>
struct CompactTaggedPtr {
  using Opaque = uintptr_t;
  static constexpr size_t kMaxTagSize = 16;
  static constexpr size_t kShiftAmount =
    std::numeric_limits<Opaque>::digits - kMaxTagSize;
  static_assert(
    std::numeric_limits<typename std::make_unsigned<TagType>::type>::digits
      <= kMaxTagSize,
    "TagType must fit in 16 bits"
  );

  CompactTaggedPtr() : m_data{makeOpaque(TagType{}, nullptr)} {}
  CompactTaggedPtr(TagType tag, T* ptr) : m_data{makeOpaque(tag, ptr)} {}

  // for save and restore
  explicit CompactTaggedPtr(Opaque v) : m_data(v) {}
  Opaque getOpaque() const { return m_data; }

  void set(TagType tag, T* ptr) {
    m_data = makeOpaque(tag, ptr);
  }

  TagType tag() const { return static_cast<TagType>(m_data >> kShiftAmount); }

  T* ptr() const {
    return reinterpret_cast<T*>(m_data & (-1ull >> kMaxTagSize));
  }

  T* operator->() const {
    return ptr();
  }

  explicit operator bool() const {
    return ptr();
  }

  void swap(CompactTaggedPtr& o) noexcept {
    std::swap(m_data, o.m_data);
  }

  bool operator==(const CompactTaggedPtr& o) const {
    return m_data == o.m_data;
  }
  bool operator!=(const CompactTaggedPtr& o) const {
    return m_data != o.m_data;
  }

private:
  uintptr_t m_data;

  static uintptr_t makeOpaque(TagType ttag, T* ptr) {
    auto const tag = static_cast<uint64_t>(ttag);
    auto const ptr_int = reinterpret_cast<uintptr_t>(ptr);
    assertx(tag <= 0xffffu);
    assertx((ptr_int >> kShiftAmount) == 0);
    return ptr_int | (tag << kShiftAmount);
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * Same thing, named differently for self-documentation when the tag is
 * representing the size of something.
 */
template<class T>
struct CompactSizedPtr {
  static constexpr size_t kMaxSize = std::numeric_limits<uint16_t>::max();

  void set(uint32_t size, T* ptr) {
    assertx(size <= kMaxSize);
    m_data.set(size, ptr);
  }

  uint32_t size() const { return m_data.tag(); }
  const T* ptr()  const { return m_data.ptr(); }
        T* ptr()        { return m_data.ptr(); }

  void swap(CompactSizedPtr& o) noexcept {
    m_data.swap(o.m_data);
  }

private:
  CompactTaggedPtr<T, uint16_t> m_data;
};

//////////////////////////////////////////////////////////////////////

template<typename T, typename TagType>
void swap(CompactTaggedPtr<T, TagType>& p1,
          CompactTaggedPtr<T, TagType>& p2) noexcept {
  p1.swap(p1);
}

template<typename T>
void swap(CompactSizedPtr<T>& p1, CompactSizedPtr<T>& p2) noexcept {
  p1.swap(p2);
}

//////////////////////////////////////////////////////////////////////

}
