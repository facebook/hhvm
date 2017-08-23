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

#ifndef incl_HPHP_HEADER_KIND_H_
#define incl_HPHP_HEADER_KIND_H_

#include "hphp/util/compilation-flags.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace HPHP {

#define COLLECTION_TYPES              \
  COL(Vector)                         \
  COL(Map)                            \
  COL(Set)                            \
  COL(Pair)                           \
  COL(ImmVector)                      \
  COL(ImmMap)                         \
  COL(ImmSet)

/*
 * every request-allocated object has a 1-byte kind field in its
 * header, from this enum. If you update the enum, be sure to
 * update header_names[] as well, and either unset or change
 * HHVM_REPO_SCHEMA, because kind values are used in HHBC.
 */
enum class HeaderKind : uint8_t {
  // ArrayKind aliases
  Packed, Mixed, Empty, Apc, Globals, Proxy,
  // Hack arrays
  Dict, VecArray, Keyset,
  // Other ordinary refcounted heap objects
  String, Resource, Ref,
  Object, WaitHandle, AsyncFuncWH, AwaitAllWH, Closure,
  // Collections
  Vector, Map, Set, Pair, ImmVector, ImmMap, ImmSet,
  // other kinds, not used for countable objects.
  AsyncFuncFrame, // NativeNode followed by Frame, Resumable, AFWH
  NativeData, // a NativeData header preceding an HNI ObjectData
  ClosureHdr, // a ClosureHdr preceding a Closure ObjectData
  SmallMalloc, // small req::malloc'd block
  BigMalloc, // big req::malloc'd block
  BigObj, // big size-tracked object (valid header follows MallocNode)
  Free, // small block in a FreeList
  Hole, // wasted space not in any freelist
  Slab, // header for a contiguous "slab" of small objects
};
const unsigned NumHeaderKinds = unsigned(HeaderKind::Slab) + 1;
extern const std::array<char*,NumHeaderKinds> header_names;

inline bool haveCount(HeaderKind k) {
  return uint8_t(k) < uint8_t(HeaderKind::AsyncFuncFrame);
}

/*
 * RefCount type for m_count field in refcounted objects.
 *
 * The sign bit flags a reference count as persistent. If a reference count is
 * persistent, it means we should never increment or decrement it: the object
 * lives across requests and may be accessed by multiple threads. Persistent
 * objects can be static or uncounted; static objects have process lifetime,
 * while uncounted objects are freed using the treadmill. Using 8-bit values
 * generates shorter cmp instructions while still being far enough from 0 to be
 * safe.
 */
enum RefCount : std::conditional<one_bit_refcount, int8_t, int32_t>::type {
  InitialValue   = one_bit_refcount ? 0 : 1,
  UncountedValue = -128,
  StaticValue    = -127,

  /* Only relevant when one_bit_refcount == true, but always defined so we
   * don't have to #ifdef away all the relevant code. */
  PrivateValue   = 0,
  SharedValue    = 1,
};

using UnsignedRefCount = std::make_unsigned<RefCount>::type;

enum GCBits {
  Unmarked = 0,
  Mark = 1,
  Pin = 3,
};

inline GCBits operator|(GCBits a, GCBits b) {
  return static_cast<GCBits>(
      static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
  );
}
inline bool operator&(GCBits a, GCBits b) {
  return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
}

/*
 * Common header for all heap-allocated objects. Layout is carefully
 * designed to allow overlapping with the second word of a TypedValue,
 * or to follow a C++ defined vptr.
 *
 * T can be any simple 16-bit type. CNT is Maybe for copy-on-write
 * objects that support being allocated outside the request heap with
 * a count field containing StaticValue or UncountedValue
 */
struct HeapObject {
protected:
  union {
    struct {
      mutable RefCount m_count;
#ifdef ONE_BIT_REFCOUNT
      int8_t m_padding[3];
#endif
      HeaderKind m_kind;
      mutable uint8_t m_weak_refed:1;
      mutable uint8_t m_marks:7;
      mutable uint16_t m_aux16;
    };
    struct {
      uint32_t m_aux32; // usable if the subclass is not refcounted
      uint32_t m_hi32;
    };
    uint64_t m_all64;
  };

  // rename aux16
  template<class T> T& aux16() const {
    static_assert(sizeof(T) == 2, "");
    return reinterpret_cast<T&>(m_aux16);
  }

  template<class T> T& aux32() {
    static_assert(sizeof(T) == 4, "");
    return reinterpret_cast<T&>(m_aux32);
  }

public:
  // All of the initHeader functions are carefully crafted to properly
  // zero-extend their arguments and compose them together into one 8-byte
  // store. You must carefully audit each one if you change anything about the
  // layout of HeapObject.

