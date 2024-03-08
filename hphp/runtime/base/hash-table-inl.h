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

#include "hphp/util/portability.h"

namespace HPHP {

namespace array {

ALWAYS_INLINE
uint32_t HashTableCommon::computeScaleFromSize(uint32_t n) {
  if (n >= MaxSize) {
    return MaxScale;
  }
  auto scale = SmallScale;
  while (Capacity(scale) < n) scale *= 2;
  return scale;
  static_assert(SmallHashSize >= 4,
                "lower limit for 0.75 load factor");
}

ALWAYS_INLINE
size_t HashTableCommon::hashSize() const {
  return HashSize(m_scale);
}

ALWAYS_INLINE
void HashTableCommon::InitHash(int32_t* hash, uint32_t scale) {
#if defined(__x86_64__)
  static_assert(Empty == -1, "The following fills with all 1's.");
  assertx(HashSize(scale) == scale * 4);

  uint64_t offset = scale * 16;
  __asm__ __volatile__(
    "pcmpeqd    %%xmm0, %%xmm0\n"          // xmm0 <- 11111....
    ASM_LOCAL_LABEL("HTCIH%=") ":\n"
    "sub        $0x10, %0\n"
    "movdqu     %%xmm0, (%1, %0)\n"
    "ja         " ASM_LOCAL_LABEL("HTCIH%=") "\n"
    : "+r"(offset) : "r"(hash) : "xmm0"
  );
#elif defined(__aarch64__)
  static_assert(Empty == -1, "The following fills with all 1's.");
  assertx(HashSize(scale) == scale * 4);

  uint64_t offset = scale * 16;
  uint64_t ones = -1;
  auto hash2 = hash;
  __asm__ __volatile__(
    ASM_LOCAL_LABEL("HTCIH%=") ":\n"
    "stp        %x2, %x2, [%x1], #16\n"
    "subs       %x0, %x0, #16\n"
    "bhi        " ASM_LOCAL_LABEL("HTCIH%=") "\n"
    : "+r"(offset), "+r"(hash2) : "r"(ones) : "cc"
  );
#elif defined(__powerpc__)
  static_assert(Empty == -1, "The following fills with all 1's.");
  assertx(HashSize(scale) == scale * 4);

  uint64_t offset = scale * 16;
  __asm__ __volatile__(
    "vspltisw   0, -1       \n"
    ASM_LOCAL_LABEL("HTCIH%=") ":\n"
    "subic.     %0, %0, 0x10\n"
    "stxvd2x    32, %1, %0   \n"
    "bgt        " ASM_LOCAL_LABEL("HTCIH%=") "\n"
    : "+b"(offset) : "b"(hash) : "v0");
#else
  static_assert(Empty == -1, "Cannot use wordfillones().");
  wordfillones(hash, HashSize(scale));
#endif
}

ALWAYS_INLINE
void HashTableCommon::CopyHash(int32_t* to,
                               const int32_t* from,
                               uint32_t scale) {
  assertx(HashSize(scale) == scale * 4);
  uint64_t nBytes = scale * 16;
  memcpy16_inline(to, from, nBytes);
}

ALWAYS_INLINE bool HashTableCommon::isFull() const {
  assertx(m_used <= capacity());
  return m_used == capacity();
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE
ssize_t HashTable<ArrayType, ElmType>::getIterBeginNotEmpty() const {
  // Expedite no tombstone case.
  assertx(!array()->empty());
  if (LIKELY(!data()[0].isTombstone())) {
      return 0;
  }
  return nextElm(data(), 0);
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE ssize_t HashTable<ArrayType, ElmType>::getIterBegin() const {
  return nextElm(data(), -1);
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE ssize_t HashTable<ArrayType, ElmType>::getIterLast() const {
  auto elms = data();
  ssize_t ei = m_used;
  while (--ei >= 0) {
    if (!elms[ei].isTombstone()) {
      return ei;
    }
  }
  return m_used;
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE ssize_t HashTable<ArrayType, ElmType>::getIterEnd() const {
  return m_used;
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE ssize_t HashTable<ArrayType, ElmType>::nextElm(Elm* elms,
                                                             ssize_t ei) const {
  assertx(-1 <= ei && ei < m_used);
  while (++ei < m_used) {
    if (LIKELY(!elms[ei].isTombstone())) return ei;
  }
  assertx(ei == m_used);
  return m_used;
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE ssize_t HashTable<ArrayType, ElmType>::nextElm(ssize_t ei) const {
  return nextElm(data(), ei);
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE ssize_t HashTable<ArrayType, ElmType>::prevElm(Elm* elms,
                                                             ssize_t ei) const {
  assertx(ei < ssize_t(m_used));
  while (--ei >= 0) {
    if (!elms[ei].isTombstone()) {
      return ei;
    }
  }
  return m_used;
}

template<typename ArrayType, typename ElmType>
ssize_t HashTable<ArrayType, ElmType>::IterBegin(const ArrayData* ad) {
  auto a = asArrayType(ad);
  return a->getIterBegin();
}

template<typename ArrayType, typename ElmType>
ssize_t HashTable<ArrayType, ElmType>::IterLast(const ArrayData* ad) {
  auto a = asArrayType(ad);
  return a->getIterLast();
}

template<typename ArrayType, typename ElmType>
ssize_t HashTable<ArrayType, ElmType>::IterEnd(const ArrayData* ad) {
  auto a = asArrayType(ad);
  return a->getIterEnd();
}

// caller has already incremented pos but encountered a tombstone
template<typename ArrayType, typename ElmType>
ssize_t
HashTable<ArrayType, ElmType>::iter_advance_helper(ssize_t next_pos) const {
  Elm* elms = data();
  for (auto limit = m_used; size_t(next_pos) < limit; ++next_pos) {
    if (!elms[next_pos].isTombstone()) {
      return next_pos;
    }
  }
  assertx(next_pos == m_used);
  return next_pos;
}

template<typename ArrayType, typename ElmType>
ssize_t HashTable<ArrayType, ElmType>::IterAdvance(const ArrayData* ad,
                                                   ssize_t pos) {
  auto a = asArrayType(ad);
  ++pos;
  if (pos >= a->m_used) return a->m_used;
  if (!a->data()[pos].isTombstone()) {
    return pos;
  }
  return a->iter_advance_helper(pos);
}

template<typename ArrayType, typename ElmType>
ssize_t HashTable<ArrayType, ElmType>::IterRewind(const ArrayData* ad,
                                                  ssize_t pos) {
  auto a = asArrayType(ad);
  if (pos == a->getIterEnd()) return pos;
  return a->prevElm(a->data(), pos);
}

template<typename ArrayType, typename ElmType>
TypedValue HashTable<ArrayType, ElmType>::NvGetInt(const ArrayData* ad,
                                                   int64_t k) {
  auto a = asArrayType(ad);
  auto i = a->find(k, hash_int64(k));
  return LIKELY(validPos(i)) ? *a->data()[i].datatv() : make_tv<KindOfUninit>();
}

#ifndef USE_X86_STRING_HELPERS
// This function is implemented directly in ASM in hash-table-x64.S otherwise.
template<typename ArrayType, typename ElmType>
TypedValue HashTable<ArrayType, ElmType>::NvGetStr(const ArrayData* ad,
                                                   const StringData* k) {
  auto a = asArrayType(ad);
  auto i = a->find(k, k->hash());
  return LIKELY(validPos(i)) ? *a->data()[i].datatv() : make_tv<KindOfUninit>();
}
#else
  // hash-table-x64.S depends on StringData and ArrayType layout.
  // If these fail, update the constants
  static_assert(sizeof(StringData) == SD_DATA, "");
  static_assert(StringData::sizeOff() == SD_LEN, "");
  static_assert(StringData::hashOff() == SD_HASH, "");
#endif

template<typename ArrayType, typename ElmType>
TypedValue HashTable<ArrayType, ElmType>::GetPosKey(const ArrayData* ad, ssize_t pos) {
  auto a = asArrayType(ad);
  assertx(pos != a->m_used);
  assertx(!a->data()[pos].isTombstone());
  return a->data()[pos].getKey();
}

template <typename ArrayType, typename ElmType> ALWAYS_INLINE
typename HashTable<ArrayType, ElmType>::Inserter
HashTable<ArrayType, ElmType>::findForNewInsert(int32_t* table, size_t mask,
                                                hash_t h0) const {
  for (uint64_t i = 1, probe = (uint32_t)h0 & mask;; ++i) {
    auto ei = &table[probe];
    if (!validPos(*ei)) {
      return Inserter(ei);
    }
    probe += i;
    probe &= mask;
    assertx(i <= mask);
    assertx(probe == ((static_cast<uint64_t>(h0) + (i + i * i) / 2) & mask));
  }
}

template <typename ArrayType, typename ElmType> ALWAYS_INLINE
typename HashTable<ArrayType, ElmType>::Inserter
HashTable<ArrayType, ElmType>::findForNewInsertWarn(int32_t* table,
                                                    size_t mask,
                                                    hash_t h0) const {
  uint64_t balanceLimit = Cfg::Server::MaxArrayChain;
  for (uint64_t i = 1, probe = (uint32_t)h0 & mask;; ++i) {
    auto ei = &table[probe];
    if (!validPos(*ei)) {
      return LIKELY(i <= balanceLimit)
             ? Inserter(ei)
             : Inserter(
                 warnUnbalanced(
                   const_cast<ArrayType*>(
                     array()
                   ),
                   i,
                   ei
                )
              );
    }
    probe += i;
    probe &= mask;
    assertx(i <= mask);
    assertx(probe == ((static_cast<uint64_t>(h0) + (i + i * i) / 2) & mask));
  }
}

template <typename ArrayType, typename ElmType>
ArrayType*
HashTable<ArrayType, ElmType>::InsertCheckUnbalanced(ArrayType* ad,
                                                     int32_t* table,
                                                     uint32_t mask,
                                                     ElmType* iter,
                                                     ElmType* stop) {
  for (uint32_t i = 0; iter != stop; ++iter, ++i) {
    auto& e = *iter;
    if (e.isTombstone()) continue;
    *ad->findForNewInsertWarn(table, mask, e.probe()) = i;
  }
  return ad;
}

// Quadratic probe is:
//
//   h(k, i) = (k + c1*i + c2*(i^2)) % tableSize
//
// Use 1/2 for c1 and c2.  In combination with a table size that is a power of
// 2, this guarantees a probe sequence of length tableSize that probes all
// table elements exactly once.

template <typename ArrayType, typename ElmType>
template <typename HashTableCommon::FindType type, typename Hit>
ALWAYS_INLINE
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
>::type HashTable<ArrayType, ElmType>::findImpl(hash_t h0, Hit hit) const {
  static_assert(
    static_cast<int>(FindType::Lookup) == 0 &&
    static_cast<int>(FindType::Exists) == 1 &&
    static_cast<int>(FindType::Insert) == 2 &&
    static_cast<int>(FindType::InsertUpdate) == 3 &&
    static_cast<int>(FindType::Remove) == 4,
    "Update the tuple accessing code below."
  );
  uint64_t mask = this->mask();
  auto elms = data();
  auto hash = hashTab();

  for (uint64_t probe = (uint32_t)h0 & mask, i = 1;; ++i) {
    auto const ei = &hash[probe];
    int32_t pos = *ei;

    if (validPos(pos)) {
      assertx(0 <= pos);
      assertx(pos < capacity());
      if (hit(elms[pos])) {
        return std::get<static_cast<int>(type)>(
          std::make_tuple(int32_t(pos), true, Inserter(nullptr),
                          Inserter(ei),
                          RemovePos{uint32_t(probe), int32_t(pos)})
        );
      }
    } else if (pos & 1) {
      assertx(pos == Empty);
      return std::get<static_cast<int>(type)>(
        std::make_tuple(int32_t(Empty), false, Inserter(ei),
                        Inserter(ei), RemovePos{})
      );
    }

    probe += i;
    probe &= mask;
    assertx(i <= mask);
    assertx(probe == ((static_cast<uint64_t>(h0) + (i * (i + 1)) / 2) & mask));
  }
}

}  // namespace array
}  // namespace HPHP
