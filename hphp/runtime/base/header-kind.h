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

#include "hphp/runtime/base/header-kind-shared.h"
#include "hphp/util/compilation-flags.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace HPHP {

/*
 * every request-allocated object has a 1-byte kind field in its
 * header, from this enum. If you update the enum, be sure to
 * update header_names[] as well, and either unset or change
 * HHVM_REPO_SCHEMA, because kind values are used in HHBC.
 */
enum class HeaderKind : uint8_t {
  // Array-like header kinds. They must match up with ArrayData::ArrayKind.
  // "vanilla" kinds must be odd and "bespoke" kinds must be even.
  Vec, BespokeVec, Dict, BespokeDict, Keyset, BespokeKeyset,

  // Other ordinary refcounted heap objects
  String, Resource, ClsMeth, RClsMeth, RFunc,

  // Valid kinds for an ObjectData; all but Object and NativeObject are
  // isCppBuiltin()
  Object, NativeObject, WaitHandle, AsyncFuncWH, AwaitAllWH, ConcurrentWH,
  Closure,
  // Collections. Vector and ImmSet are used for range checks; be careful
  // when adding new collection kinds.
  Vector, Map, Set, Pair, ImmVector, ImmMap, ImmSet,

  // other kinds, not used for countable objects.
  AsyncFuncFrame, // NativeNode followed by Frame, Resumable, AFWH
  NativeData, // a NativeData header preceding an HNI ObjectData
  ClosureHdr, // a ClosureHdr preceding a Closure ObjectData
  MemoData, // Memoization data preceding an ObjectData
  Cpp, // a managed object with associated C++ type
  SmallMalloc, // small req::malloc'd block
  BigMalloc, // big req::malloc'd block
  Free, // small block in a FreeList
  Hole, // wasted space not in any freelist
  Slab, // header for a contiguous "slab" of small objects
};
const unsigned NumHeaderKinds = unsigned(HeaderKind::Slab) + 1;
extern const std::array<char*,NumHeaderKinds> header_names;

inline bool haveCount(HeaderKind k) {
  return uint8_t(k) < uint8_t(HeaderKind::AsyncFuncFrame);
}

static_assert((uint8_t)CollectionType::Vector == (uint8_t)HeaderKind::Vector);
static_assert((uint8_t)CollectionType::Map == (uint8_t)HeaderKind::Map);
static_assert((uint8_t)CollectionType::Set == (uint8_t)HeaderKind::Set);
static_assert((uint8_t)CollectionType::Pair == (uint8_t)HeaderKind::Pair);
static_assert((uint8_t)CollectionType::ImmVector == (uint8_t)HeaderKind::ImmVector);
static_assert((uint8_t)CollectionType::ImmMap == (uint8_t)HeaderKind::ImmMap);
static_assert((uint8_t)CollectionType::ImmSet == (uint8_t)HeaderKind::ImmSet);

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
enum RefCount : int32_t {
  OneReference   = 1,
  // MultiReference should never be used outside of one-bit mode, so set it to
  // something above RefCountMaxRealistic to trip asserts.
  MultiReference = 0x40000000,

  // Uncounted refcounts count down from UncountedValue to INT_MIN. A count
  // of UncountedZero is not a valid count - it indicates we must release the
  // memory of the uncounted object.
  UncountedValue = -128,
  UncountedZero  = -127,
  StaticValue    = -126,

  RefCountMaxRealistic = (1 << 30) - 1,
};

using UnsignedRefCount = std::make_unsigned<RefCount>::type;

enum class GCBits : uint8_t {};