  static_assert(std::is_unsigned<std::underlying_type<HeaderKind>::type>::value,
                "initHeader() functions assume HeaderKind is unsigned");

  void initHeader(HeaderKind kind, RefCount count) {
    m_all64 = uint64_t(kind) << (8 * offsetof(HeapObject, m_kind)) |
              UnsignedRefCount(count);
  }

  void initHeader_32(HeaderKind kind, uint32_t aux32) {
    m_all64 = uint64_t(kind) << (8 * offsetof(HeapObject, m_kind)) |
              aux32;
  }

  void initHeader_16(HeaderKind kind, RefCount count, uint16_t aux16) {
    m_all64 = uint64_t(kind)  << (8 * offsetof(HeapObject, m_kind)) |
              uint64_t(aux16) << (8 * offsetof(HeapObject, m_aux16)) |
              UnsignedRefCount(count);
  }

  void initHeader_32_16(HeaderKind kind, uint32_t aux32, uint16_t aux16) {
    m_all64 = uint64_t(kind)  << (8 * offsetof(HeapObject, m_kind)) |
              uint64_t(aux16) << (8 * offsetof(HeapObject, m_aux16)) |
              aux32;
  }

  void initHeader(const HeapObject& h, RefCount count) {
    m_all64 = uint64_t(h.m_hi32) << (8 * offsetof(HeapObject, m_hi32)) |
              UnsignedRefCount(count);
  }

  static constexpr size_t kind_offset() {
    return offsetof(HeapObject, m_kind);
  }
  static constexpr size_t count_offset() {
    return offsetof(HeapObject, m_count);
  }
  static constexpr size_t aux_offset() {
    return offsetof(HeapObject, m_aux16);
  }

public:
  HeaderKind kind() const { return m_kind; }
  GCBits marks() const { return (GCBits)m_marks; }
  void clearMarks() const { m_marks = GCBits::Unmarked; }
  GCBits mark(GCBits m) const {
    auto const old = (GCBits)m_marks;
    m_marks = old | m;
    return old;
  }
};
static_assert(sizeof(HeapObject) == sizeof(uint64_t),
              "HeapObject is expected to be 8 bytes.");

constexpr auto HeaderKindOffset = HeapObject::kind_offset();
constexpr auto HeaderAuxOffset = HeapObject::aux_offset();

inline bool isObjectKind(HeaderKind k) {
  return k >= HeaderKind::Object && k <= HeaderKind::ImmSet;
}

inline bool isArrayKind(HeaderKind k) {
  return k >= HeaderKind::Packed && k <= HeaderKind::Keyset;
}

inline bool isFreeKind(HeaderKind k) {
  return k >= HeaderKind::Free;
}

inline bool isHackArrayKind(HeaderKind k) {
  return
    k == HeaderKind::Dict     ||
    k == HeaderKind::VecArray ||
    k == HeaderKind::Keyset;
}

inline bool isWaithandleKind(HeaderKind k) {
  return k == HeaderKind::WaitHandle ||
         k == HeaderKind::AwaitAllWH ||
         k == HeaderKind::AsyncFuncWH;
}

inline bool isBigKind(HeaderKind k) {
  return k == HeaderKind::BigObj || k == HeaderKind::BigMalloc;
}

enum class CollectionType : uint8_t {
#define COL(name) name = uint8_t(HeaderKind::name),
  COLLECTION_TYPES
#undef COL
};

inline bool isVectorCollection(CollectionType ctype) {
  return ctype == CollectionType::Vector || ctype == CollectionType::ImmVector;
}
inline bool isMapCollection(CollectionType ctype) {
  return ctype == CollectionType::Map || ctype == CollectionType::ImmMap;
}
inline bool isSetCollection(CollectionType ctype) {
  return ctype == CollectionType::Set || ctype == CollectionType::ImmSet;
}
inline bool isValidCollection(CollectionType ctype) {
  return uint8_t(ctype) >= uint8_t(CollectionType::Vector) &&
         uint8_t(ctype) <= uint8_t(CollectionType::ImmSet);
}
inline bool isMutableCollection(CollectionType ctype) {
  return ctype == CollectionType::Vector ||
         ctype == CollectionType::Map ||
         ctype == CollectionType::Set;
}
inline bool isImmutableCollection(CollectionType ctype) {
  return !isMutableCollection(ctype);
}

inline bool collectionAllowsIntStringKeys(CollectionType ctype) {
  return isSetCollection(ctype) || isMapCollection(ctype);
}

}

#endif
