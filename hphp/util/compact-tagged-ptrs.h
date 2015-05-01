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
#ifndef incl_HPHP_UTIL_COMPACT_TAGGED_PTRS_H_
#define incl_HPHP_UTIL_COMPACT_TAGGED_PTRS_H_

#include <cstdint>

/*
 * A combined pointer + tag intended to save space on x64.  The tag must fit in
 * a 16 bit integer anywhere you use this class (although the non-x64
 * implementation uses larger space for the tag).
 *
 * For the x64 version, this class relies on the x64 property that a
 * "canonical form" address must have all the upper bits set to the same
 * value as bit 47.  Further, it relies on the OS using only "lower
 * half" canonical form addresses for userland pointers.  For more,
 * see:
 *
 *     http://en.wikipedia.org/wiki/X86-64#Canonical_form_addresses
 *
 * The portable version just uses a pointer plus an actual TagType for the tag.
 */

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// The generic version
template<class T, class TagType = uint32_t, size_t size = sizeof(TagType)>
struct CompactTaggedPtr;
#if 0
// The generic version of the taged pointer
// Disabled for now as it is not needed for ARM64 or x86_64
// Might be needed for ARM64 in the future.
template<class T, class TagType, size_t size>
struct CompactTaggedPtr;
 {
  using Opaque = std::pair<T*,TagType>;

  CompactTaggedPtr() { set(TagType{}, 0); }

  // for save and restore
  explicit CompactTaggedPtr(Opaque v) : m_ptr(v.first), m_tag(v.second) {}
  Opaque getOpaque() const { return std::make_pair(m_ptr, m_tag); }

  void set(TagType ttag, T* ptr) {
    auto const tag = static_cast<uint32_t>(ttag);
    assert(tag <= 0xffffu);
    m_tag = ttag;
    m_ptr = ptr;
  }

  TagType  tag() const { return m_tag; }
  const T* ptr() const { return m_ptr; }
        T* ptr()       { return m_ptr; }

private:
  T* m_ptr;
  TagType m_tag;
};
#endif

#ifdef __x86_64__
// x86_64 has 48bit Virtual address space so we can support uint16_t and uint8_t as specializations

template<class T>
struct CompactTaggedPtr<T, uint16_t, 2> {
  using Opaque = uintptr_t;
  using TagType = uint16_t;
  CompactTaggedPtr() { set(TagType{}, 0); }

  // for save and restore
  explicit CompactTaggedPtr(Opaque v) : m_data(v) {}
  Opaque getOpaque() const { return m_data; }

  void set(TagType ttag, T* ptr) {
    auto const tag = static_cast<uint64_t>(ttag);
    assert(tag <= 0xffffu);
    assert(!(uintptr_t(ptr) >> 48));
    m_data = uintptr_t(ptr) | (size_t(tag) << 48);
  }

  TagType tag() const { return static_cast<TagType>(m_data >> 48); }

  const T* ptr() const {
    return const_cast<CompactTaggedPtr*>(this)->ptr();
  }
  T* ptr() {
    return reinterpret_cast<T*>(m_data & (-1ull >> 16));
  }

private:
  uintptr_t m_data;
};

template<class T>
struct CompactTaggedPtr<T, uint8_t, 1> {
  using Opaque = uintptr_t;
  using TagType = uint8_t;
  CompactTaggedPtr() { set(TagType{}, 0); }

  // for save and restore
  explicit CompactTaggedPtr(Opaque v) : m_data(v) {}
  Opaque getOpaque() const { return m_data; }

  void set(TagType ttag, T* ptr) {
    auto const tag = static_cast<uint64_t>(ttag);
    assert(tag <= 0xffffu);
    assert(!(uintptr_t(ptr) >> 48));
    m_data = uintptr_t(ptr) | (size_t(tag) << 48);
  }

  TagType tag() const { return static_cast<TagType>(m_data >> 48); }

  const T* ptr() const {
    return const_cast<CompactTaggedPtr*>(this)->ptr();
  }
  T* ptr() {
    return reinterpret_cast<T*>(m_data & (-1ull >> 16));
  }

private:
  uintptr_t m_data;
};

#endif

#ifdef __aarch64__

// ARMv8 supports real tag bits in [63:56]
// From B2.1 of ARMv8 spec:
// The 56-bit address range A[55:0], where tagged addressing is used.
template<class T, class TagType>
struct CompactTaggedPtr<T, TagType, 1> {
  using Opaque = uintptr_t;
  CompactTaggedPtr() { set(TagType{}, 0); }

  // for save and restore
  explicit CompactTaggedPtr(Opaque v) : m_data(v) {}
  Opaque getOpaque() const { return m_data; }

  void set(TagType ttag, T* ptr) {
    auto const tag = static_cast<uint64_t>(ttag);
    assert(tag <= 0xffu);
    assert(!(uintptr_t(ptr) >> (64-8)));
    m_data = uintptr_t(ptr) | (size_t(tag) << (64-8));
  }

  TagType tag() const { return static_cast<TagType>(m_data >> 48); }

  const T* ptr() const {
    // The tagged bits are ignored by hardware
    return reinterpret_cast<const T*>(m_data);
  }
  T* ptr() {
    // The tagged bits are ignored by hardware
    return reinterpret_cast<T*>(m_data);
  }

private:
  uintptr_t m_data;
};

// The current ARMv8 only supports VA up to 48bits so we can also support 16bit tags too.

template<class T, class TagType>
struct CompactTaggedPtr<T, TagType, 2> {
  using Opaque = uintptr_t;
  CompactTaggedPtr() { set(TagType{}, 0); }

  // for save and restore
  explicit CompactTaggedPtr(Opaque v) : m_data(v) {}
  Opaque getOpaque() const { return m_data; }

  void set(TagType ttag, T* ptr) {
    auto const tag = static_cast<uint64_t>(ttag);
    assert(tag <= 0xffffu);
    assert(!(uintptr_t(ptr) >> 48));
    m_data = uintptr_t(ptr) | (size_t(tag) << 48);
  }

  TagType tag() const { return static_cast<TagType>(m_data >> 48); }

  const T* ptr() const {
    return const_cast<CompactTaggedPtr*>(this)->ptr();
  }
  T* ptr() {
    return reinterpret_cast<T*>(m_data & (-1ull >> 16));
  }

private:
  uintptr_t m_data;
};

#endif

//////////////////////////////////////////////////////////////////////

/*
 * Same thing, named differently for self-documentation when the tag is
 * representing the size of something.
 * Supports only up to 16bits.
 */
template<class T>
struct CompactSizedPtr {
  void set(uint32_t size, T* ptr)  {
    assert(size <= 0xffffu);
    m_data.set(size, ptr);
  }

  uint32_t size() const { return m_data.tag(); }
  const T* ptr()  const { return m_data.ptr(); }
        T* ptr()        { return m_data.ptr(); }

private:
  CompactTaggedPtr<T, uint16_t> m_data;
};

//////////////////////////////////////////////////////////////////////

}

#endif