/*
 * Header Layout
 *
 * Refcounted types have a 32-bit RefCount normally, or 8-bit plus 24 bits of
 * padding with ONE_BIT_REFCOUNT.
 *
 * 0       32     40      48            56
 * [ cnt | kind | marks | arrBits      | sizeClass ] (vanilla) Vec
 * [ cnt | kind | marks | arrBits      | keyTypes  ] (vanilla) Dict
 * [ cnt | kind | marks |                          ] (vanilla) Keyset
 * [ cnt | kind | marks | arrBits      | extraData ] any BespokeArray
 * [ cnt | kind | marks | sizeClass    | isSymbol  ] String
 * [ cnt | kind | marks | heapSize:16              ] Resource (ResourceHdr)
 * [ cnt | kind | marks | Attribute    |           ] Object..ImmSet (ObjectData)
 *
 * Note: arrBits includes several flags, mostly from the Hack array migration:
 *  - 1 bit for hasAPCTypedValue
 *  - 1 bit for isLegacyArray
 *  - 1 bit for hasStrKeyTable
 *  - 1 bit for isSampledArray
 *  - 4 bits unused
 *
 * Now that HAM is done, we can merge the ArrayKeyTypes bitset (which also
 * uses 4 bits) into this field, so the highest byte is always the size class.
 *
 * Note: when an ObjectData is preceded by a special header (AsyncFuncFrame,
 * NativeData, ClosureHeader, or MemoData), only the special header is marked
 * using the m_marks field; the m_marks field on the interior ObjectData is
 * unused.
 *
 * Special headers have non-refcount uses for m_aux32:
 *
 * 0          32     40      48
 * [ ar_off | kind | marks |              ] AsyncFuncFrame (NativeNode)
 * [ ar_off | kind | marks | tyindex:16   ] NativeData (NativeNode)
 * [ size   | kind | marks |              ] ClosureHeader (ClosureHdr)
 * [ objoff | kind | marks |              ] MemoData
 * [        | kind | marks | tyindex:16   ] Cpp, SmallMalloc (MallocNode)
 * [ index  | kind | marks | tyindex:16   ] BigMalloc (MallocNode)
 * [ index  | kind | marks | kIndexUnkown ] BigObj (MallocNode)
 * [ size   | kind | marks |              ] Free, Hole (FreeNode)
 * [        | kind |       |              ] Slab
 */

/*
 * Common header for all heap-allocated objects. Layout is carefully
 * designed to fit in one 64-bit word.
 */
struct HeapObject {
protected:
  union {
    struct {
      mutable RefCount m_count;
      HeaderKind m_kind;
      mutable GCBits m_marks;
      mutable uint16_t m_aux16;
    };
    mutable std::atomic<std::underlying_type<RefCount>::type> m_atomic_count{};
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
  GCBits marks() const { return m_marks; }
  void setmarks(GCBits m) const { m_marks = m; }
};
static_assert(sizeof(HeapObject) == sizeof(uint64_t),
              "HeapObject is expected to be 8 bytes.");

constexpr auto HeaderKindOffset = HeapObject::kind_offset();
constexpr auto HeaderAuxOffset = HeapObject::aux_offset();

inline constexpr bool isObjectKind(HeaderKind k) {
  return k >= HeaderKind::Object && k <= HeaderKind::ImmSet;
}

inline constexpr bool isArrayKind(HeaderKind k) {
  return k >= HeaderKind::Vec && k <= HeaderKind::BespokeKeyset;
}

inline constexpr bool isFreeKind(HeaderKind k) {
  return k >= HeaderKind::Free;
}

namespace detail {
// update these if you add more kinds for ObjectData that should
// pass the isCppBuiltin() predicate.
constexpr auto FirstCppBuiltin = HeaderKind::WaitHandle;
constexpr auto LastCppBuiltin = HeaderKind::ImmSet;
static_assert(uint8_t(LastCppBuiltin) - uint8_t(FirstCppBuiltin) == 11,
              "keep predicate in sync with enum");
}

// legacy CppBuiltins have custom C++ types (not plain ObjectData layouts)
// are not considered HNI objects, and do not have NativeData headers.
inline constexpr bool isCppBuiltin(HeaderKind k) {
  return k >= detail::FirstCppBuiltin && k <= detail::LastCppBuiltin;
}

inline constexpr bool hasInstanceDtor(HeaderKind k) {
  static_assert(uint8_t(HeaderKind::NativeObject) + 1 ==
                uint8_t(detail::FirstCppBuiltin), "");
  return k >= HeaderKind::NativeObject && k <= detail::LastCppBuiltin;
}

inline constexpr bool isWaitHandleKind(HeaderKind k) {
  return k >= HeaderKind::WaitHandle && k <= HeaderKind::ConcurrentWH;
  static_assert(
    (int)HeaderKind::ConcurrentWH - (int)HeaderKind::WaitHandle == 3,
    "isWaitHandleKind requires updating"
  );
}

}
