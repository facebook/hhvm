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

namespace HPHP {

namespace array {

ALWAYS_INLINE
uint32_t HashTableCommon::computeScaleFromSize(uint32_t n) {
  assert(n <= 0x7fffffffU);
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
    ".l%=:\n"
    "sub        $0x10, %0\n"
    "movdqu     %%xmm0, (%1, %0)\n"
    "ja         .l%=\n"
    : "+r"(offset) : "r"(hash) : "xmm0"
  );
#elif defined(__aarch64__)
  static_assert(Empty == -1, "The following fills with all 1's.");
  assertx(HashSize(scale) == scale * 4);

  uint64_t offset = scale * 16;
  uint64_t ones = -1;
  auto hash2 = hash;
  __asm__ __volatile__(
    ".l%=:\n"
    "stp        %x2, %x2, [%x1], #16\n"
    "subs       %x0, %x0, #16\n"
    "bhi        .l%=\n"
    : "+r"(offset), "+r"(hash2) : "r"(ones)
  );
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
  assert(m_used <= capacity());
  return m_used == capacity();
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE
ssize_t HashTable<ArrayType, ElmType>::getIterBeginNotEmpty() const {
  // Expedite no tombstone case.
  assert(!array()->empty());
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
  assert(-1 <= ei && ei < m_used);
  while (++ei < m_used) {
    if (LIKELY(!elms[ei].isTombstone())) return ei;
  }
  assert(ei == m_used);
  return m_used;
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE ssize_t HashTable<ArrayType, ElmType>::nextElm(ssize_t ei) const {
  return nextElm(data(), ei);
}

template<typename ArrayType, typename ElmType>
ALWAYS_INLINE ssize_t HashTable<ArrayType, ElmType>::prevElm(Elm* elms,
                                                             ssize_t ei) const {
  assert(ei < ssize_t(m_used));
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
  assert(next_pos == m_used);
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
ALWAYS_INLINE void HashTable<ArrayType, ElmType>::InitSmallHash(ArrayType* a) {
  // Hash table should be initialized before header.
#if defined(__x86_64__)
  static_assert(Empty == -1, "");
  static_assert(SmallSize == 3, "");
  __asm__ __volatile__(
    "pcmpeqd    %%xmm0, %%xmm0\n"          // xmm0 <- 11111....
    "movdqu     %%xmm0, %0\n"
    : : "o"(*((uint8_t*)a + (sizeof(ArrayType) + SmallSize * sizeof(Elm))))
    : "xmm0"
  );
#elif defined(__aarch64__)
  static_assert(Empty == -1, "");
  static_assert(SmallSize == 3, "");
  auto const emptyVal = int64_t{Empty};
  //Use a2 since writeback == true for stp instruction
  auto a2 = a;
  __asm__ __volatile__(
    "stp        %x1, %x1, %x0\n"
    : "+o"(*((uint8_t*)a2 + (sizeof(ArrayType) + SmallSize * sizeof(Elm))))
    : "r"(emptyVal)
  );
#else
  auto const hash = HashTab(a, SmallScale);
  auto const emptyVal = int64_t{Empty};
  reinterpret_cast<int64_t*>(hash)[0] = emptyVal;
  reinterpret_cast<int64_t*>(hash)[1] = emptyVal;
#endif
}

template<typename ArrayType, typename ElmType>
const TypedValue* HashTable<ArrayType, ElmType>::NvGetInt(const ArrayData* ad,
                                                          int64_t ki) {
  auto a = asArrayType(ad);
  auto i = a->find(ki, hash_int64(ki));
  return LIKELY(validPos(i)) ? a->data()[i].datatv() : nullptr;
}

#if !defined(__SSE4_2__) || defined(NO_HWCRC) || !defined(NO_M_DATA) || \
  defined(_MSC_VER)
// This function is implemented directly in ASM in hash-table-x64.S otherwise.
template<typename ArrayType, typename ElmType>
const TypedValue* HashTable<ArrayType, ElmType>::NvGetStr(const ArrayData* ad,
                                                          const StringData* k) {
  auto a = asArrayType(ad);
  auto i = a->find(k, k->hash());
  if (LIKELY(validPos(i))) {
    return a->data()[i].datatv();
  }
  return nullptr;
}
#else
  // hash-table-x64.S depends on StringData and ArrayType layout.
  // If these fail, update the constants
  static_assert(sizeof(StringData) == SD_DATA, "");
  static_assert(StringData::sizeOff() == SD_LEN, "");
  static_assert(StringData::hashOff() == SD_HASH, "");
#endif

template<typename ArrayType, typename ElmType>
Cell HashTable<ArrayType, ElmType>::NvGetKey(const ArrayData* ad, ssize_t pos) {
  auto a = asArrayType(ad);
  assert(pos != a->m_used);
  assert(!a->data()[pos].isTombstone());
  return a->data()[pos].getKey();
}

template <typename ArrayType, typename ElmType> ALWAYS_INLINE
typename HashTable<ArrayType, ElmType>::Inserter
HashTable<ArrayType, ElmType>::findForNewInsert(int32_t* table, size_t mask,
                                                hash_t h0) const {
  for (uint32_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) {
      return Inserter(ei);
    }
    probe += i;
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

template <typename ArrayType, typename ElmType> ALWAYS_INLINE
typename HashTable<ArrayType, ElmType>::Inserter
HashTable<ArrayType, ElmType>::findForNewInsertWarn(int32_t* table,
                                                    size_t mask,
                                                    hash_t h0) const {
  uint32_t balanceLimit = RuntimeOption::MaxArrayChain;
  for (uint32_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
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
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

// Quadratic probe is:
//
//   h(k, i) = (k + c1*i + c2*(i^2)) % tableSize
//
// Use 1/2 for c1 and c2.  In combination with a table size that is a power of
// 2, this guarantees a probe sequence of length tableSize that probes all
// table elements exactly once.

template <typename ArrayType, typename ElmType>
template <typename HashTableCommon::FindType type,
         typename Hit,
         typename RLambda>
ALWAYS_INLINE
typename std::conditional<
  type != HashTableCommon::FindType::Insert &&
  type != HashTableCommon::FindType::InsertUpdate,
  int32_t,
  typename HashTableCommon::Inserter
>::type HashTable<ArrayType, ElmType>::findImpl(hash_t h0,
                                                Hit hit,
                                                RLambda remove) const {
  static_assert(
    static_cast<int>(FindType::Lookup) == 0 &&
    static_cast<int>(FindType::Insert) == 1 &&
    static_cast<int>(FindType::InsertUpdate) == 2 &&
    static_cast<int>(FindType::Remove) == 3,
    "Update the tuple accessing code below."
  );
  uint32_t mask = this->mask();
  auto elms = data();
  auto hash = hashTab();

  for (uint32_t probeIndex = h0, i = 1;; ++i) {
    auto const ei = &hash[probeIndex & mask];
    int32_t pos = *ei;

    if (validPos(pos)) {
      assert(0 <= pos);
      assert(pos < capacity());
      if (hit(elms[pos])) {
        if (type == FindType::Remove) {
          remove(elms[pos]);
          *ei = Tombstone;
        }
        return std::get<static_cast<int>(type)>(
          std::make_tuple(int32_t(pos), Inserter(nullptr),
                          Inserter(ei), int32_t(pos))
        );
      }
    } else if (pos & 1) {
      assert(pos == Empty);
      return std::get<static_cast<int>(type)>(
        std::make_tuple(int32_t(pos), Inserter(ei),
                        Inserter(ei), int32_t(pos))
      );
    }

    probeIndex += i;
    assertx(i <= mask);
    assertx(probeIndex == static_cast<uint32_t>(h0) + (i * (i + 1)) / 2);
  }
}

}  // namespace array
}  // namespace HPHP
