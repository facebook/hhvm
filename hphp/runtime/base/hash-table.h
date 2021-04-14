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

#include "hphp/runtime/base/hash-table-x64.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-uncounted.h"

// NvGetStr is implemented in assembly in hash-table-x64.S, additional macros
// are defined for various offsets in hash-table-x64.h
// Types inheriting from HashTable should add this macro to statically verify
// the offsets are correct.
#if defined(__SSE4_2__) && defined(NO_M_DATA) && !defined(NO_HWCRC) && \
    !defined(_MSC_VER)

#define HASH_TABLE_CHECK_OFFSETS(ArrayType, ElmType) \
  static_assert(ArrayType::dataOff() == ArrayType ## _DATA, ""); \
  static_assert(ArrayType::scaleOff() == ArrayType ## _SCALE, ""); \
  static_assert(ElmType::keyOff() == ElmType ## _KEY, ""); \
  static_assert(ElmType::hashOff() == ElmType ## _HASH, ""); \
  static_assert(ElmType::dataOff() == ElmType ## _DATA, ""); \
  static_assert(ElmType::typeOff() == ElmType ## _TYPE, ""); \
  static_assert(sizeof(ElmType) == ElmType ## _QUADWORDS * 8, "");

#else
#define HASH_TABLE_CHECK_OFFSETS(ArrayType, ElmType)
#endif

namespace HPHP {

ALWAYS_INLINE bool validPos(int32_t pos) {
  return pos >= 0;
}

ALWAYS_INLINE bool validPos(ssize_t pos) {
  return pos >= 0;
}

namespace array {

struct HashTableCommon {
  // Load factor scaler. If S is the # of elements, C is the
  // power-of-2 capacity, and L=LoadScale, we grow when S > C-C/L.
  // So 2 gives 0.5 load factor, 4 gives 0.75 load factor, 8 gives
  // 0.875 load factor. Use powers of 2 to enable shift-divide.
  static constexpr uint32_t LoadScale = 4;

  // Element index, with special values < 0 used for hash tables.
  static constexpr int32_t Empty      = -1;
  static constexpr int32_t Tombstone  = -2;

  // Use a minimum of an 4-element hash table.  Valid range: [2..32]
  static constexpr uint32_t LgSmallScale = 0;
  static constexpr uint32_t SmallScale = 1 << LgSmallScale;
  static constexpr uint32_t SmallHashSize = SmallScale * 4;
  static constexpr uint32_t SmallMask = SmallHashSize - 1; // 3
  static constexpr uint32_t SmallSize = SmallScale * 3;

  static constexpr uint64_t MaxHashSize = uint64_t(1) << 32;
  static constexpr uint32_t MaxMask = MaxHashSize - 1;
  static constexpr uint32_t MaxSize = MaxMask - MaxMask / LoadScale;
  static constexpr uint32_t MaxMakeSize = 4 * SmallSize;
  static constexpr uint32_t MaxScale = MaxHashSize / LoadScale;

  constexpr static uint32_t HashSize(uint32_t scale) { return 4 * scale; }
  constexpr static uint32_t Capacity(uint32_t scale) { return 3 * scale; }
  constexpr static uint32_t Mask(uint32_t scale) { return 4 * scale - 1; }

  ALWAYS_INLINE uint32_t capacity() const { return Capacity(m_scale); }
  ALWAYS_INLINE uint32_t mask() const { return Mask(m_scale); }
  ALWAYS_INLINE uint32_t scale() const { return m_scale; }

  static constexpr size_t computeMaxElms(uint32_t mask) {
    return mask - mask / LoadScale;
  }

  static uint32_t computeScaleFromSize(uint32_t n);
  size_t hashSize() const;

  enum FindType { Lookup, Exists, Insert, InsertUpdate, Remove };

  struct Inserter {
    explicit Inserter(int32_t *p) : ei(p) {}
    Inserter& operator=(const int32_t pos) {
      *ei = pos;
      return *this;
    }
    bool isValid() const {
      return ei != nullptr;
    }
    Inserter& operator*() {
      return *this;
    }
    explicit operator int32_t() const {
      return *ei;
    }
  private:
    int32_t* ei;
  };

  struct RemovePos {
    bool valid() const {
      return validPos(elmIdx);
    }
    size_t probeIdx{0};
    int32_t elmIdx{Empty};
  };

  static ALWAYS_INLINE
  bool isValidIns(Inserter e) {
    return e.isValid();
  }

  static ALWAYS_INLINE
  bool isValidPos(Inserter e) {
    assertx(isValidIns(e));
    return (int32_t)(*e) >= 0;
  }
protected:
  static void InitHash(int32_t* hash, uint32_t scale);
  static void CopyHash(int32_t* to, const int32_t* from, uint32_t scale);
  bool isFull() const;

  // Some of these are packed into qword-sized unions so we can
  // combine stores during initialization. (gcc won't do it on its own.)
  union {
    struct {
      uint32_t m_scale;     // size-class equal to 1/4 table size
      uint32_t m_used;      // Number of used elements (values or tombstones)
    };
    uint64_t m_scale_used;
  };
};

///////////////////////////////////////////////////////////////////////////////

template <typename ArrayType, typename ElmType>
struct HashTable : HashTableCommon {
  using Elm = ElmType;
  using hash_t = typename Elm::hash_t;

  /////////////////////////////////////////////////////////////////////////////
  // Offset calculations.
  /////////////////////////////////////////////////////////////////////////////

  static ALWAYS_INLINE Elm* Data(const HashTable* ht) {
    if (ht == nullptr) {
      __builtin_unreachable();
    }
    return const_cast<Elm*>(
      reinterpret_cast<Elm const*>(
        static_cast<ArrayType const*>(ht) + 1
      )
    );
  }

  ALWAYS_INLINE Elm* data() const {
    return Data(this);
  }

  static ALWAYS_INLINE int32_t* HashTab(const HashTable* ht, uint32_t scale) {
    return const_cast<int32_t*>(
      reinterpret_cast<int32_t const*>(
        Data(ht) + static_cast<size_t>(scale) * 3
      )
    );
  }

  ALWAYS_INLINE int32_t* hashTab() const {
    return HashTab(this, m_scale);
  }

  static constexpr ptrdiff_t dataOff() {
    return sizeof(ArrayType);
  }
  static constexpr ptrdiff_t usedOff() {
    return offsetof(ArrayType, m_used);
  }
  static constexpr ptrdiff_t scaleOff() {
    return offsetof(ArrayType, m_scale);
  }

  static constexpr ptrdiff_t elmOff(uint32_t pos) {
    return dataOff() + pos * sizeof(Elm);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Allocation utilities.
  /////////////////////////////////////////////////////////////////////////////

  static ALWAYS_INLINE
  ArrayType* reqAlloc(uint32_t scale) {
    auto const allocBytes = computeAllocBytes(scale);
    return static_cast<ArrayType*>(tl_heap->objMalloc(allocBytes));
  }

  static ALWAYS_INLINE
  ArrayType* staticAlloc(uint32_t scale, size_t extra = 0) {
    extra = (extra + 15) & ~15ull;
    auto const size = computeAllocBytes(scale) + extra;
    auto const mem = RuntimeOption::EvalLowStaticArrays
      ? lower_malloc(size)
      : uncounted_malloc(size);
    return reinterpret_cast<ArrayType*>(reinterpret_cast<char*>(mem) + extra);
  }

  static ALWAYS_INLINE
  ArrayType* uncountedAlloc(uint32_t scale, size_t extra = 0) {
    auto const size = computeAllocBytes(scale) + extra;
    auto const mem = AllocUncounted(size);
    return reinterpret_cast<ArrayType*>(reinterpret_cast<char*>(mem) + extra);
  }

  static ALWAYS_INLINE
  constexpr size_t computeAllocBytes(uint32_t scale) {
    return sizeof(ArrayType) +
           HashSize(scale) * sizeof(int32_t) +
           Capacity(scale) * sizeof(Elm);
  }

  ALWAYS_INLINE
  size_t heapSize() const {
    return computeAllocBytes(m_scale);
  }

  static ALWAYS_INLINE
  size_t computeAllocBytesFromMaxElms(uint32_t maxElms) {
    auto const scale = computeScaleFromSize(maxElms);
    return computeAllocBytes(scale);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Non variant interface
  /////////////////////////////////////////////////////////////////////////////

  static TypedValue NvGetInt(const ArrayData* ad, int64_t k);
  static TypedValue NvGetStr(const ArrayData* ad, const StringData* k);

  // Return the key at the given element, without any refcount ops.
  static TypedValue GetPosKey(const ArrayData* ad, ssize_t pos);

  /////////////////////////////////////////////////////////////////////////////
  // findForNewInsertImpl
  /////////////////////////////////////////////////////////////////////////////
  /**
   * findForNewInsert() CANNOT be used unless the caller can guarantee that
   * the relevant key is not already present in the array. Otherwise this can
   * put the array into a bad state; use with caution. The *Warn
   * version checks for the array becoming too unbalanced because of hash
   * collisions, and is only called when an array Grow()s.
   */

  ALWAYS_INLINE
  Inserter findForNewInsert(hash_t h0) const {
    return findForNewInsert(hashTab(), mask(), h0);
  }

  ALWAYS_INLINE
  Inserter findForNewInsertWarn(hash_t h0) const {
    return findForNewInsertWarn(hashTab(), mask(), h0);
  }

  Inserter findForNewInsert(int32_t* table, size_t mask,
                                hash_t h0) const;
  Inserter findForNewInsertWarn(int32_t* table, size_t mask,
                                    hash_t h0) const;

  /////////////////////////////////////////////////////////////////////////////
  // Iteration
  /////////////////////////////////////////////////////////////////////////////
  ssize_t getIterBeginNotEmpty() const;

  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);

  /////////////////////////////////////////////////////////////////////////////
  // Utility methods
  /////////////////////////////////////////////////////////////////////////////
protected:
  ALWAYS_INLINE Elm* allocElm(Inserter ei) {
    assertx(!isValidPos(ei) && !isFull());
    assertx(array()->m_size <= m_used);
    ++(array()->m_size);
    size_t i = m_used;
    (*ei) = i;
    m_used = i + 1;
    return &data()[i];
  }

  ALWAYS_INLINE int32_t* initHash(uint32_t scale) {
    auto table = HashTab(this, scale);
    InitHash(table, scale);
    return table;
  }

  static ALWAYS_INLINE bool hitIntKey(const Elm& e, int64_t ki) {
    assertx(!e.isInvalid());
    return e.intKey() == ki && e.hasIntKey();
  }

  static ALWAYS_INLINE bool hitStrKey(const Elm& e,
                                      const StringData* ks,
                                      hash_t h) {
    assertx(!e.isInvalid());
    /*
     * We do not have to check e.hasStrKey() because it is
     * implicitely done by the check on the hash.
     */
    return e.hash() == h && (ks == e.strKey() || ks->same(e.strKey()));
  }

  /////////////////////////////////////////////////////////////////////////////
  // findImpls
  /////////////////////////////////////////////////////////////////////////////
public:
  ALWAYS_INLINE
  int32_t find(int64_t ki, hash_t h0) const {
    return findImpl<FindType::Lookup>(
        h0,
        [ki](const Elm& e) {
          return hitIntKey(e, ki);
        }
      );
  }

  ALWAYS_INLINE
  int32_t find(const StringData* ks, hash_t h0) const {
    return findImpl<FindType::Lookup>(
        h0,
        [ks, h0](const Elm& e) {
          return hitStrKey(e, ks, h0);
        }
      );
  }

  ALWAYS_INLINE
  bool findForExists(int64_t ki, hash_t h0) const {
    return findImpl<FindType::Exists>(
        h0,
        [ki](const Elm& e) {
          return hitIntKey(e, ki);
        }
      );
  }

  ALWAYS_INLINE
  Inserter findForInsert(int64_t ki, hash_t h0) const {
    return findImpl<FindType::Insert>(
        h0,
        [ki](const Elm& e) {
          return hitIntKey(e, ki);
        }
      );
  }

  ALWAYS_INLINE
  Inserter findForInsert(const StringData* ks, hash_t h0) const {
    return findImpl<FindType::Insert>(
        h0,
        [ks, h0](const Elm& e) {
          return hitStrKey(e, ks, h0);
        }
      );
  }

  ALWAYS_INLINE
  Inserter findForInsertUpdate(int64_t ki, hash_t h0) const {
    return findImpl<FindType::InsertUpdate>(
        h0,
        [ki](const Elm& e) {
          return hitIntKey(e, ki);
        }
      );
  }

  ALWAYS_INLINE
  Inserter findForInsertUpdate(const StringData* ks, hash_t h0) const {
    return findImpl<FindType::InsertUpdate>(
        h0,
        [ks, h0](const Elm& e) {
          return hitStrKey(e, ks, h0);
        }
      );
  }

  ALWAYS_INLINE
  auto findForRemove(int64_t ki, hash_t h0) const {
    return findImpl<FindType::Remove>(
        h0,
        [ki](const Elm& e) {
          return hitIntKey(e, ki);
        }
      );
  }

  ALWAYS_INLINE
  auto findForRemove(const StringData* ks, hash_t h0) const {
    return findImpl<FindType::Remove>(
        h0,
        [ks, h0](const Elm& e) {
          return hitStrKey(e, ks, h0);
        }
    );
  }

  template <typename Hit> ALWAYS_INLINE
  auto findForRemove(
      typename std::enable_if<!std::is_integral<Hit>::value &&
                              !std::is_pointer<Hit>::value, hash_t>::type h0,
      Hit hit) const {
    return findImpl<FindType::Remove>(h0, hit);
  }

protected:
  template <FindType type, typename Hit>
  typename std::conditional<
    type == HashTableCommon::FindType::Lookup,
    int32_t,
    typename std::conditional<
      type == HashTableCommon::FindType::Remove,
      typename HashTableCommon::RemovePos,
      typename std::conditional<
        type == HashTableCommon::FindType::Exists,
        bool,
        typename HashTableCommon::Inserter
      >::type
    >::type
  >::type findImpl(hash_t h0, Hit hit) const;

  /////////////////////////////////////////////////////////////////////////////
  // Iteration helpers.
  /////////////////////////////////////////////////////////////////////////////
  ssize_t getIterBegin() const;
  ssize_t getIterLast() const;
  ssize_t getIterEnd() const;

  ssize_t iter_advance_helper(ssize_t next_pos) const;

  ssize_t nextElm(Elm* elms, ssize_t ei) const;
  ssize_t prevElm(Elm* elms, ssize_t ei) const;
  ssize_t nextElm(ssize_t ei) const;

  /////////////////////////////////////////////////////////////////////////////
  // Downcast helpers
  /////////////////////////////////////////////////////////////////////////////
private:
  static ALWAYS_INLINE
  ArrayType* asArrayType(ArrayData* ad) {
    assertx(ad->isVanillaDict() || ad->isVanillaKeyset());
    auto a = static_cast<ArrayType*>(ad);
    assertx(a->checkInvariants());
    return a;
  }
  static ALWAYS_INLINE
  const ArrayType* asArrayType(const ArrayData* ad) {
    assertx(ad->isVanillaDict() || ad->isVanillaKeyset());
    auto a = static_cast<const ArrayType*>(ad);
    assertx(a->checkInvariants());
    return a;
  }

  ALWAYS_INLINE ArrayType* array() {
    return static_cast<ArrayType*>(this);
  }
  ALWAYS_INLINE const ArrayType* array() const {
    return static_cast<ArrayType const*>(this);
  }
};

}  // namespace array
}  // namespace HPHP

#include "hphp/runtime/base/hash-table-inl.h"

